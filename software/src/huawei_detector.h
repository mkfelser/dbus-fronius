#ifndef HUAWEI_DETECTOR_H
#define HUAWEI_DETECTOR_H

#include <QAbstractSocket>
#include <QHash>
#include "abstract_detector.h"
#include "defines.h"

class ModbusReply;
class ModbusTcpClient;

#define HUAWEI_REG_MODEL 30000
#define HUAWEI_REG_MODEL_SIZE (15+10+10+15)
#define HUAWEI_REG_RATED_POWER 30073
#define HUAWEI_REG_RATED_POWER_SIZE 2
	

class HuaweiSUN2000Detector : public AbstractDetector
{
	Q_OBJECT
public:
	HuaweiSUN2000Detector(QObject *parent = 0);

	HuaweiSUN2000Detector(quint8 unitId, QObject *parent = 0);

	DetectorReply *start(const QString &hostName, int timeout) override;
	DetectorReply *start(const QString &hostName, int timeout, quint8 unitId);
/*
	quint8 unitId() const
	{
		return mUnitId;
	}

	void setUnitId(quint8 unitId)
	{
		mUnitId = unitId;
	}
*/
private slots:
	void onConnected();

	void onDisconnected();

	void onFinished();

private:
	class Reply : public DetectorReply
	{
	public:
		Reply(QObject *parent = 0);

		virtual ~Reply();

		QString hostName() const override
		{
			return di.hostName;
		}
		void setResult()
		{
			emit deviceFound(di);
		}

		void setFinished()
		{
			emit finished();
		}

		DeviceInfo di;
		ModbusTcpClient *client;
		quint16 currentRegister;
	};

	void startNextRequest(Reply *di, quint16 regCount);

	void checkDone(Reply *di);
	void setDone(Reply *di);

	QHash<ModbusTcpClient *, Reply *> mClientToReply;
	QHash<ModbusReply *, Reply *> mModbusReplyToReply;

	quint8 mUnitId;
};

#endif // HUAWEI_DETECTOR_H
