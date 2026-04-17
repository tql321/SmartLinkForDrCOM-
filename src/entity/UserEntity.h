#pragma once
#include<QString>
#include<QDateTime>
#include<QList>
#include<QMetaType>
#include<QDataStream>

struct UserEntity
{
	QString username;
	QString password;
	QDateTime lastLoginTime;

	bool operator==(const UserEntity& other) const
	{
		return username == other.username
			&& password == other.password
			&& lastLoginTime == other.lastLoginTime;
	}
};

inline QDataStream& operator<<(QDataStream& out, const UserEntity& user)
{
	out << user.username << user.password << user.lastLoginTime;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, UserEntity& user)
{
	in >> user.username >> user.password >> user.lastLoginTime;
	return in;
}

Q_DECLARE_METATYPE(UserEntity)
Q_DECLARE_METATYPE(QList<UserEntity>)
