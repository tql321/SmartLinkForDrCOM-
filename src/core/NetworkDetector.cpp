#include "NetworkDetector.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QNetworkAccessManager>
#include"KeepLiveManager.h"
#include<QNetworkConfigurationManager>
#include"../models/DataMaid.h"
#include <QNetworkInterface>
#include<QString>
#include <QRegularExpression>
#include<QRegularExpression>
NetworkDetector::NetworkDetector()
	: QObject(nullptr)
	, m_netManager(new QNetworkAccessManager(this))
	, m_netConfigManager(new QNetworkConfigurationManager(this))
	, m_forceLoginTimer(new QTimer(this))
	, m_autoLoginTimer(new QTimer(this))
{
	initNetworkMonitor();
}

void NetworkDetector::startDetection()
{
	qDebug() << "开始双重网络探测...";
	probeGateway(); // 从第一步开始
}

void NetworkDetector::initNetworkMonitor()
{
	// 2. 监听底层网络状态变化 (休眠/唤醒/换Wi-Fi 都会触发这个)
	// 注意：QNetworkConfigurationManager 在某些系统（如Windows）上断开WiFi时不敏感
	// 因此我们同时使用定时器轮询 QNetworkInterface 状态来弥补
	connect(m_netConfigManager, &QNetworkConfigurationManager::onlineStateChanged,
		this, &NetworkDetector::handleNetworkStateChanged);
	connect(m_autoLoginTimer, &QTimer::timeout, this, [this]() {
		bool currentOnline = checkCurrentOnlineState();
		if (currentOnline != m_lastOnlineState) {
			handleNetworkStateChanged(currentOnline);
		}
		});
	// 掉线强制重连检测定时器
	connect(m_forceLoginTimer, &QTimer::timeout, this, [this]() {
		if (DATAMAID.getEnableForceLogin()) {
			startDetection();
		}
		});
	// 自动登录检测定时器
	if (DATAMAID.getEnableAutoLogin()) {
		
		m_autoLoginTimer->start(5000); // 每5秒检查一次网络状态
	}
	if (DATAMAID.getEnableForceLogin()) {
		
		m_forceLoginTimer->start(60000); // 每分钟检查一次网络状态
	}


	// 监听实时变更
	connect(&DATAMAID, &DataMaid::sigEnableAutoLoginChanged, this, [this]() {
		if (DATAMAID.getEnableAutoLogin()) {
			if (!m_autoLoginTimer->isActive()) {
				m_autoLoginTimer->start(5000); 
			}
		}
		else {
			m_autoLoginTimer->stop();
		}
		});
	connect(&DATAMAID, &DataMaid::sigEnableForceLoginChanged, this, [this]() {
		if (DATAMAID.getEnableForceLogin()) {
			if (!m_forceLoginTimer->isActive()) {
				m_forceLoginTimer->start(60000); 
			}
		}
		else {
			m_forceLoginTimer->stop();
		}
		});
	// 4. 软件刚启动时，主动触发一次检查
	m_lastOnlineState = checkCurrentOnlineState();
	if (m_lastOnlineState) {
		QTimer::singleShot(1000, this, &NetworkDetector::startDetection);
	}
}

void NetworkDetector::handleNetworkStateChanged(bool isOnline)
{
	// 防止重复触发
	if (m_lastOnlineState == isOnline && sender() != m_netConfigManager) {
		return;
	}
	m_lastOnlineState = isOnline;

	if (isOnline) {
		qDebug() << "检测到网络连接建立！(可能从休眠唤醒，或切换了 Wi-Fi)";

		// 💡 极其关键的实战技巧：延迟探测
		// 刚连上 Wi-Fi 的头一两秒，网卡可能还在获取 DHCP/DNS，直接发请求必败。
		// 必须给系统留出 2~3 秒的缓冲时间！
		QTimer::singleShot(3000, this, [this]() {
			qDebug() << "🔍 缓冲结束，发射探针侦测当前网络环境...";
			startDetection();
			});
	}
	else {
		qDebug() << "❌ 检测到网络断开！(可能进入休眠，或离开了 Wi-Fi 范围)";
		// 网络都没了，保活毫无意义，立刻停止
		KeepLiveManager::instance().stopBrowseTimer();
		//TODO: 停止自动登录
		
	}
}



