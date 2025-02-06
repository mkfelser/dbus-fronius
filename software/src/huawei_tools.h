#ifndef HUAWEI_TOOLS_H
#define HUAWEI_TOOLS_H

#include <QVector>

void delay(int secs);
inline qint32 getIntValue(const QVector<quint16> &values, int offset) { return (values[offset] << 16) | values[offset + 1]; }
inline double getFloatValue(const QVector<quint16> &values, int offset) { return static_cast<double>(getIntValue(values, offset)); }


#endif // HUAWEI_TOOLS_H
