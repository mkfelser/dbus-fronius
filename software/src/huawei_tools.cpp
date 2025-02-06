#include "huawei_tools.h"

#include <QTime>
#include <QTimer>
#include <QEventLoop>


void delay(int seconds) {
    QEventLoop loop;
	QTimer::singleShot(seconds * 1000, &loop, SLOT(quit()));
    loop.exec();
}

qint32 getIntValue(const QVector<quint16> &values, int offset, int size)
{
	Q_ASSERT(size == 2);

	qint32 v = 0;
	for (int i=0; i<size; ++i) {
		v = (v << 16) | values[offset + i];
	}
    return v;
}
