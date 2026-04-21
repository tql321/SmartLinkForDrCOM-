#include "SmartLinkForDrCOM.h"
#include "core/AccountManager.h"
#include "ui_SmartLinkForDrCOM.h"
#include <QComboBox>
#include <QPushButton>
#include <iostream>
#include <algorithm>
#include <QLineEdit>
#include "models/DataMaid.h"
#include "models/ConfigHelper.h"
#include "core/SystemTrayManager.h"
#include <QApplication>
#include <QSpinBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include"core/NetworkDetector.h"
SmartLinkForDrCOM::SmartLinkForDrCOM(DataMaid* dataMaid, AccountManager* accountManager, QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::SmartLinkForDrCOMClass)
	, m_dataMaid(dataMaid)
	, m_accountManager(accountManager)
{
	ui->setupUi(this);
	
	initUI();
	connect(ui->accountCB, &QComboBox::currentTextChanged, m_dataMaid, &DataMaid::curUsernameChanged);
	connect(ui->accountCB, QOverload<int>::of(&QComboBox::currentIndexChanged), m_dataMaid, &DataMaid::userItemChanged);
	connect(ui->passwordLE, &QLineEdit::textChanged, m_dataMaid, &DataMaid::curPasswordChanged);
	connect(ui->networkTypeCB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		if (index >= 0) {
			m_dataMaid->networkTypeChanged(ui->networkTypeCB->itemText(index), ui->networkTypeCB->itemData(index).toString());
		}
	});
	connect(ui->authServerIpE, &QLineEdit::textChanged, m_dataMaid, &DataMaid::authServerIpChanged);
	connect(ui->loginBtn, &QPushButton::clicked, this, [this]() {
		QString curUser = ui->accountCB->currentText();
		QString curPass = ui->passwordLE->text();
		m_accountManager->onLogin(curUser, curPass);
	});
	 connect(ui->logoutBtn, &QPushButton::clicked, this, [this]() {
	 m_accountManager->onLogout();
	 });
	connect(ui->simulatedBrowseIntervalSB, QOverload<int>::of(&QSpinBox::valueChanged), m_dataMaid, &DataMaid::simulatedBrowseIntervalChanged);
	connect(ui->enableAutoStartCB, &QCheckBox::toggled, m_dataMaid, &DataMaid::enableAutoStartChanged);
	connect(ui->enableAutoLoginCB, &QCheckBox::toggled, m_dataMaid, &DataMaid::enableAutoLoginChanged);
	connect(ui->enableForceLogin, &QCheckBox::toggled, m_dataMaid, &DataMaid::enableForceLoginChanged);
	connect(ui->parseIpBtn, &QPushButton::clicked, this, [this]() {
		QString parsedIp = NetworkDetector::instance().parseAuthServerIp(
			NetworkDetector::instance().parseServerUrl());
		if (!parsedIp.isEmpty()) {
			ui->authServerIpE->setText(parsedIp);
		} else {
			qDebug() << "解析 Portal IP 失败，保留原配置。";
		}
	});
	connect(m_dataMaid, &DataMaid::sigUsersChanged, this, &SmartLinkForDrCOM::usersChanged);
	connect(m_accountManager, &AccountManager::sigLoginSuccess, m_dataMaid, &DataMaid::loginSuccess);
	connect(m_dataMaid, &DataMaid::sigCurUsernameChanged, this, [this]() {
		ui->accountCB->setCurrentText(m_dataMaid->getCurUsername());
		});
	connect(m_dataMaid, &DataMaid::sigCurPasswordChanged, this, [this]() {
		ui->passwordLE->setText(m_dataMaid->getCurPassword());
	});
	connect(m_dataMaid, &DataMaid::sigEnableAutoLoginChanged, this, [this]() {
		ui->enableAutoLoginCB->setChecked(m_dataMaid->getEnableAutoLogin());
	});
	connect(m_dataMaid, &DataMaid::sigEnableForceLoginChanged, this, [this]() {
		ui->enableForceLogin->setChecked(m_dataMaid->getEnableForceLogin());
	});
	connect(m_dataMaid, &DataMaid::sigLogAdded, this, [this](const LogDataEntity& log) {
		QString logStr = QString("[%1] [%2] %3").arg(log.logTime).arg(log.logLevel).arg(log.logMessage);
		ui->logPT->appendPlainText(logStr);
	});

	m_trayManager = new SystemTrayManager(this);

	connect(m_trayManager, &SystemTrayManager::showWindowRequested, this, [this]() {
		this->showNormal();
		this->activateWindow();
		});
	connect(m_trayManager, &SystemTrayManager::quitRequested, qApp, &QApplication::quit);
	
}

SmartLinkForDrCOM::~SmartLinkForDrCOM()
{
	delete ui;
}

void SmartLinkForDrCOM::usersChanged(QList<UserEntity>& users)
{
	ui->accountCB->blockSignals(true);
	ui->accountCB->clear();
	for (const UserEntity& user : users) {
		ui->accountCB->addItem(user.username);
	}
}

void SmartLinkForDrCOM::initUI() {
	this->setWindowIcon(QIcon(":/resources/logo/logo.png"));
	for (const UserEntity& user : m_dataMaid->getUsers()) {
		ui->accountCB->addItem(user.username);
	}
	ui->accountCB->setCurrentText(m_dataMaid->getCurUsername());
	ui->passwordLE->setText(m_dataMaid->getCurPassword());
	ui->simulatedBrowseIntervalSB->setValue(m_dataMaid->getSimulatedBrowseInterval());
	ui->enableAutoStartCB->setChecked(m_dataMaid->getEnableAutoStart());
	ui->enableAutoLoginCB->setChecked(m_dataMaid->getEnableAutoLogin());
	ui->enableForceLogin->setChecked(m_dataMaid->getEnableForceLogin());
	ui->authServerIpE->setText(m_dataMaid->getAuthServerIp());
	QMap<QString, QString>networkType;
	networkType.insert(QString::fromUtf8("校园网"), "");
	networkType.insert(QString::fromUtf8("中国电信"), "@ctc");
	networkType.insert(QString::fromUtf8("中国联通"), "@cuc");
	networkType.insert(QString::fromUtf8("中国移动"), "@cmc");
	ui->networkTypeCB->clear();
	for (auto it = networkType.begin(); it != networkType.end(); ++it) {
		ui->networkTypeCB->addItem(it.key(), it.value());
		if (m_dataMaid->getNetworkType() == it.value()) {
			ui->networkTypeCB->setCurrentText(it.key());
		}
	}

}

void SmartLinkForDrCOM::closeEvent(QCloseEvent* event)
{
	if (m_trayManager->isVisible()) {
		this->hide();
		event->ignore();
		m_trayManager->showMessage(QString::fromUtf8("提示"), QString::fromUtf8("程序已最小化到托盘运行"));
	}
	else {
		event->accept();
	}
}
