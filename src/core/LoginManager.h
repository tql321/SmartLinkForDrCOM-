#pragma once
#include <QObject>
#include"../entity/JsonReturnEntity.h"
class QNetworkAccessManager;
struct UserEntity;
class LoginManager :public QObject
{
	Q_OBJECT
public:
	JsonReturnEntity parseNetworkReply(const QString& responseString);
	LoginManager();
	~LoginManager() = default;
	
signals:
	void sigLoginSuccess(const UserEntity& user);

public slots:
	void onLogin(const QString& username, const QString& password);
private:
	//避免在登录过程中创建多个网络访问管理器实例，导致资源浪费和潜在的冲突，因此将其作为LoginManager的成员变量，并在构造函数中进行初始化
	QNetworkAccessManager* networkManager;
};
