#pragma once
#include<QString>
#include<QObject>
#include"../Entity/UserEntity.h"
#include"../mould/Singleton.h"
class DataMaid :public QObject
{
	
	Q_OBJECT
		Q_PROPERTY(qint32 simulatedBrowseInterval MEMBER m_simulatedBrowseInterval NOTIFY sigSimulatedBrowseIntervalChanged)
		Q_PROPERTY(qint32 loginInterval MEMBER m_loginInterval NOTIFY sigLoginIntervalChanged)
		Q_PROPERTY(bool enableAutoStart MEMBER m_enableAutoStart NOTIFY sigEnableAutoStartChanged)
		Q_PROPERTY(QList<UserEntity> users MEMBER m_users NOTIFY sigUsersChanged)
		Q_PROPERTY(QString curUsername MEMBER m_curUsername NOTIFY sigCurUsernameChanged)
		Q_PROPERTY(QString curPassword MEMBER m_curPassword NOTIFY sigCurPasswordChanged)
public:
	DataMaid();
	~DataMaid() = default;
signals:

	void sigSimulatedBrowseIntervalChanged();
	void sigLoginIntervalChanged();
	void sigEnableAutoStartChanged();
	void sigCurUsernameChanged();
	void sigCurPasswordChanged();
	void sigUsersChanged(QList<UserEntity>& users);
	void sigUsersInit(QString& curUsername, QString& curPassword);
public slots:
	void curUsernameChanged(const QString& username);
	void curPasswordChanged(const QString& password);
	void loginSuccess(const UserEntity& user);
	void userItemChanged(int index);
public:
	void addUser(const UserEntity& user);
	void sortUsers();
	void memberIni();
	QList<UserEntity> getUsers() const { return m_users; };
	QString getCurUsername() const { return m_curUsername; };
	QString getCurPassword() const { return m_curPassword; };


private:
	qint32 m_simulatedBrowseInterval;
	qint32 m_loginInterval;
	bool m_enableAutoStart;
	QList<UserEntity> m_users;
	QString m_curUsername;
	QString m_curPassword;
};