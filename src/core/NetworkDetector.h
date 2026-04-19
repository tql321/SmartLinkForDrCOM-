#pragma once
#ifndef NETWORKDETECTOR_H
#define NETWORKDETECTOR_H

#include <QObject>
#include"../mould/Singleton.h"
class QNetworkAccessManager;
class QNetworkConfigurationManager;
class QTimer;
class NetworkDetector : public QObject
{
	Q_OBJECT
public:
	// 定义三种网络状态
	enum NetworkState {
		OuterNetwork,       // 非校园网（家里/流量）
		CampusNeedsLogin,   // 校园网：未登录（被劫持）
		CampusOnline        // 校园网：已登录（可通外网）
	};
	Q_ENUM(NetworkState)

		explicit NetworkDetector(QNetworkAccessManager* netManager, QObject* parent = nullptr);

	// 触发探测的入口函数
	void startDetection();
	void initNetworkMonitor();
	
	// 处理网络状态变化的逻辑
	void handleNetworkStateChanged(bool isOnline);
	
signals:
	// 探测完成时发出此信号
	void sigDetectionFinished(NetworkDetector::NetworkState state);

private:
	QNetworkAccessManager* m_netManager;
	QNetworkConfigurationManager* m_netConfigManager;
	bool m_lastOnlineState = true;
	QTimer* m_forceLoginTimer;
	QTimer* m_autoLoginTimer;
private:
	// 内部的两步探测逻辑
	void probeGateway();       // 第一步：探测内网
	void probeCaptivePortal(); // 第二步：探测外网 204
	
	bool checkCurrentOnlineState();
};
#endif // NETWORKDETECTOR_H