bool NetworkDetector::checkCurrentOnlineState()
{
	// 遍历所有网络接口，判断是否有活动的非本地回环网卡
	const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	for (const QNetworkInterface& iface : interfaces) {
		// 网卡已启动，正在运行，并且不是本地回环(127.0.0.1)
		if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
			iface.flags().testFlag(QNetworkInterface::IsRunning) &&
			!iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

			// 排除常见的虚拟网口、代理网卡、虚拟机网段
			QString name = iface.humanReadableName().toLower();
			if (name.contains("vmware") ||
				name.contains("virtual") ||
				name.contains("vbox") ||
				name.contains("vpn") ||
				name.contains("tap") ||
				name.contains("tun") ||
				name.contains("vethernet") ||
				name.contains("npcap") ||
				name.contains("wsl") ||
				name.contains("singbox") ||
				name.contains("clash") ||
				name.contains("v2ray")) {
				continue;
			}

			// 确保网卡确实分配了IPv4地址
			const QList<QNetworkAddressEntry> entries = iface.addressEntries();
			for (const QNetworkAddressEntry& entry : entries) {
				if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
					QString ipString = entry.ip().toString();
					// 排除无效的 APIPA (169.254.x.x) 和回环地址
					if (!ipString.startsWith("169.254.") && !ipString.startsWith("127.")) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

// ================= 第一步：探测内网网关 =================
void NetworkDetector::probeGateway()
{
	QString gwUrl = "http://" + DATAMAID.getAuthServerIp();
	QNetworkRequest request((QUrl(gwUrl)));
	QNetworkReply* reply = m_netManager->get(request);

	// Qt5 兼容的 1.5 秒超时机制
	QTimer* timeoutTimer = new QTimer(reply);
	timeoutTimer->setSingleShot(true);
	connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
		if (reply->isRunning()) reply->abort();
		});
	timeoutTimer->start(1500);

	connect(reply, &QNetworkReply::finished, this, [this, reply, timeoutTimer]() {
		timeoutTimer->stop();

		bool isCampus = false;
		if (reply->error() == QNetworkReply::NoError ||
			reply->error() == QNetworkReply::ContentAccessDenied ||
			reply->error() == QNetworkReply::RemoteHostClosedError) {
			// 能连通或者被拦截，说明在局域网内
			isCampus = true;
		}

		reply->deleteLater();

		if (isCampus) {
			qDebug() << "[步骤一] 网关可达，确认为校园网环境。开始探测是否已登录...";
			// 触发第二步
			probeCaptivePortal();
		}
		else {
			qDebug() << "[步骤一] 网关不可达，判定为外网环境。";
			// 流程结束，发射信号
			emit sigDetectionFinished(OuterNetwork);
		}
		});
}

// ================= 第二步：探测 204 外网接口 =================
void NetworkDetector::probeCaptivePortal()
{
	QNetworkRequest request((QUrl(CPD_URL)));

	// 【关键】：禁止 Qt 自动处理重定向！
	// 如果校园网重定向到登录页，我们要第一时间抓到 302 状态码，而不是让它跟着跳
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);

	QNetworkReply* reply = m_netManager->get(request);

	// 设 3 秒超时
	QTimer* timeoutTimer = new QTimer(reply);
	timeoutTimer->setSingleShot(true);
	connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
		if (reply->isRunning()) reply->abort();
		});
	timeoutTimer->start(3000);

	connect(reply, &QNetworkReply::finished, this, [this, reply, timeoutTimer]() {
		timeoutTimer->stop();

		int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (statusCode == 204) {
			qDebug() << "[步骤二] 外网畅通 (HTTP 204)。";
			emit sigDetectionFinished(CampusOnline);
		}
		else {
			// 返回 302, 或者 200(但返回了 HTML 登录页代码)，或者其他网络错误
			qDebug() << "[步骤二] 探测被拦截或返回异常状态码:" << statusCode;
			emit sigDetectionFinished(CampusNeedsLogin);
		}

		reply->deleteLater();
		});
}

