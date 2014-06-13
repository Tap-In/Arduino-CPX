/*
    This program is free software: you can redistribute it and/or modify it under the 
    terms of the GNU General Public License as published by the Free Software Foundation, 
    either version 3 of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#include <MFRC522.h>
#include <SPI.h>
#include <EEPROM.h>

#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include "Lang.h"
#include "Config.h"

////////////////////////////////////////////////////////

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.
WiFiClient client;
JsonParser<128> parser;
JsonHashTable hashTable;
int interface = INTERFACE_TYPE;
IPAddress server(CONTROL_PLAN_ADDR); 

int nJumps = 0;
int nLang = 0;
int nFuncs = 0;
int nSym = 0;
jumpTYPE jumps[8];
langTYPE lang[32];
callTYPE functions[8];         // a list of 8 possible functions
symbolTYPE symbols[8];

/*
 * Internal functions of the CPX
 */
void doWiFi();
int getRfid(char *data);
void doCommand(char* returns, JsonHashTable json, char* label);

void setup() {
  char returns[128];
  strcpy(functions[0].name,"testme");
  functions[0].functionPtr = testme;
  nFuncs++;
  
  strcpy(lang[nLang].name,"ping");
  lang[nLang++].functionPtr = &ping;
  strcpy(lang[nLang].name,"readrfid");
  lang[nLang++].functionPtr = &readrfid;
  strcpy(lang[nLang].name,"digitalwrite");
  lang[nLang++].functionPtr = &digitalwrite;
  strcpy(lang[nLang].name,"digitalread");
  lang[nLang++].functionPtr = &digitalread;
  strcpy(lang[nLang].name,"analogread");
  lang[nLang++].functionPtr = &analogread;
  strcpy(lang[nLang].name,"analogwrite");
  lang[nLang++].functionPtr = &analogwrite;
  strcpy(lang[nLang].name,"delay");
  lang[nLang++].functionPtr = &delayx;
  strcpy(lang[nLang].name,"notify");
  lang[nLang++].functionPtr = &notify;
  strcpy(lang[nLang].name,"goto");
  lang[nLang++].functionPtr = &gotox;
  strcpy(lang[nLang].name,"print");
  lang[nLang++].functionPtr = &printx;
  strcpy(lang[nLang].name,"call");
  lang[nLang++].functionPtr = &call;
  strcpy(lang[nLang].name,"allocate");
  lang[nLang++].functionPtr = &allocate;
  strcpy(lang[nLang].name,"getimage");
  lang[nLang++].functionPtr = &getimage;
  strcpy(lang[nLang].name,"setimage");
  lang[nLang++].functionPtr = &setimage;
  strcpy(lang[nLang].name,"seteeprom");
  lang[nLang++].functionPtr = &seteeprom;
  strcpy(lang[nLang].name,"geteeprom");
  lang[nLang++].functionPtr = &geteeprom;
  
  Serial.begin(19200);
  SPI.begin();
  mfrc522.PCD_Init();	// Init MFRC522 card
  
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  
  
/*
 * Comment out if you don't want the light show on Peter's fun box
 */
  int mode = 1;
  for (int i=0;i<6;i++) {
    digitalWrite(7,!mode);
    digitalWrite(8,!mode);
    digitalWrite(9,!mode);
    digitalWrite(10,mode);
    delay(250);
    if (mode == 1)
      mode = 0;
    else
      mode = 1;
      
  }
  digitalWrite(7,0);
  digitalWrite(8,0);
  digitalWrite(9,0);
  digitalWrite(10,0);
/************************************************************/
 if (interface == WIFI) {
     doWiFi(); 
     Serial.println("CPX .1 started");
  } else {
    sprintf_P(returns,iprint,"CPX .1 started");
    char *send = encode(returns);
    Serial.write(send);
    free(send);
  }
}

void doWiFi() {
   int status;
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if ( fv != "1.1.0" )
    Serial.println("Please upgrade the firmware");

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SSID, WPA2);

    // wait 10 seconds for connection:
    delay(10000);
  }
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  status = 0;
  while(status == 0) {
   if (client.connect(server, CONTROL_PLAN_PORT)) {
      Serial.println("connected");
      status = 1;
    } else {
      Serial.println("Waiting...");
     delay(10000); 
    }
  }
}

/*
 * Read commands from Control Plan
 */
void loop() {
  char *json;
  char command[32];
  char returns[512];
  char label[32];
  int v1, v2, pc;
  nJumps = 0;
  json = readBlock();
  JsonHashTable hashTable = parser.parseHashTable(json);
  hashTable = hashTable.getHashTable("map");
  JsonArray commands = hashTable.getArray("commands");
  if (commands.success()) {
    pc = 0;
    sprintf_P(returns,temp,"ok");
    char* send = encode(returns);
    transmit(send);                 // ack command block
    free(send);
   
    while(pc < commands.getLength()) {
        label[0] = 0;
        int inter = 0;
        if (interface == PROXY) {
          if (Serial.available()) { 
              inter = 1;
              char* send = encode("{\"map\":{\"value\":\"ok\"},\"globals\":[]}");      
              transmit(send);
              free(json);;
              free(send);
              while(Serial.available()) Serial.read();
              return;
          }
        } else {
          if (client.available()) {
              inter = 1;
          }
        }
        
        if (inter == 1) {
            char* send = encode("{\"map\":{\"value\":\"ok\"},\"globals\":[]}");      
            transmit(send);
            free(json);;
            free(send);
            while(Serial.available()) Serial.read();
            return;
        }
        
        JsonHashTable cmd = commands.getHashTable(pc);
        doCommand(returns,cmd,label);
        if (strlen(label) == 0)
          pc++;
        else
          pc = findIndex(label,commands);
          if (pc == -1) {
            sprintf_P(returns,err,"No such label %s",label);
            break;
          }
      }
  } else {
      doCommand(returns,hashTable,label);
  }
    
  char* send = encode(returns);
  transmit(send);
  free(json);
  free(send);
  
}

/*

/*
 * Execute the command specified
 */
void doCommand(char* returns, JsonHashTable json, char* text) { 
  char* command;
  command = json.getString("command");
  if (command == NULL) {
     sprintf_P(returns,err,"badly formed command"); 
     return;
  }
 
  for (int i=0;i<nLang;i++) {
    if (strcmp(lang[i].name,command)==0) {
      lang[i].functionPtr(returns,json,text);
      return;
    }
  }
  sprintf_P(returns,err,"unknown command"); 
}


/*
 * A demonstration function for use with the 'call' command.
 */
int testme(char* rets, char* param) {
    strcpy(rets,"You have arrived");
    return 1;
}


int getRfid(char *data) {
  data[0] = 0;
  int startTime = millis();
  while ( ! (mfrc522.PICC_IsNewCardPresent())) {
/*      if ((millis() - startTime) > 20000) {
           //mfrc522.PICC_HaltA();  
           //return 0;
           break;
      }    */
  }
  
  if (!mfrc522.PICC_ReadCardSerial())
    return 0;
    
  String s = "";
  for (int i=0;i<mfrc522.uid.size;i++) {
     s += String(mfrc522.uid.uidByte[i],HEX);
     if (i+1 != mfrc522.uid.size)
        s += ".";
     }
   s.toCharArray(data,12);
   mfrc522.PICC_HaltA();  
   return 1;
}

void setEEPROM(int address, int value) {
   EEPROM.write(address, value);
}

int getEEPROM(int address) {
   return EEPROM.read(address);
}



