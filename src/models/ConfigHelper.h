#pragma once
#include <QVariant>
#include <QObject>
class ConfigHelper:QObject {
public:
    static QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant());
    static void setSetting(const QString& key, const QVariant& value);
	static void removeAllSettings();
};
