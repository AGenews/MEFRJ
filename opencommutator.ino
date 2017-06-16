 //    OpenCommutator Control Software
//    Copyright (C) 2015 Andreas Genewsky
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    This sketch is inspired by Josh Siegle the OpenEphys community and
//    the Twister sketch. Thanks!		


#include <math.h>

// ------------------------------ //
//    SOFTWARE VERSION NUMBER:
int ver[2] = {
  1, 2};
// ------------------------------ //
//    PCB VERSION NUMBER:
int verPCB = 2; 

// -------------------------------------------------------------------- //
//    DEBUG MODE?
 
boolean debugMode = false;
boolean fullspeed_noise_test = false;


// -------------------------------------------------------------------- //

// ARDUINO CONNECTIONS:
//
// DIGITAL I/O
// D2	External Stop
// D4	Indicator LED
// D6	DIRECTION +
// D7	ENABLE
// D9	DIRECTION -
// D12  goes HIGH if Motor is on!
// D13	on-board LED
//
// ANALOG I/O
// A0	SENS_VAL
// A1	SPEED_VAL


// ARDUINO DEFINITIONS:
int extstop = 2;
int indled = 4;
int dirpos = 6;
int dirneg = 9;
int enable = 7;
int motoron = 12;
int obled = 13;
int senspin = A0;
int speedpin = A1;


volatile int extstate = LOW;
float sensval = 0.0;
int minspeed = 30;
int maxspeed = 0;
int speed = 0;
int step = 5;
int direction = 0;
unsigned long task1 = 0;
unsigned long task2 = 0;
unsigned long  turning_start = 0;
volatile boolean turning = false;
boolean fail = false;
int ledstate1 = HIGH;

// BEGIN ACTUAL CODE:


void setup()
{
	pinMode(extstop, INPUT_PULLUP);
	pinMode(indled, OUTPUT);
	pinMode(dirpos, OUTPUT);
	pinMode(dirneg, OUTPUT);
	pinMode(enable, OUTPUT);
	pinMode(obled, OUTPUT);
	pinMode(motoron, OUTPUT);

	if (debugMode) {
    	Serial.begin(9600);
  	}
	extstate = digitalRead(extstop);
	if(extstate == LOW){
		attachInterrupt(0,start,RISING);
	}
	if(extstate == HIGH){
		attachInterrupt(0,stop,LOW);
	}
	digitalWrite(indled, extstate);
}


void loop()
{
	if(fullspeed_noise_test){
		turn(-255);
		delay(1000);
		turn(0);
		delay(2000);
		turn(255);
		delay(1000);
		turn(0);
		delay(2000);
	}
	
	
	if(!fullspeed_noise_test){
		if(!turning){turning_start = millis();}
		if( ((millis()-turning_start)>5000)&&extstate==HIGH ){
			fail=true;
		}
		sensval = get_sens();
		maxspeed = get_speed();
		
		if( ((millis()-task1)>20)&&(!fail) ){
			if(sensval<3.0){
				speed += step;
				direction = 1;
			}
			if(sensval>97.0){
				speed += step;
				direction = -1;
			}
			if( (sensval>5.0) && (sensval<95.0) ){
				speed = minspeed;
				direction = 0;
			}
			speed = constrain(speed,minspeed,maxspeed);
			turn(speed*direction);
			task1 = millis();
		}
		if(fail){
			turn(0);
			if((millis()-task2)>150){
				ledstate1 = !ledstate1;
				digitalWrite(indled, ledstate1);
				task2 = millis();
			}
		}
	}
}


void start(){
	extstate = HIGH;
	attachInterrupt(0,stop,LOW);
	digitalWrite(indled, extstate);
}
void stop(){
	extstate = LOW;
	attachInterrupt(0,start,RISING);
	digitalWrite(indled, extstate);
	digitalWrite(enable, LOW);
	digitalWrite(dirpos, LOW);
	digitalWrite(dirneg, LOW);
	fail=false;
	
}

void turn(int speed){
	int val = 0;
	if(speed<0){
		val = abs(speed);
		digitalWrite(enable, extstate);
		analogWrite(dirpos, 0);
		analogWrite(dirneg, val);
		digitalWrite(obled,HIGH);
		digitalWrite(motoron,HIGH);
		turning = true;
	}
	if(speed==0){
		digitalWrite(enable, LOW);
		analogWrite(dirpos, 0);
		analogWrite(dirneg, 0);
		digitalWrite(obled, LOW);
		digitalWrite(motoron,LOW);
		turning = false;
	}
	if(speed>0){
		val = abs(speed);
		digitalWrite(enable, extstate);
		analogWrite(dirpos, val);
		analogWrite(dirneg, 0);
		digitalWrite(obled,HIGH);
		digitalWrite(motoron,HIGH);
		turning = true;
	}	
}


float get_sens(){
	float val = 0;
	val = analogRead(senspin);
	val = val/10.23;
	return val;
}
int get_speed(){
	int val = 0;
	val = analogRead(speedpin);
	val = map(val,0,1023,0,255);
	return val;
}