QString NetworkDetector::parseAuthServerIp(const QString& portalUrl)
{
	if (portalUrl.isEmpty()) {
		qDebug() << "[-] portalUrl is empty.";
		return "";
	}

	QNetworkRequest request((QUrl(portalUrl)));
	// 伪装成浏览器请求
	request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

	qDebug() << "[*] 正在请求 Portal 页面以提取参数:" << portalUrl;
	QNetworkReply* reply = m_netManager->get(request);

	QEventLoop loop;
	QTimer timer;
	timer.setSingleShot(true);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

	timer.start(5000); // 加入温和的 5 秒超时保护
	loop.exec(); // 等待请求完成

	QString v4serip = "";
	// 检查是否是因为超时而退出的循环
	if (!timer.isActive()) {
		qDebug() << "[x] 请求 Portal 页面超时(无响应)。";
		reply->abort();
		reply->deleteLater();
		return v4serip;
	}
	timer.stop(); // 正常完成则停掉定时器

	if (reply->error() == QNetworkReply::NoError) {
		QString htmlContent = QString::fromUtf8(reply->readAll());
		QRegularExpression re("v4serip\\s*=\\s*['\"]([^'\"]+)['\"]");
		QRegularExpressionMatch match = re.match(htmlContent);

		if (match.hasMatch()) {
			v4serip = match.captured(1);
			qDebug() << "[+] 成功提取到 v4serip:" << v4serip;
		} else {
			qDebug() << "[-] 页面请求成功，但未能在 HTML 中匹配到 v4serip 的值。";
			qDebug() << "[Debug] 返回的前段内容:" << (htmlContent.length() > 200 ? htmlContent.left(200) + "..." : htmlContent);
		}
	} else {
		qDebug() << "[x] 请求 Portal 页面失败:" << reply->errorString();
	}

	reply->deleteLater();
	return v4serip;
}

QString NetworkDetector::parseServerUrl()
{
	QUrl url("http://captive.apple.com/hotspot-detect.html");
	QNetworkRequest request(url);
	// 禁止 Qt 自动跟随重定向，方便我们主动抓取 301/302 头中的 Location
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, false);

	// 发起请求
	QNetworkReply* reply = m_netManager->get(request);

	QEventLoop loop;
	QTimer timer;
	timer.setSingleShot(true);
	connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

	timer.start(5000); // 5秒超时
	loop.exec(); // 等待请求完成

	QString targetUrl = "";
	if (!timer.isActive()) {
		qDebug() << "[x] 请求探测地址(captive)超时。";
		reply->abort();
		reply->deleteLater();
		return targetUrl;
	}
	timer.stop();

	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	// 如果校园网网关使用 HTTP 302/301 方式重定向
	if (statusCode >= 300 && statusCode < 400) {
		targetUrl = reply->header(QNetworkRequest::LocationHeader).toString();
		qDebug() << "[+] 检测到 HTTP 30x 重定向，目标网址:" << targetUrl;
	} 
	// 如果校园网网关返回 200 OK 并使用 JS 强行跳转 (修改 href)
	else {
		QString content = QString::fromUtf8(reply->readAll());
		// 增加兼容 window.location 等情况
		QRegularExpression re("(?:document\\.location\\.href|window\\.location|location\\.href)\\s*=\\s*['\"]([^'\"]+)['\"]");
		QRegularExpressionMatch match = re.match(content);

		if (match.hasMatch()) {
			targetUrl = match.captured(1);
			qDebug() << "[+] 从 HTML 脚本中提取到的目标网址:" << targetUrl;
		} else {
			qDebug() << "[-] 未能在返回值中匹配到重定向地址。状态码:" << statusCode;
		}
	}

	reply->deleteLater();
	return targetUrl;
}
