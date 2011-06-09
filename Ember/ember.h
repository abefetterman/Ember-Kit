#ifndef Ember_v1_h
#define Ember_v1_h
#define LIBRARY_VERSION	1.0.0

#include <inttypes.h>
#include <EEPROM.h>

class DoEvery
{
public:
	DoEvery(long);
	void reset();
	bool check();
	bool before(double);
private:
	unsigned long period;
	unsigned long lastTime;
};

class LED
{
public:
	inline LED() { writeDigit=0; }
	void init();
	void write(double, bool);
private:
	int writeDigit;
};

class Encoder
{
public:
	Encoder();
	void init();
	double read();
	void save();
	double recall();
	
	double printTemp(double);
	inline bool displayQ() { return displayPos; }
	
private:	
	void switchCF();

	double position;
	
	uint8_t old_AB;
	bool pushed;
	bool displayPos;
	bool useF;
	unsigned long lastKnobTurn;
	unsigned long displayTime;
};

class TempSensor
{
public:
	TempSensor();
	
	double updateTemp(double);
	//Sensor must implement getTemp
	virtual double getTemp() = 0;
private:
	double temp;
	int temp_n;
	int temp_n_max;
};

class NTCThermistor : public TempSensor
{
public:
	NTCThermistor();
	virtual double getTemp();
private:
	double Bval;
	double r_inf;
	double vDiv;
};

class RTDThermistor : public TempSensor
{
public:
	RTDThermistor();
	virtual double getTemp();
private:
	double bridgeR1;
	double bridgeR2;
	double bridgeR3;
	double ampFactor;
};

static uint8_t seven_seg_digits[11][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                                           { 0,1,1,0,0,0,0 },  // = 1
                                                           { 1,1,0,1,1,0,1 },  // = 2
                                                           { 1,1,1,1,0,0,1 },  // = 3
                                                           { 0,1,1,0,0,1,1 },  // = 4
                                                           { 1,0,1,1,0,1,1 },  // = 5
                                                           { 1,0,1,1,1,1,1 },  // = 6
                                                           { 1,1,1,0,0,0,0 },  // = 7
                                                           { 1,1,1,1,1,1,1 },  // = 8
                                                           { 1,1,1,0,0,1,1 },   // = 9
                                                           { 0,0,0,0,0,0,0 }   // = blank
                                                           };


static int8_t enc_states[16] = {0,0,-1,0,0,0,0,0,1,0,0,0,0,0,0,0};

//ports for encoder
#define ENC_A 14
#define ENC_B 15
#define ENC_PORT PINC
#define ENC_C 16

//port for temp sensor
#define THERM_PIN 18

//amplified input for RTD, Thermocouples
#define AMP_PIN 19

//port for PID output
#define PID_OUT 17

#endif