#include "ConfigHelper.h"
#include<QSettings>
#include<QVariant>
#include<QString>
QVariant ConfigHelper::getSetting(const QString& key, const QVariant& defaultValue) {
    QSettings settings;
    return settings.value(key, defaultValue);
}
void ConfigHelper::setSetting(const QString& key, const QVariant& value) {
    QSettings settings;
    settings.setValue(key, value);
    settings.sync();
}