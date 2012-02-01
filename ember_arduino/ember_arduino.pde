#include <PID_v1.h>
#include <EEPROM.h>
#include <ember.h>                                 

#define EMBER_VERSION 2

//set pid window size
#define PID_WINDOW_SIZE 3000

//aggressive PID parameters
#define KP_AGR 3000
#define KI_AGR 0
#define KD_AGR 0

//conservative PID parameters
#define KP_CONS 1500
#define KI_CONS 1
#define KD_CONS 0

DoEvery tempTimer(100);
DoEvery pidTimer(3000);

double temp, setpoint, output;

PID pid(&temp, &output, &setpoint,KP_AGR,KI_AGR,KD_AGR,DIRECT);
Encoder encoder;
LED led;

#if EMBER_VERSION == 1
  NTCThermistor tempProbe(10000, 3380); 
  //RTDThermistor tempProbe; //for Pt-100
#else
  //assume version 2
  NTCThermistor tempProbe(10000, 3988);

#endif

void setup()
{  
  //initialize ports and timers
  encoder.init(); 
  led.init();
  tempProbe.init();
  
  tempTimer.reset();
  pidTimer.reset();
  
  //initialize the PID variables
  pid.SetOutputLimits(0,PID_WINDOW_SIZE);
  
  temp=tempProbe.getTemp();
  setpoint=encoder.recall();
  
  //prepare PID port for writing
  pinMode(PID_OUT, OUTPUT);  

  //turn the PID on
  pid.SetMode(AUTOMATIC);
  
  //setup serial for output
//  Serial.begin (115200);
}
 
void loop()
{
   //First update the LED display
   led.write(encoder.printTemp(temp), encoder.displayQ());
   
   //Check the encoder for the setpoint
   setpoint=encoder.read();
   
   //every 100 ms, update the temperature
   if(tempTimer.check()) temp=tempProbe.getTemp();

	//use conservative values if within 3 deg, aggressive if >3 deg
	//only update once every pid period
   if (pidTimer.check()) {
		if (abs(setpoint-temp)<3) {
			pid.SetTunings(KP_CONS, KI_CONS, KD_CONS);
		} else  {
			pid.SetTunings(KP_AGR, KI_AGR, KD_AGR);
		}
   }
   
   //compute new PID parameters
   pid.Compute();

   //output on pin PID_OUT based on PID output and timer
   if(pidTimer.before(output)) digitalWrite(PID_OUT,HIGH);
   else digitalWrite(PID_OUT,LOW);
   
   delay(3);
}
