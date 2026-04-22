#include "ConfigHelper.h"
#include<QSettings>
#include<QVariant>
#include<QString>
#include<QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QProcess>

namespace {
    QString getConfigPath() {
        // 先检查是否是以“便携模式”在软件根目录运行（比如旧版在Windows下的数据）
        QString localConfig = QCoreApplication::applicationDirPath() + "/config.ini";
        QFileInfo localFileInfo(localConfig);
        if (localFileInfo.exists() && localFileInfo.isWritable()) {
            return localConfig;
        }

        // 如果旧版配置不存在，或者在 Linux 打包环境下（只读目录）
        // 采用操作系统的标准用户配置路径 (Linux: ~/.config/APP_NAME, Win: AppData/Local/APP_NAME)
        QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir dir;
        if (!dir.exists(configDir)) {
            dir.mkpath(configDir);
        }
        return configDir + "/config.ini";
    }
}

QVariant ConfigHelper::getSetting(const QString& key, const QVariant& defaultValue) {
    QSettings settings(getConfigPath(), QSettings::IniFormat);
    return settings.value(key, defaultValue);
}

void ConfigHelper::setSetting(const QString& key, const QVariant& value) {
    QSettings settings(getConfigPath(), QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}

void ConfigHelper::removeAllSettings() {
    QSettings settings(getConfigPath(), QSettings::IniFormat);
    settings.clear();
    settings.sync();
}
