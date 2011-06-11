/**********************************************************************************************
 * Ember Library - Version 1
 * by Lower East Kitchen lowereastkitchen.com
 *
 * This Code is licensed under a Creative Commons Attribution-ShareAlike 3.0 License.
 **********************************************************************************************/

#include <WProgram.h>
#include <ember.h>

DoEvery::DoEvery(long _period) {
	period=_period;
	lastTime=0;
}

void DoEvery::reset() {
	lastTime=millis();
}

bool DoEvery::check() {
	if (millis()-lastTime > period) {
		lastTime+=period;
		return true;
	} else {
		return false;
	}
}

bool DoEvery::before(double threshTime) {
	if (millis()-lastTime < threshTime) {
		return true;
	} else {
		return false;
	}
}

/*LED Constructor in header *****************/

void LED::init() {
  //setup LED pins for output
  for(int i=2;i<=13;i++) {
    pinMode(i,OUTPUT);
  }
}

/*write (...)*********************************************************
 *    Takes a value and writes it to 4 digit, 7-seg display
 ***************************************************************************/
void LED::write(double writeValue, bool displayPos)
{
  //turn all digits off
  for(int i=2;i<=5;i++) {
    digitalWrite(i,LOW);
  }
  
  //increment digit
  writeDigit++;
  while(writeDigit>3) writeDigit-=4;
  
  //find digit value
  int digitVal=writeValue*10;
  int shiftCount=3-writeDigit;
  while (shiftCount>0) {
    digitVal=digitVal/10;
    shiftCount--;
  }
  digitVal=digitVal%10;
  
  //blank leading zeroes
  if((writeDigit==0)&&(writeValue<100)) digitVal=10;
  if((writeDigit==1)&&(writeValue<10)) digitVal=10;
  
  //write digit
  for(byte i;i<7;i++) {
    digitalWrite(i+6,1-seven_seg_digits[digitVal][i]);
  }
  
  //write decimal
  if((writeDigit==2)||displayPos) {
    digitalWrite(13,LOW);
  } else {
    digitalWrite(13,HIGH);
  }
  
  //turn correct digit on
  digitalWrite(2+writeDigit,HIGH);
  
  return;
}

/*Encoder Constructor ()    ***********************************************
 *    We do not need much
 ***************************************************************************/
Encoder::Encoder()
{
	old_AB = 0;
	pushed=false;
	displayPos=false;
	useF=false;
	displayTime=3000;//how long to display set temp
	position=25.0;
}

void Encoder::init()
{
	/* Setup encoder pins as inputs */
  pinMode(ENC_A, INPUT);
  digitalWrite(ENC_A, HIGH);
  pinMode(ENC_B, INPUT);
  digitalWrite(ENC_B, HIGH);
  pinMode(ENC_C, INPUT);
  digitalWrite(ENC_C, HIGH);
}

/* read ()******************************************************************
 *    Returns encoder position
 ***************************************************************************/
double Encoder::read()
{
    old_AB <<= 2;                   //remember previous state
    old_AB |= ( ENC_PORT & 0x03 );  //add current state
    int8_t encoderChange = enc_states[( old_AB & 0x0f )];
	if( encoderChange ) {
    	position += double(encoderChange*0.1);
	    displayPos=true;
	    lastKnobTurn=millis();
    }

  	if((millis()-displayTime>lastKnobTurn) && displayPos) {
    	displayPos=false;
    	save();//save setpoint in EEPROM
  	}
	
	if ((digitalRead(ENC_C)==LOW) && !(pushed)) {
	  switchCF();
	  pushed=true;
	}
	
	//return pushed state when released
	if ((digitalRead(ENC_C)==HIGH) && pushed) {
	  pushed=false;
	}

	if (useF) {
		return (position-32.0)*5.0/9.0;
	} else {
		return position;
	}

}

/* switchCF ()******************************************************************
 *    Switches values between deg C and F
 ***************************************************************************/
void Encoder::switchCF()
{
  if (useF) {
    position=(position-32.0)*5.0/9.0;
    useF=false;
  } else {
    position=position*9.0/5.0+32.0;
    useF=true;
  }
}

