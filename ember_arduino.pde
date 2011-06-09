#include <PID_v1.h>
#include <EEPROM.h>
#include <ember.h>                                 

//set pid window size
#define PID_WINDOW_SIZE 3000

DoEvery tempTimer(100);
DoEvery pidTimer(PID_WINDOW_SIZE);

double temp, setpoint, output;

PID pid(&temp, &output, &setpoint,200,0.2,0,DIRECT);
Encoder encoder;
LED led;
NTCThermistor tempProbe;

void setup()
{  
  //initialize ports and timers
  encoder.init();
  led.init();
  
  tempTimer.reset();
  pidTimer.reset();
  
  //initialize the PID variables
  pid.SetOutputLimits(0,PID_WINDOW_SIZE);
  
  temp=tempProbe.getTemp();
  setpoint=encoder.recall();
  
  //prepare PID port for writing
  pinMode(PID_OUT, OUTPUT);  

  //turn the PID on and start windows
  pid.SetMode(AUTOMATIC);
  
  //setup serial for output
  Serial.begin (115200);
}
 
void loop()
{
   //First update the LED display
   led.write(encoder.printTemp(temp), encoder.displayQ());
   
   //Check the encoder for the setpoint
   setpoint=encoder.read();
   
   //every 100 ms, update the temperature
   if(tempTimer.check()) temp=tempProbe.getTemp();
   
   //compute new PID parameters
   pid.Compute();
   
   //output on pin PID_OUT based on PID output and timer
   pidTimer.check();
   if(pidTimer.before(output)) digitalWrite(PID_OUT,HIGH);
   else digitalWrite(PID_OUT,LOW);
   
   delay(3);
}
