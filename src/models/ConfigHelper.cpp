#include "ConfigHelper.h"
#include<QSettings>
#include<QVariant>
#include<QString>
#include<QCoreApplication>

QVariant ConfigHelper::getSetting(const QString& key, const QVariant& defaultValue) {
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    return settings.value(key, defaultValue);
}

void ConfigHelper::setSetting(const QString& key, const QVariant& value) {
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}
