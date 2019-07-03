#include "DataMngr.h"

union floatBin {
	float f;
	uint8_t bin[4];
} fnB;

void DataMngr::begin(int baud, bool debug)
{
	_debug = debug;
	Serial.begin(baud);
	kp = readData(DataBuffer::Kp);
	ki = readData(DataBuffer::Ki);
	kd = readData(DataBuffer::Kd);
	speed = readData(DataBuffer::speed);
}

void DataMngr::run()
{
	String data = "";
	switch (statusMngr)
	{
	case MngrState::Idle:
	{
		if (Serial.available())
		{
			head++;
			readbuffer[head] = Serial.read();
		}
		else
		{
			if (readbuffer[head] == '\n')
			{
				if (_debug)
				{
					Serial.print("Number of Char = ");
					Serial.println(head + 1);
				}
				tmpBuffer = "";
				for (; tail < head; tail++)
				{
					tmpBuffer += String(readbuffer[tail]);
					if (_debug)
					{
						Serial.print("Head : ");
						Serial.print(head);
						Serial.print("\tTail : ");
						Serial.println(tail);
					}
				}

				head = 0;
				tail = 0;

				statusMngr = MngrState::Check;
			}
		}
	}
	break;

	case MngrState::Check:
	{
		if (_debug)
		{
			Serial.println(tmpBuffer);
		}
		if (tmpBuffer.indexOf("Kp<") >= 0 || tmpBuffer.indexOf("kp<") >= 0)
		{
			dataSelect = DataBuffer::Kp;
			statusMngr = MngrState::Value;
		}
		else if (tmpBuffer.indexOf("Ki<") >= 0 || tmpBuffer.indexOf("ki<") >= 0)
		{
			dataSelect = DataBuffer::Ki;
			statusMngr = MngrState::Value;
		}
		else if (tmpBuffer.indexOf("Kd<") >= 0 || tmpBuffer.indexOf("kd<") >= 0)
		{
			dataSelect = DataBuffer::Kd;
			statusMngr = MngrState::Value;
		}
		else if (tmpBuffer.indexOf("speed<") >= 0 || tmpBuffer.indexOf("speed<") >= 0)
		{
			dataSelect = DataBuffer::speed;
			statusMngr = MngrState::Value;
		}
		else if (tmpBuffer.indexOf("Kp?") >= 0 || tmpBuffer.indexOf("kp?") >= 0)
		{
			dataSelect = DataBuffer::Kp;
			statusMngr = MngrState::ReadVal;
		}
		else if (tmpBuffer.indexOf("Ki?") >= 0 || tmpBuffer.indexOf("ki?") >= 0)
		{
			dataSelect = DataBuffer::Ki;
			statusMngr = MngrState::ReadVal;
		}
		else if (tmpBuffer.indexOf("Kd?") >= 0 || tmpBuffer.indexOf("kd?") >= 0)
		{
			dataSelect = DataBuffer::Kd;
			statusMngr = MngrState::ReadVal;
		}
		else if (tmpBuffer.indexOf("speed?") >= 0 || tmpBuffer.indexOf("speed?") >= 0)
		{
			dataSelect = DataBuffer::speed;
			statusMngr = MngrState::ReadVal;
		}
		else if (tmpBuffer.indexOf("Stop") >= 0 || tmpBuffer.indexOf("stop") >= 0)
		{
			startLF = false;
			tmpBuffer = "";
			statusMngr = MngrState::Idle;
		}
		else if (tmpBuffer.indexOf("Start") >= 0 || tmpBuffer.indexOf("start") >= 0)
		{
			startLF = true;
			tmpBuffer = "";
			statusMngr = MngrState::Idle;
		}
		else
		{
			tmpBuffer = "";
			statusMngr = MngrState::Idle;
		}
		break;
	}
	case MngrState::Value:
	{
		int start = tmpBuffer.indexOf("<") + 1;
		int stop = tmpBuffer.indexOf(">");

		for (; start < stop; start++)
		{
			data += tmpBuffer.charAt(start);
		}

		tmpVal = data.toFloat();

		if (_debug)
		{
			Serial.print("Value store : ");
			Serial.println(tmpVal);
		}
		tmpBuffer = "";
		if (tmpVal != 0)
			statusMngr = MngrState::SaveData;
		else
			statusMngr = MngrState::Idle;

		break;
	}
	case MngrState::SaveData:
	{
		if (_debug)
		{
			Serial.println("Saving...");
		}
		if (storeData(tmpVal, dataSelect))
		{
			readData(dataSelect);
		}
		statusMngr = MngrState::Idle;
		break;
	}
	case MngrState::ReadVal:
	{
		readData(dataSelect);
		statusMngr = MngrState::Idle;
	}
	}
}

bool DataMngr::go(){
	return startLF;
}
int DataMngr::getAdd(DataBuffer slot)
{
	int tmpAdd;

	if (slot == DataBuffer::Kp)
	{
		tmpAdd = add[0];
	}
	else if (slot == DataBuffer::Ki)
	{
		tmpAdd = add[0 + 4];
	}
	else if (slot == DataBuffer::Kd)
	{
		tmpAdd = add[0 + 8];
	}
	else if (slot == DataBuffer::speed)
	{
		tmpAdd = add[0 + 12];
	}
	else
	{
		tmpAdd = -1;
	}

	return tmpAdd;
}

bool DataMngr::storeData(float data, DataBuffer slot)
{
	int tmpAdd = getAdd(slot);
	uint8_t bytes[sizeData];
	*(float *)(bytes) = data;

	for (int i = 0; i < sizeData; i++)
	{
		EEPROM.write(tmpAdd + i, bytes[i]);
		fnB.bin[i] = EEPROM.read(tmpAdd + i);
	}
	if (data == fnB.f)
		return 1;
	else
		return 0;
}

float DataMngr::readData(DataBuffer slot)
{
	int tmpAdd = getAdd(slot);
	for (int i = 0; i < sizeData; i++)
	{
		fnB.bin[i] = EEPROM.read(tmpAdd + i);
	}
	if (slot == DataBuffer::Kp)
		kp = fnB.f;
	else if (slot == DataBuffer::Ki)
		ki = fnB.f;
	else if (slot == DataBuffer::Kd)
		kd = fnB.f;
	else if (slot == DataBuffer::speed)
		speed = fnB.f;
	if (_debug)
	{
		switch (slot)
		{
		case DataBuffer::Kp:
			Serial.print("Kp = ");
			break;
		case DataBuffer::Ki:
			Serial.print("Ki = ");
			break;
		case DataBuffer::Kd:
			Serial.print("Kd = ");
			break;
		case DataBuffer::speed:
			Serial.print("speed = ");
			break;
		}
		Serial.println(fnB.f);
	}
	return fnB.f;
}