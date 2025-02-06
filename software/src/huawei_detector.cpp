#include "products.h"
#include "modbus_tcp_client.h"
#include "modbus_reply.h"
#include "sunspec_updater.h"
#include "huawei_detector.h"
#include "sunspec_tools.h"
#include "huawei_tools.h"

HuaweiSUN2000Detector::HuaweiSUN2000Detector(QObject *parent):
	AbstractDetector(parent),
	mUnitId(0)
{
}

HuaweiSUN2000Detector::HuaweiSUN2000Detector(quint8 unitId, QObject *parent):
	AbstractDetector(parent),
	mUnitId(unitId)
{
}

DetectorReply *HuaweiSUN2000Detector::start(const QString &hostName, int timeout)
{
	return start(hostName, timeout, mUnitId);
}

DetectorReply *HuaweiSUN2000Detector::start(const QString &hostName, int timeout, quint8 unitId)
{
	Q_ASSERT(unitId != 0);

	// If we already have a connection to this inverter, then there is
	// no need to scan it again.
	if (SunspecUpdater::hasConnectionTo(hostName, unitId)) {
		return 0;
	}

	ModbusTcpClient *client = new ModbusTcpClient(this);
	connect(client, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(client, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	client->setTimeout(timeout);
	client->connectToServer(hostName);
	Reply *reply = new Reply(this);
	reply->client = client;
	reply->di.networkId = unitId;
	reply->di.hostName = hostName;
	mClientToReply[client] = reply;
	return reply;
}

void HuaweiSUN2000Detector::onConnected()
{
	ModbusTcpClient *client = static_cast<ModbusTcpClient *>(sender());
	Reply *di = mClientToReply.value(client);
	Q_ASSERT(di != 0);
	di->currentRegister = HUAWEI_REG_MODEL;
	delay(2); // let's wait 2 sec, my inverter needs this time to accept the request
	startNextRequest(di, HUAWEI_REG_MODEL_SIZE);
}

void HuaweiSUN2000Detector::onDisconnected()
{
	ModbusTcpClient *client = static_cast<ModbusTcpClient *>(sender());
	Reply *di = mClientToReply.value(client);
	if (di != 0)
		setDone(di);
}

void HuaweiSUN2000Detector::onFinished()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	Reply *di = mModbusReplyToReply.take(reply);
	reply->deleteLater();

	QVector<quint16> values = reply->registers();

	switch (di->currentRegister) {
	case HUAWEI_REG_MODEL:
	{
		if (values.size() != (HUAWEI_REG_MODEL_SIZE) || getString(values, 0, 4) != "SUN2000-") {
			setDone(di);
			return;
		}
		di->di.retrievalMode = ProtocolHuaweiSUN2000;
		di->di.phaseCount = 3;
		di->di.productId = VE_PROD_ID_PV_INVERTER_HUAWEI;
		QString model = getString(values, 0, 15);
		di->di.productName = QString("Huawei %1").arg(model);
		di->di.uniqueId = di->di.serialNumber = getString(values, 15, 10);
		di->di.firmwareVersion = getString(values, 35, 15);
//		di->di.powerLimitScale = 100.0 / getScale(values, 0);

		di->currentRegister = HUAWEI_REG_POWER;
		startNextRequest(di, HUAWEI_REG_POWER_SIZE);
		return;
	}
	case HUAWEI_REG_POWER:
	{
		if (values.size() != HUAWEI_REG_POWER_SIZE) {
			setDone(di);
			return;
		}
		di->di.maxPower = 	static_cast<double>(getIntValue(values, 0, 2));
		di->currentRegister = HUAWEI_REG_POWER;
		checkDone(di);
		return;
	}
	}
	return;
}

void HuaweiSUN2000Detector::startNextRequest(Reply *di, quint16 regCount)
{
	ModbusReply *reply = di->client->readHoldingRegisters(di->di.networkId, di->currentRegister,
														  regCount);
	mModbusReplyToReply[reply] = di;
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void HuaweiSUN2000Detector::checkDone(Reply *di)
{
	if ( !di->di.productName.isEmpty() &&
			di->di.phaseCount > 0 &&
			di->di.networkId > 0)
		di->setResult();
	qDebug() << "found!! " << di->di.productName << " SN: "<< di->di.uniqueId << " FW: " << di->di.firmwareVersion << " pwr: " << di->di.maxPower;
			setDone(di);
}

void HuaweiSUN2000Detector::setDone(Reply *di)
{
	if (!mClientToReply.contains(di->client))
		return;
	di->setFinished();
	disconnect(di->client);
	mClientToReply.remove(di->client);
	di->client->deleteLater();
}

HuaweiSUN2000Detector::Reply::Reply(QObject *parent):
	DetectorReply(parent),
	client(0),
	currentRegister(0)
{
}

HuaweiSUN2000Detector::Reply::~Reply()
{
}
