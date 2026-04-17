#pragma once

#include <QtWidgets/QMainWindow>
#include<QList>
#include "entity/UserEntity.h" // 引入 UserEntity 的定义

namespace Ui { class SmartLinkForDrCOMClass; }
class LoginManager;
class DataMaid;
class SmartLinkForDrCOM : public QMainWindow
{
    Q_OBJECT

public:
    SmartLinkForDrCOM(DataMaid *dataMaid,LoginManager *loginManager,QWidget *parent = nullptr);
    ~SmartLinkForDrCOM();
	void initUserUI();
public slots:

    void usersChanged(QList<UserEntity>& users);

private:
    Ui::SmartLinkForDrCOMClass *ui;
	DataMaid* m_dataMaid;
	LoginManager* m_loginManager;
};

