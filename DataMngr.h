// DataMngr.h

#ifndef _DATAMNGR_h
#define _DATAMNGR_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "EEPROM.h"
typedef enum MngrState
{
	Idle,
	Check,
	Value,
	ReadVal,
	SaveData
};

typedef enum DataBuffer
{
	tmp,
	Kp,
	Ki,
	Kd,
	speed
};

class DataMngr
{
public:
	void begin(int baud, bool debug = false);
	void run();
	bool go();

private:
	#define numData 4
	#define sizeData 4
	#define bufferSize numData*sizeData
	bool startLF=false;
	int add[bufferSize] = {0, 1, 2, 3,  //kp
				   4, 5, 6, 7,  //ki
				   8, 9, 10, 11,	//kd
				   12, 13, 14, 15}; //speed
	
	DataBuffer dataSelect;
	MngrState statusMngr;

	float tmpVal = 0;
	String tmpBuffer = "";
	char readbuffer[128];
	int head = 0;
	int tail = 0;

	bool _debug;

	float kp;
	float ki;
	float kd;
	float speed;

	void waitFor(String);
	int getAdd(DataBuffer slot);
	bool storeData(float data,DataBuffer slot);
	float readData(DataBuffer slot);
	void readData();

};

#endif
