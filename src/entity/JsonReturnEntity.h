#pragma once
#include <QString>
#include <QVariant>
struct JsonReturnEntity
{

	qint32 result;
	QString message;
	qint32 ret_code;
};
Q_DECLARE_METATYPE(JsonReturnEntity)