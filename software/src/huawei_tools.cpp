#include "huawei_tools.h"

#include <QTime>
#include <QTimer>
#include <QEventLoop>


void delay(int seconds) {
    QEventLoop loop;
	QTimer::singleShot(seconds * 1000, &loop, SLOT(quit()));
    loop.exec();
}