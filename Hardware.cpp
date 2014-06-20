#include "Arduino.h"
#include "Config.h"
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

void fade(int count) {
  int redVal = 255;
  int blueVal = 0;
  int greenVal = 0;
  
for (int k=0;k<count;k++) {
  for( int i = 0 ; i < 255 ; i += 1 ){
    greenVal += 1;
    redVal -= 1;
    analogWrite( RGB_GREEN, 255 - greenVal );
    analogWrite( RGB_RED, 255 - redVal );

    delay( delayTime );
  }
 
  redVal = 0;
  blueVal = 0;
  greenVal = 255;
  for( int i = 0 ; i < 255 ; i += 1 ){
    blueVal += 1;
    greenVal -= 1;
    analogWrite( RGB_BLUE, 255 - blueVal );
    analogWrite( RGB_GREEN, 255 - greenVal );

    delay( delayTime );
  }
 
  redVal = 0;
  blueVal = 255;
  greenVal = 0;
  for( int i = 0 ; i < 255 ; i += 1 ){
    redVal += 1;
    blueVal -= 1;
    analogWrite( RGB_RED, 255 - redVal );
    analogWrite( RGB_BLUE, 255 - blueVal );

    delay( delayTime );
  }
  
}
}
    
