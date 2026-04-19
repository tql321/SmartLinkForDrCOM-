#include "SmartLinkForDrCOM.h"
#include <QtWidgets/QApplication>
#include "entity/UserEntity.h"
#include "models/DataMaid.h"
#include "core/AccountManager.h"
#include "core/KeepLiveManager.h"
#include <QDateTime>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	LogDataEntity logData;
	logData.logTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	switch (type) {
	case QtDebugMsg:
		logData.logLevel = "Debug";
		break;
	case QtInfoMsg:
		logData.logLevel = "Info";
		break;
	case QtWarningMsg:
		logData.logLevel = "Warning";
		break;
	case QtCriticalMsg:
		logData.logLevel = "Critical";
		break;
	case QtFatalMsg:
		logData.logLevel = "Fatal";
		break;
	}
	logData.logMessage = msg;
	DATAMAID.addLog(logData);
}

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QApplication::setOrganizationName(ORG_NAME);
	QApplication::setApplicationName(APP_NAME);
	qRegisterMetaType<QList<UserEntity>>("QList<UserEntity>");
	qRegisterMetaTypeStreamOperators<QList<UserEntity>>("QList<UserEntity>");
	qRegisterMetaType<UserEntity>("UserEntity");
	qRegisterMetaTypeStreamOperators<UserEntity>("UserEntity");
	qRegisterMetaType<LogDataEntity>("LogDataEntity");

	qInstallMessageHandler(customMessageHandler);
	

	// Because they inherit Singleton, we pass their instances directly
	SmartLinkForDrCOM window(&DataMaid::instance(), &AccountManager::instance());
	window.show();

	// Start Keep-Alive tasks if needed
	KeepLiveManager::instance();

	return app.exec();
}
