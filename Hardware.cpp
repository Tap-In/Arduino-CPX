#include "Arduino.h"
#include "Hardware.h";

////////////////////// ARDUINO HARDWARE COMMANDS ///////////////////////////////

void setPinMode(int pin, int value) {
  pinMode(pin,value);
}

void setDigitalValue(int pin, int value) {
   digitalWrite(pin,value);   
}

int getDigitalValue(int pin) {
   return digitalRead(pin);
}


void setAnalogValue(int pin, int value) {
   analogWrite(pin,value);   
}

int getAnalogValue(int pin) {
   return analogRead(pin);
}

    
