#pragma once
#ifndef NETWORKDETECTOR_H
#define NETWORKDETECTOR_H

#include <QObject>
#include"../mould/Singleton.h"
class QNetworkAccessManager;
class QNetworkConfigurationManager;
class QTimer;
class NetworkDetector : public QObject, public Singleton<NetworkDetector>
{
	Q_OBJECT
	friend class Singleton<NetworkDetector>;
public:
	// 定义三种网络状态
	enum NetworkState {
		OuterNetwork,       // 非校园网（家里/流量）
		CampusNeedsLogin,   // 校园网：未登录（被劫持）
		CampusOnline        // 校园网：已登录（可通外网）
	};
	Q_ENUM(NetworkState)

		

	// 触发探测的入口函数
	void startDetection();
	void initNetworkMonitor();
	
	// 处理网络状态变化的逻辑
	void handleNetworkStateChanged(bool isOnline);
	//解析校园网验证服务器ip
	QString parseAuthServerIp(const QString& portalUrl);
	QString parseServerUrl();
	
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
	NetworkDetector();
	// 内部的两步探测逻辑
	void probeGateway();       // 第一步：探测内网
	void probeCaptivePortal(); // 第二步：探测外网 204
	
	bool checkCurrentOnlineState();
	const QString CPD_URL = "http://connect.rom.miui.com/generate_204";
};
#endif // NETWORKDETECTOR_H
