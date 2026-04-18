#include "AccountManager.h"
#include "../models/DataMaid.h"
#include "../entity/LoginDataEntiry.h"
#include "../entity/UserEntity.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QDebug>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslError>
#include <QJsonParseError>
#include <QJsonObject>
#include <QNetworkInterface>

AccountManager::AccountManager()
	:networkManager(new QNetworkAccessManager(this))
{
}

JsonReturnEntity AccountManager::parseNetworkReply(const QString& responseString)
{
	int startIndex = responseString.indexOf('{');
	int endIndex = responseString.lastIndexOf('}');
	if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
		QString jsonString = responseString.mid(startIndex, endIndex - startIndex + 1);
		QJsonParseError parseError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

		if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
			QJsonObject jsonObj = jsonDoc.object();
			JsonReturnEntity resultEntity;
			if (jsonObj.contains("result")) {
				resultEntity.result = jsonObj.value("result").toInt();
			}
			if (jsonObj.contains("msg")) {
				resultEntity.message = jsonObj.value("msg").toString();
			}
			if (jsonObj.contains("ret_code")) {
				resultEntity.ret_code = jsonObj.value("ret_code").toInt();
			}
			return resultEntity;
		}
		else {
			qDebug() << "JSON 解析失败:" << parseError.errorString();
		}
	}
	else {
		qDebug() << "未能找到有效的 JSON 结构";
	}
	return JsonReturnEntity();
}

void AccountManager::onLogin(const QString& username, const QString& password)
{
	qDebug() << "是否支持 SSL:" << QSslSocket::supportsSsl();
	qDebug() << "Qt 编译时使用的 SSL 版本:" << QSslSocket::sslLibraryBuildVersionString();
	qDebug() << "当前运行时的 SSL 版本:" << QSslSocket::sslLibraryVersionString();
	LoginDataEntiry loginData;
	loginData.user_account = username;
	loginData.user_password = password;
	QUrl url("https://192.168.125.13:802/eportal/portal/login");
	QUrlQuery query;
	query.addQueryItem("user_account", loginData.user_account);
	query.addQueryItem("user_password", loginData.user_password);
	url.setQuery(query);
	QNetworkRequest request(url);
	QSslConfiguration sslConfig = request.sslConfiguration();
	sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
	request.setSslConfiguration(sslConfig);
	request.setPeerVerifyName("yc.gxnu.edu.cn");
	request.setRawHeader("Host", "yc.gxnu.edu.cn");

	QNetworkReply* reply = networkManager->get(request);

	connect(reply, &QNetworkReply::sslErrors, this,
		[reply](const QList<QSslError>& errors) {
			for (const auto& error : errors) {
				qDebug() << "SSL error:" << error.errorString();
			}
		});
	connect(reply, &QNetworkReply::finished, this, [reply, loginData, this]() {
		const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << "请求失败:" << reply->errorString() << "status:" << statusCode;
			reply->deleteLater();
			return;
		}

		const QByteArray body = reply->readAll();
		qDebug() << "HTTP状态码:" << statusCode;
		qDebug() << "响应体:" << QString::fromUtf8(body);

		JsonReturnEntity resultEntity = parseNetworkReply(QString::fromUtf8(body));
		if (resultEntity.result == 1) {
			UserEntity curUser;
			curUser.username = loginData.user_account;
			curUser.password = loginData.user_password;
			curUser.lastLoginTime = QDateTime::currentDateTime();
			emit sigLoginSuccess(curUser);
		}
		else {
			qDebug() << "登录失败，服务器返回的消息:" << resultEntity.message;
		}

		reply->deleteLater();
		});
}

void AccountManager::onLogout()
{
	QUrl url("https://192.168.125.13:802/eportal/portal/logout");
	QUrlQuery query;

	QString localIp = DATAMAID.getLocalIp();

	query.addQueryItem("wlan_user_ip", localIp);
	url.setQuery(query);
	QNetworkRequest request(url);
	QSslConfiguration sslConfig = request.sslConfiguration();
	sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
	request.setSslConfiguration(sslConfig);
	request.setPeerVerifyName("yc.gxnu.edu.cn");
	request.setRawHeader("Host", "yc.gxnu.edu.cn");

	QNetworkReply* reply = networkManager->get(request);
	connect(reply, &QNetworkReply::finished, this, [reply, this]() {
		const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << "请求失败:" << reply->errorString() << "status:" << statusCode;
			reply->deleteLater();
			return;
		}

		const QByteArray body = reply->readAll();
		qDebug() << "HTTP状态码:" << statusCode;
		qDebug() << "响应体:" << QString::fromUtf8(body);


		JsonReturnEntity resultEntity = parseNetworkReply(QString::fromUtf8(body));
		if (resultEntity.result == 1) {
			qDebug() << "注销成功，服务器返回的消息:" << resultEntity.message;
		}
		else {
			qDebug() << "注销失败，服务器返回的消息:" << resultEntity.message;
		}

		reply->deleteLater();
		});
}
