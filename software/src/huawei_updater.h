#ifndef INVERTER_MODBUS_HUAWEI_UPDATER_H
#define INVERTER_MODBUS_HUAWEI_UPDATER_H

#include "sunspec_updater.h"

class HuaweiSUN2000Updater : public SunspecUpdater
{
	Q_OBJECT
public:
	explicit HuaweiSUN2000Updater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent = 0);

private:
	virtual void readPowerAndVoltage();

	virtual bool parsePowerAndVoltage(QVector<quint16> values);
	
	void setInverterState(int sunSpecState);
};

#endif // INVERTER_MODBUS_HUAWEI_UPDATER_H
