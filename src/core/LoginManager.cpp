#include "loginManager.h"
#include"../models/DataMaid.h"
#include"../Entity/LoginDataEntity.h"
#include"../Entity/UserEntity.h"
#include<QNetworkAccessManager>
#include<QUrl>
#include <QNetworkRequest>
#include <QUrlQuery>
#include<QNetworkReply>
#include <QDebug>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslError>
#include<QJsonParseError>
#include<QJsonObject>

LoginManager::LoginManager()
	:networkManager(new QNetworkAccessManager(this))

{
	
}
JsonReturnEntity LoginManager::parseNetworkReply(const QString& responseString)
{
	// 假设 responseString 是从 QNetworkReply 读取到的完整字符串
	// 例如: "jsonpReturn({\"result\":0,\"msg\":\"终端IP已在线！\",\"ret_code\":2});"

	// 1. 查找 JSON 对象的起始 '{' 和结束 '}'
	int startIndex = responseString.indexOf('{');
	int endIndex = responseString.lastIndexOf('}');
	if (startIndex != -1 && endIndex != -1 && endIndex > startIndex) {
		// 截取纯 JSON 字符串
		QString jsonString = responseString.mid(startIndex, endIndex - startIndex + 1);

		// 2. 将字符串解析为 QJsonDocument
		QJsonParseError parseError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

		// 检查解析是否出错
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


void LoginManager::onLogin(const QString& username, const QString& password)
{
	qDebug() << "是否支持 SSL:" << QSslSocket::supportsSsl();
	qDebug() << "Qt 编译时使用的 SSL 版本:" << QSslSocket::sslLibraryBuildVersionString();
	qDebug() << "当前运行时的 SSL 版本:" << QSslSocket::sslLibraryVersionString();
	LoginDataEntiry loginData;
	loginData.user_account = username;
	loginData.user_password = password;
	//HACK: 这里的网络地址应该设置成方便修改的，不应该直接写死在代码里
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
	//伪造Host请求头，强制指定Host头为内网域名
	//HACK:伪造的Host请求头也不应该写死
	request.setRawHeader("Host", "yc.gxnu.edu.cn");

	//HACK:后续将登录的结果写进日志，并在界面上进行反馈，而不是简单地输出到调试控制台
	QNetworkReply* reply = networkManager->get(request);

	connect(reply, &QNetworkReply::sslErrors, this,
		[reply](const QList<QSslError>& errors) {
			for (const auto& error : errors) {
				qDebug() << "SSL error:" << error.errorString();
			}
		});
	/*只写变量名和=是按值捕获，写变量名和&是按引用捕获，按值捕获会在lambda表达式被调用时创建一个副本，
	而按引用捕获则会直接使用外部变量的引用。由于网络请求是异步的，按值捕获可以确保在lambda表达式被调用时，
    loginData的值不会因为外部变量的改变而发生变化，从而避免潜在的线程安全问题和数据不一致问题。*/
	connect(reply, &QNetworkReply::finished, this, [reply, loginData,this]() {
		const int statusCode =reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << "请求失败:" << reply->errorString() << "status:" << statusCode;
			reply->deleteLater();
			return;
		}

		const QByteArray body = reply->readAll();
		qDebug() << "HTTP状态码:" << statusCode;
		qDebug() << "响应体:" << QString::fromUtf8(body);

		// 这里再根据响应是否成功决定是否保存用户
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
