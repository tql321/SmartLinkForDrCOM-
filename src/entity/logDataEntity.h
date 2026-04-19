#pragma once
#include <QString>
struct LogDataEntity
{
	QString logTime;
	QString logLevel;
	QString logMessage;
};
Q_DECLARE_METATYPE(LogDataEntity)
