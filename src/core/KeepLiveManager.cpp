#include "KeepLiveManager.h"
#include "KeepLiveManager.h"
#include "KeepLiveManager.h"
#include "KeepLiveManager.h"
#include "KeepLiveManager.h"
#include "AccountManager.h"
#include "../models/DataMaid.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include"NetworkDetector.h"


// 恢复的占位符结构：由于之前未经追踪的 KeepLiveManager 源码被 git clean 意外删除，
// 此处保留它在数据库中的原型供后续开发。
KeepLiveManager::KeepLiveManager()
    : m_browseTimer(new QTimer(this)),
    m_networkManager(new QNetworkAccessManager(this)),
    m_networkDetector(new NetworkDetector(m_networkManager, this))

{
    startBrowseTimer();
    // 检测网络状态变化的结果，并根据结果调整保活策略
    connect(m_networkDetector, &NetworkDetector::sigDetectionFinished,
        this, &KeepLiveManager::handleNetworkState);

}

void KeepLiveManager::handleNetworkState(NetworkDetector::NetworkState state)
{
    switch (state) {
    case NetworkDetector::OuterNetwork:
        qDebug() << "当前处于外部网络（如家庭宽带）。无需操作。";
        stopBrowseTimer(); // 确保心跳关闭
		//TODO:关闭自动登录（如果有的话）
        break;

    case NetworkDetector::CampusNeedsLogin:
        qDebug() << "处于校园网，且未认证！准备自动登录...";

        // 检查用户是否开启了“自动登录”设置
        if (DATAMAID.getEnableAutoLogin()) { // 假设你有个获取配置的接口
            QString user = DATAMAID.getCurUsername();
            QString pwd = DATAMAID.getCurPassword();
            if (!user.isEmpty() && !pwd.isEmpty()) {
                AccountManager::instance().onLogin(user, pwd);
            }
            else {
                qDebug() << "账号密码为空，等待用户手动输入。";
            }
        }
        break;

    case NetworkDetector::CampusOnline:
        qDebug() << "✅ 校园网已认证通畅！启动保活心跳...";
        // 验证成功后开始模拟浏览定时器
        startBrowseTimer();
        break;
    }
}


void KeepLiveManager::startBrowseTimer()
{
    if (m_browseTimer->isActive()) {
        return;
    }
    // 将分钟转换为毫秒
    int intervalMs = DATAMAID.getSimulatedBrowseInterval() * 60 * 1000;
    m_browseTimer  ->start(intervalMs);

    qDebug() << "保活服务已启动，每" << DATAMAID.getSimulatedBrowseInterval() << "分钟模拟一次上网行为。";

    // 启动时立刻执行一次，确保马上刷新活跃状态
    sendHeartbeat();
}

void KeepLiveManager::stopBrowseTimer()
{
    if (m_browseTimer->isActive()) {
        m_browseTimer->stop();
        qDebug() << "保活服务已停止。";
    }
}


void KeepLiveManager::sendHeartbeat()
{
    if (!m_networkManager) return;

    // 使用极轻量的 204 接口（这里以 MIUI 连通性测试接口为例）
    QUrl url("http://connect.rom.miui.com/generate_204");
    QNetworkRequest request(url);

    QNetworkReply* reply = m_networkManager->get(request);

    // 【兼容 Qt5 的超时控制】
    // 设置 5 秒超时，如果发不出去就强行掐断
    QTimer* timeoutTimer = new QTimer(reply);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, reply, [reply]() {
        if (reply->isRunning()) {
            qDebug() << "保活请求超时...";
            reply->abort();
        }
        });
    timeoutTimer->start(5000);

    // 处理请求结果
    connect(reply, &QNetworkReply::finished, this, [reply, timeoutTimer]() {
        timeoutTimer->stop();

        if (reply->error() == QNetworkReply::NoError) {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (statusCode == 204) {
                qDebug() << "[" << QDateTime::currentDateTime().toString("HH:mm:ss") << "] 模拟上网成功，保持在线。";
            }
            else {
                qDebug() << "警告：收到非 204 状态码 (" << statusCode << ")，可能已被重定向到登录页。";
            }
        }
        else if (reply->error() != QNetworkReply::OperationCanceledError) {
            // 忽略因超时 abort 引发的 OperationCanceledError，打印真实的错误
            qDebug() << "模拟上网失败：" << reply->errorString();
        }

        reply->deleteLater();
        });
}
