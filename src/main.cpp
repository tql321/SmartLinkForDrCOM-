#include "SmartLinkForDrCOM.h"
#include <QtWidgets/QApplication>
#include "Entity/UserEntity.h"
#include "models/DataMaid.h"
#include"core/LoginManager.h"
int main(int argc, char *argv[])
{
	qRegisterMetaType<QList<UserEntity>>("QList<UserEntity>");
	qRegisterMetaTypeStreamOperators<QList<UserEntity>>("QList<UserEntity>");
	qRegisterMetaType<UserEntity>("UserEntity");
	qRegisterMetaTypeStreamOperators<UserEntity>("UserEntity");

	QApplication app(argc, argv);
	//将应用程序的组织名称和应用程序名称设置为常量，以便在其他地方使用
	QApplication::setOrganizationName(ORG_NAME);
	QApplication::setApplicationName(APP_NAME);
	DataMaid dataMaid;
	LoginManager loginManager;
	SmartLinkForDrCOM window(&dataMaid, &loginManager);
	window.show();
	return app.exec();
}
