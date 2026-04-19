#include "NetworkDetector.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>
#include <QNetworkAccessManager>
#include"KeepLiveManager.h"
#include<QNetworkConfigurationManager>
#include"../models/DataMaid.h"
#include <QNetworkInterface>

// HACK:ip先写死了，后续可以改成自动获取默认网关地址
const QString GATEWAY_URL = "http://192.168.125.13";
// MIUI 204 探测接口
const QString CPD_URL = "http://connect.rom.miui.com/generate_204";

NetworkDetector::NetworkDetector(QNetworkAccessManager* netManager, QObject* parent)
	: QObject(parent)
	, m_netManager(netManager)
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
	// 自动登录检测定时器
	if (DATAMAID.getEnableAutoLogin()) {
		connect(m_autoLoginTimer, &QTimer::timeout, this, [this]() {
			bool currentOnline = checkCurrentOnlineState();
			if (currentOnline != m_lastOnlineState) {
				handleNetworkStateChanged(currentOnline);
			}
			});
		m_autoLoginTimer->start(5000); // 每5秒检查一次网络状态
	}
	if (DATAMAID.getEnableForceLogin()) {
		// 掉线强制重连检测定时器
		connect(m_forceLoginTimer, &QTimer::timeout, this, [this]() {
			if (DATAMAID.getEnableForceLogin()) {
				startDetection();
			}
			});
		m_forceLoginTimer->start(60000); // 每分钟检查一次网络状态
	}


	// 监听实时变更
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
	QNetworkRequest request((QUrl(GATEWAY_URL)));
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
