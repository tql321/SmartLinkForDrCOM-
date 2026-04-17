#include "SmartLinkForDrCOM.h"
#include"core/loginManager.h"
#include "ui_SmartLinkForDrCOM.h"
#include<QComboBox>
#include<QPushButton>
#include<iostream>
#include<algorithm>
#include<QLineEdit>
#include"models/DataMaid.h"
#include"models/ConfigHelper.h"
SmartLinkForDrCOM::SmartLinkForDrCOM(DataMaid *dataMaid, LoginManager *loginManager, QWidget *parent)
	: QMainWindow(parent)
	,ui(new Ui::SmartLinkForDrCOMClass)
	, m_dataMaid(dataMaid)
	, m_loginManager(loginManager)
{
	ui->setupUi(this);
	//初始化账号和密码输入框
	initUserUI();
	connect(ui->accountCB, &QComboBox::currentTextChanged, m_dataMaid, &DataMaid::curUsernameChanged);
	connect(ui->accountCB, QOverload<int>::of(&QComboBox::currentIndexChanged), m_dataMaid, &DataMaid::userItemChanged);
	connect(ui->passwordLE, &QLineEdit::textChanged, m_dataMaid, &DataMaid::curPasswordChanged);
	connect(ui->loginBtn, &QPushButton::clicked, this, [this]() {
		QString curUser = ui->accountCB->currentText();
		QString curPass = ui->passwordLE->text();
		m_loginManager->onLogin(curUser, curPass);
	});
	connect(m_dataMaid, &DataMaid::sigUsersChanged, this, &SmartLinkForDrCOM::usersChanged);
	connect(m_loginManager, &LoginManager::sigLoginSuccess, m_dataMaid, &DataMaid::loginSuccess);
	connect(m_dataMaid, &DataMaid::sigCurUsernameChanged, this, [this]() {
		ui->accountCB->setCurrentText(m_dataMaid->getCurUsername());
	});
	connect(m_dataMaid, &DataMaid::sigCurPasswordChanged, this, [this]() {
		ui->passwordLE->setText(m_dataMaid->getCurPassword());
	});
}

SmartLinkForDrCOM::~SmartLinkForDrCOM()
{
	delete ui;
}

void SmartLinkForDrCOM::usersChanged(QList<UserEntity>& users)
{
	//更新界面
	ui->accountCB->clear();
	for (const UserEntity& user : users) {
		ui->accountCB->addItem(user.username);
	}
	// 将用户列表保存到设置中
	ConfigHelper::setSetting("users", QVariant::fromValue(users));
}
void SmartLinkForDrCOM::initUserUI() {

	//更新用户列表
	for (const UserEntity& user : m_dataMaid->getUsers()) {
		ui->accountCB->addItem(user.username);
	}
	//从DataMaid中获取当前用户名和密码并更新界面
	ui->accountCB->setCurrentText(m_dataMaid->getCurUsername());
	ui->passwordLE->setText(m_dataMaid->getCurPassword());
}
