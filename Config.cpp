#include <SPI.h>
#include <EEPROM.h>
#include "Config.h"

void eepromString(int address);
void eepromPrint(int address);
void eitem(char* label, int address, int ro);
void eepromCopy(int address, char* base);

#define USER_ADDR  0
#define ID_ADDR  32
#define ISD_SSID 64
#define INTERFACE_ADDR 96
#define SSID_ADDR 128
#define PASS_ADDR  160
#define WIFI_ADDR  192

extern char USER[];
extern char ID[];
extern char WLAN_SSID[];
extern char WLAN_PASS[];
extern int INTERFACE_TYPE;

void eitem(char* label, int address, int ro) {
  Serial.print(label);
  if (!ro) {
    eepromString(address);
  }
  eepromPrint(address);
}

void eepromString(int address) {
   byte n;
   int k = 0;
   while(true) {
    if (Serial.available()) {
      n=Serial.read();
      if (n == '!')
        break;
      else
        k++;
      EEPROM.write(address++,n);
    }
  }
  if (k > 0)
    EEPROM.write(address,0);
}

void writeString(int address, char *s) {
   int n = 0;
   for (int i=address; i < address+16;i++) {
      EEPROM.write(i,s[n]);
      if (s[n] == 0) 
        return;
    }
  }

void eepromPrint(int address) {
  byte n;
  while(true) {
    n = EEPROM.read(address++);
    if (n != 0) {
      Serial.write(n);
    }
    else
      break;
  }
  Serial.println();
}

void eepromCopy(int address, char* base) {
  int i = 0;
  while(true) {
    byte b = EEPROM.read(address+i);
    if (b == 0)
      break;
    base[i] = b;
    i++;
  }
  base[i] = 0;
}  

int changeProm() {
  int n = 10;
  int i = 0;
  for (i=0;i<10;i++) {
    digitalWrite(RED,1);
    digitalWrite(GREEN,0);
    
    if (Serial.available()) {
      Serial.read();
      break;
    }
    
    Serial.print("\rChange prom: "); Serial.print(n); Serial.print("   ");
    n--;
    delay(500);
    digitalWrite(RED,0);
    digitalWrite(GREEN,1);
    delay(500);
    Serial.println();
  }
  digitalWrite(GREEN,0);
    digitalWrite(RED,0);
  if (i < 10)
    return 1;
  else
    return 0;
}

void readConfigFromProm() {
  int change = 1;
 
   Serial.println("\n---------------");
   if (EEPROM.read(0) != 0)  {
      eitem("User Name:",USER_ADDR,1);
      eitem("ID:",ID_ADDR,1);
      eitem("WIFI (y/n):",WIFI_ADDR,1);
      if (EEPROM.read(WIFI_ADDR) == 'y')
       INTERFACE_TYPE = WIFI;
      else
       INTERFACE_TYPE = PROXY;
       eitem("SSID:",SSID_ADDR,1);
      eitem("PASS:",PASS_ADDR,1);
   
      Serial.println("---------------");
  } else {
    Serial.print("User Name:"); Serial.println(USER);
    Serial.print("ID:"); Serial.println(ID);
    Serial.print("WIFI:"); if (INTERFACE_TYPE == 1)  { 
      Serial.println("y");
      Serial.print("SSID:"); Serial.println(WLAN_SSID);
      Serial.print("PASS:"); Serial.println(WLAN_PASS);
    }
    else
      Serial.println("n");
}
  
  if (changeProm()) {
    eitem("User Name:",USER_ADDR,0);
    eitem("ID:",ID_ADDR,0);
    eitem("WIFI (y/n):",WIFI_ADDR,0); 
    if (EEPROM.read(WIFI_ADDR)=='y') {
      eitem("SSID:",SSID_ADDR,0);
      eitem("PASS:",PASS_ADDR,0);
    }
  
    eepromCopy(USER_ADDR,USER);
    eepromCopy(ID_ADDR,ID);
    eepromCopy(SSID_ADDR,WLAN_SSID);
    eepromCopy(PASS_ADDR,WLAN_PASS);
    if (EEPROM.read(WIFI_ADDR) == 'y')
     INTERFACE_TYPE = WIFI;
   else
     INTERFACE_TYPE = PROXY;  
  }
}

