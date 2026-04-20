#pragma once
#include <QString>
#include <QObject>
#include "../entity/UserEntity.h"
#include "../entity/logDataEntity.h"
#include "../mould/Singleton.h"
#define DATAMAID DataMaid::instance()
class DataMaid : public QObject, public Singleton<DataMaid>
{
	
	Q_OBJECT
		Q_PROPERTY(qint32 simulatedBrowseInterval MEMBER m_simulatedBrowseInterval NOTIFY sigSimulatedBrowseIntervalChanged)
		Q_PROPERTY(bool enableAutoStart MEMBER m_enableAutoStart NOTIFY sigEnableAutoStartChanged)
		Q_PROPERTY(QList<UserEntity> users MEMBER m_users NOTIFY sigUsersChanged)
		Q_PROPERTY(QString curUsername MEMBER m_curUsername NOTIFY sigCurUsernameChanged)
		Q_PROPERTY(QString curPassword MEMBER m_curPassword NOTIFY sigCurPasswordChanged)
public:
	DataMaid();
signals:

	void sigSimulatedBrowseIntervalChanged();
	void sigLoginIntervalChanged();
	void sigEnableAutoStartChanged();
	void sigCurUsernameChanged();
	void sigCurPasswordChanged();
	void sigUsersChanged(QList<UserEntity>& users);
	void sigUsersInit(QString& curUsername, QString& curPassword);
	void sigEnableAutoLoginChanged();
	void sigEnableForceLoginChanged();
	void sigLogAdded(const LogDataEntity& log);
	void sigAuthServerIpChanged();
public slots:
	void curUsernameChanged(const QString& username);
	void curPasswordChanged(const QString& password);
	void loginSuccess(const UserEntity& user);
	void userItemChanged(int index);
	void enableAutoLoginChanged(bool checked);
	void enableForceLoginChanged(bool checked);
	void simulatedBrowseIntervalChanged(int value);
	void enableAutoStartChanged(bool value);
	void addLog(const LogDataEntity& log);
	void authServerIpChanged(const QString& ip);
	void networkTypeChanged(const QString& name, const QString& value);
public:
	void addUser(const UserEntity& user);
	void sortUsers();
	void memberIni();
	QList<UserEntity> getUsers() const { return m_users; };
	QString getCurUsername() const { return m_curUsername; };
	QString getCurPassword() const { return m_curPassword; };
	bool getEnableAutoStart() const { return m_enableAutoStart; };
	bool getEnableAutoLogin() const { return m_enableAutoLoginCB; };
	bool getEnableForceLogin() const { return m_enableForceLogin; };
	int getSimulatedBrowseInterval() const { return m_simulatedBrowseInterval; };
	bool getEnableAutoLoginCB() const { return m_enableAutoLoginCB; };
	QString getAuthServerIp()const{return m_authServerIp;}
	QString getAuthAddress()const { return m_authAddress; }
	QString getLocalIp() const;
	QString getlogoutAddress()const {return m_logoutAddress;}
	QString getNetworkType() const { return m_networkType; }
	

private:
	qint32 m_simulatedBrowseInterval;
	bool m_enableAutoStart;
	QList<UserEntity> m_users;
	QString m_curUsername;
	QString m_curPassword;
	bool m_enableAutoLoginCB;
	bool m_enableForceLogin;
	QString m_authServerIp;
	QString m_authAddress;
	QString m_logoutAddress;
	QString m_networkType;
};