/* printTemp (...)**********************************************************
 *    Display argument as C or F, or display setpoint
 ***************************************************************************/
double Encoder::printTemp(double temp)
{
	if(displayPos) {
		return position;
	}  
	if(useF) {
		return temp*9.0/5.0+32.0;
	}
	return temp;
}


/* save (...)******************************************************
 *    Writes position and useF to EEPROM
 ***************************************************************************/
void Encoder::save() {
  if(useF) {
    EEPROM.write(0,(byte)1);
  } else {
    EEPROM.write(0,(byte)0);
  }
  short writeInt=(short)(position*10);
  EEPROM.write(1,(byte)((writeInt >> 8) & 0xff));
  EEPROM.write(2,(byte )(writeInt & 0xff));
  
  return;
}

/* recall () *********************************************************
 *    Reads position value from EEPROM
 ***************************************************************************/
double Encoder::recall() {
	byte firstbyte=EEPROM.read(0);
	if (firstbyte < 255) {
		if (firstbyte==1) useF=true;
		short savedPosition;
		savedPosition  = ((short)EEPROM.read(1))<<8;
		savedPosition |= EEPROM.read(2);
		position=savedPosition/10.0;
	}
	return position;
}

/* TempSensor Constructor () *************************************************
 *    Set a few defaults
 ***************************************************************************/
TempSensor::TempSensor() {
	temp=20.0;//guess room temperature to start
	temp_n=0;
	temp_n_max=20;
}

/* updateTemp () *********************************************************
 *  Takes in new temp in C. Returns running average temperature in C.
 ***************************************************************************/
double TempSensor::updateTemp(double newReading)
{
  //do time integration
  if(temp_n==0) {
    temp=newReading;
  } else {
    temp=temp*(temp_n-1)/temp_n+newReading/temp_n;
  }
  if (temp_n<temp_n_max) temp_n++;
  
  return temp;
}

NTCThermistor::NTCThermistor() {
	Bval=3988.0; // B value
	r_inf=0.0155223; //r infinity
	vDiv=3000.0; //voltage divider upper resistance
}

/* NTCThermistor::getTemp () ************************************************
 *  Takes a new temperature reading.
 ***************************************************************************/

double NTCThermistor::getTemp() {
  int reading=analogRead(THERM_PIN);
  //voltage divider; messy to avoid div by 0; fix?
  float resistance=vDiv/(1023.02/(reading+.01)-1.0);
  float temp_reading=Bval/log(resistance/r_inf)-273.15;
  return updateTemp(temp_reading);
}

RTDThermistor::RTDThermistor() {
	bridgeR1=3000.0;
	bridgeR2=100.0;
	bridgeR3=3000.0;
	ampFactor=20;
}

double RTDThermistor::getTemp() {
  int reading=analogRead(AMP_PIN);
  float voltage=reading/1023.0/ampFactor;
  float resistance=bridgeR3*(bridgeR1*voltage+bridgeR2*(voltage+1))/
              (bridgeR1-(bridgeR1+bridgeR2)*voltage);
  float temp_reading=(resistance/100.0-1.0)/.0039;//Pt100, fix to be more general

  return updateTemp(temp_reading);
}

Thermocouple::Thermocouple(char type) {
	ampFactor=200;
	//see http://srdata.nist.gov/its90/menu/menu.html inverse coefficients for data
	switch (type) {
		case 'k':
			d1=2.508355e1;
			d2=7.860106e-2;
			d3=-2.503131e-1;
			d4=8.315270e-2;
			d5=-1.228034e-2;
			d6=9.804036e-6;
			
			break;
	}
}

double Thermocouple::getTemp() {
	int reading=analogRead(AMP_PIN);
	float v=1000*reading/1023.0/ampFactor;//should be in mV
	float temp_reading=d1*v+d2*v*v+d3*pow(v,3)+d4*pow(v,4)+d5*pow(v,5)+d6*pow(v,6);
	
	return updateTemp(temp_reading);
}