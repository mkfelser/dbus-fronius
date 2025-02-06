#include "froniussolar_api.h"
#include "data_processor.h"
#include "inverter.h"
#include "huawei_updater.h"
#include "inverter_settings.h"
#include "huawei_tools.h"


#define HUAWEI_REG_VOLTAGE 32066
#define HUAWEI_REG_VOLTAGE_SIZE (HUAWEI_REG_TOTAL_ENERGY-HUAWEI_REG_VOLTAGE+2)
#define HUAWEI_REG_L1_VOLTAGE 32069
#define HUAWEI_REG_L2_VOLTAGE 32070
#define HUAWEI_REG_L3_VOLTAGE 32071
#define HUAWEI_REG_L1_CURRENT 32072
#define HUAWEI_REG_L2_CURRENT 32074
#define HUAWEI_REG_L3_CURRENT 32076
#define HUAWEI_REG_ACTIVE_POWER 32080
#define HUAWEI_REG_STATUS 32089
#define HUAWEI_REG_TOTAL_ENERGY 32106

#define HUAWEI_REG_POWER_OFFSET (HUAWEI_REG_ACTIVE_POWER-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_STATUS_OFFSET (HUAWEI_REG_STATUS-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_TOTAL_ENERGY_OFFSET (HUAWEI_REG_TOTAL_ENERGY-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L1_VOLTAGE_OFFSET (HUAWEI_REG_L1_VOLTAGE-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L2_VOLTAGE_OFFSET (HUAWEI_REG_L2_VOLTAGE-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L3_VOLTAGE_OFFSET (HUAWEI_REG_L3_VOLTAGE-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L1_CURRENT_OFFSET (HUAWEI_REG_L1_CURRENT-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L2_CURRENT_OFFSET (HUAWEI_REG_L2_CURRENT-HUAWEI_REG_VOLTAGE)
#define HUAWEI_REG_L3_CURRENT_OFFSET (HUAWEI_REG_L3_CURRENT-HUAWEI_REG_VOLTAGE)


HuaweiSUN2000Updater::HuaweiSUN2000Updater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent):
	SunspecUpdater(limiter, inverter, settings, parent)
{
}

void HuaweiSUN2000Updater::setInverterState(int state)
{
	/*
	 * Status as returned by the fronius inverter
	 * - 0-6: Startup
	 * - 7: Running
	 * - 8: Standby
	 * - 9: Boot loading
	 * - 10: Error
	 * - 11: Running (MPPT)
	 * - 12: Running (Throttled)
	*/
	int froniusState = 0;
	switch (state) {
	case 0:
	case 1:
	case 2:
	case 3:
		froniusState = state;  // startup
		break;
	case 256:
		froniusState = 6;  // startup
		break;
	case 512:
		froniusState = 7;   // running
		break;
	case 40960:
		froniusState = 8;  // standby
		break;
	case 768:
		froniusState = 10;  // error
		break;
	case 513:
	case 514:
		froniusState = 12;
		break;
	default:
		inverter()->invalidateStatusCode();
		return;
	}
	inverter()->setStatusCode(froniusState);
}

void HuaweiSUN2000Updater::readPowerAndVoltage()
{
	readHoldingRegisters(HUAWEI_REG_VOLTAGE, HUAWEI_REG_VOLTAGE_SIZE);
}

bool HuaweiSUN2000Updater::parsePowerAndVoltage(QVector<quint16> values)
{
	const DeviceInfo &deviceInfo = inverter()->deviceInfo();
	Q_ASSERT(deviceInfo.retrievalMode == ProtocolHuaweiSUN2000);
	if (values.size() != HUAWEI_REG_VOLTAGE_SIZE )
		return false;
	CommonInverterData cid;
	cid.acCurrent = getFloatValue(values, HUAWEI_REG_L1_CURRENT_OFFSET) + getFloatValue(values, HUAWEI_REG_L2_CURRENT_OFFSET) + getFloatValue(values, HUAWEI_REG_L3_CURRENT_OFFSET);
	cid.acPower = getFloatValue(values, HUAWEI_REG_POWER_OFFSET);
	// huawei does not provide a voltage for the system as a whole. This does not
	// make a lot of sense. Since previous versions of dbus-fronius published this
	// value (retrieved via the Solar API) we use the value from phase 1.
	cid.acVoltage = static_cast<double>(values[HUAWEI_REG_L1_VOLTAGE_OFFSET])/10;
	cid.totalEnergy = getFloatValue(values, HUAWEI_REG_TOTAL_ENERGY_OFFSET)*10;
	processor()->process(cid);

	if (deviceInfo.phaseCount > 1) {
		ThreePhasesInverterData tpid;
		tpid.acCurrentPhase1 = getFloatValue(values, HUAWEI_REG_L1_CURRENT_OFFSET)/1000;
		tpid.acCurrentPhase2 = getFloatValue(values, HUAWEI_REG_L2_CURRENT_OFFSET)/1000;
		tpid.acCurrentPhase3 = getFloatValue(values, HUAWEI_REG_L3_CURRENT_OFFSET)/1000;
		tpid.acVoltagePhase1 = static_cast<double>(values[HUAWEI_REG_L1_VOLTAGE_OFFSET])/10;
		tpid.acVoltagePhase2 = static_cast<double>(values[HUAWEI_REG_L2_VOLTAGE_OFFSET])/10;
		tpid.acVoltagePhase3 = static_cast<double>(values[HUAWEI_REG_L3_VOLTAGE_OFFSET])/10;
		processor()->process(tpid);
	} else if (settings()->phase() == MultiPhase) {
			// A single phase inverter used as a Multiphase
			// generator. This only makes sense in a split-phase
			// system. Typical in North America, and fully
			// supported by Fronius.
		updateSplitPhase(cid.acPower/2, cid.totalEnergy/2);
	}
	setInverterState(values[HUAWEI_REG_STATUS_OFFSET]);
	return true;
}

