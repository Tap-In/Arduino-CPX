
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
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>


#include <MFRC522.h>
#include <SPI.h>

#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>

#define WIFI   0
#define PROXY  1

#define INTERFACE_TYPE PROXY

// If proxy these will be ignored
#define IPADDRESS            192.168.0.1
#define MACADDRESS           de.ad.be.ef
#define CONTROL_PLAN_ADDR    50,16,114,126
#define CONTROL_PLAN_PORT    8080
#define AUTHKEY              "555-1212"
#define SSID                 "SuperiorData"
#define WPA2                 "jiujitsu"

////////////////////////////////////////////////////////

/* Function prototype for user created methods */
typedef struct
{
    char name[16];
    int (*functionPtr)(char*,char*);
} callTYPE;
int nFuncs = 0;
char* testme(char*);           // a sample internal function
callTYPE functions[8];         // a list of 8 possible functions

int interface = INTERFACE_TYPE;
IPAddress server(CONTROL_PLAN_ADDR);  

WiFiClient client;

#define SS_PIN 53
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.

JsonParser<128> parser;
JsonHashTable hashTable;

char iprint[512] = {"{\"iprint\":\"%s\"}"};
char temp[512] = {"{\"map\":{\"value\":\"%s\"},\"globals\":[]}"};
char tempn[512] = {"{\"map\":{\"value\":\"%d\"},\"globals\":[]}"};
char err[512] = {"{\"map\":{\"error\":true,\"value\":\"%s\"},\"globals\":[]}"};

/*
 * Internal functions of the CPX
 */
void doWiFi();
void transmit(char* buf);
int findFunction(char* name);
void doCommand(char* returns, JsonHashTable json, char* label);
int findIndex(char* label,JsonArray commands);
char* encode(char* data);
char* readBlock();
void setDigitalValue(int pin, int value);
int getDigitalValue(int pin);
void setAnalogValue(int pin, int value);
int getAnalogValue(int pin);
int getRfid(char *data);

void setup() {
  strcpy(functions[0].name,"testme");
  functions[0].functionPtr = testme;
  nFuncs++;
  
  Serial.begin(19200);
  SPI.begin();
  mfrc522.PCD_Init();	// Init MFRC522 card
  
  if (interface == WIFI) {
     doWiFi(); 
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
  json = readBlock();
  JsonHashTable hashTable = parser.parseHashTable(json);
  hashTable = hashTable.getHashTable("map");
  JsonArray commands = hashTable.getArray("commands");
  if (commands.success()) {
    pc = 0;
    while(pc < commands.getLength()) {
        label[0] = 0;
        if (interface == PROXY) {
          if (Serial.available()) {
              json = readBlock();
              hashTable = hashTable.getHashTable("map");
              doCommand(returns,hashTable,label);
              break;
          }
        } else {
          if (client.available()) {
              json = readBlock();
              hashTable = hashTable.getHashTable("map");
              doCommand(returns,hashTable,label);
              break;
          }
        }
        JsonHashTable cmd = commands.getHashTable(pc);
        doCommand(returns,cmd,label);
        if (strlen(label) == 0)
          pc++;
        else
          pc = findIndex(label,commands);
          if (pc == -1) {
            sprintf(returns,err,"No such label %s",label);
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
 * Call an internal function by name, the 'call' method
 */
int findFunction(char* name) {
  for (int i=0;i<nFuncs;i++) {
     if (strcmp(name,functions[i].name)==0)
      return i; 
  }
  return -1;
}

/*
 * Find the label specified in the goto construct
 */
int findIndex(char* label,JsonArray commands) {
    int pc = 0;
    char* what;
    while(pc < commands.getLength()) {
      JsonHashTable cmd = commands.getHashTable(pc);
      char* cLabel = cmd.getString("label");
      if (cLabel != NULL && strcmp(cLabel,label)==0)
        return pc;
      pc++;
    }
    return -1;
}

/*
 * Execute the command specified
 */
void doCommand(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;
  command = json.getString("command");
  if (command == NULL) {
     sprintf(returns,err,"badly formed command"); 
     return;
  }

  if (strcmp(command,"ping")==0) {
    sprintf(returns,temp,"ok");
  } 
  else
  if (strcmp(command,"readrfid")==0) {
      if (getRfid(command))
        sprintf(returns,temp,command);
      else
        sprintf(returns,err,"RFID reader timed out");
  } 
  else
  if (strcmp(command,"digitalwrite")==0) {
     v1 = json.getLong("pin");
     v2 = json.getLong("value");
    setDigitalValue(v1,v2);
    sprintf(returns,temp,"ok");
  }
  else
  if (strcmp(command,"digitalread")==0) {
     v1 = json.getLong("pin");
     v2 = getDigitalValue(v1);
    sprintf(returns,tempn,v2);
  }
  else
  if (strcmp(command,"analogwrite")==0) {
    v1 = json.getLong("pin");
    v2 = json.getLong("value");
    setAnalogValue(v1,v2);
    sprintf(returns,temp,"ok");
  }
  else
  if (strcmp(command,"analogread")==0) {
    v1 = json.getLong("pin");
    v2 = getAnalogValue(v1);
    sprintf(returns,tempn,v2);
  }
  else
  if (strcmp(command,"delay")==0) {
     v1 = json.getLong("value");
    delay(v1);
    sprintf(returns,temp,"ok");
  }
  else
  if (strcmp(command,"goto")==0) {
    char* label = json.getString("where");
    strcpy(text,label);
    sprintf(returns,temp,"ok");
  }
  else
    if (strcmp(command,"notify")==0) {
    char returns[512];
    char* auth = json.getString("auth");
    char* plan = json.getString("plan");
    char* value = json.getString("value");
    int wait = json.getLong("wait");
    sendCPmessage(auth, plan, value, returns, wait);
    sprintf(returns,temp,returns);
  }
 else
    if (strcmp(command,"print")==0) {
    char* value = json.getString("value");
    sprintf(returns,iprint,value);
  }
  else
  if (strcmp(command,"call")==0) {
    char* fname = json.getString("function");
    char* param = json.getString("param");
    int k = findFunction(fname);
    if (k == -1)
      sprintf(returns,err,fname);
    else {
      char rval[128];
      int rc = functions[k].functionPtr(rval,param);
      sprintf(returns,"{\"map\":{\"value\":\"%s\",\"rc\":%d},\"globals\":[]}",rval,rc);
    }
  }
  else {
     sprintf(returns,err,"unknown command"); 
  }
}

/*
 * Read first 4 bytes, convert to a number, which is how many bytes follow.
 * For example XXX9{"a":"b"}
 */
char* readBlock() {
  char len[4];
  char* buf;
  int sz = 0;
  int value;
  for (int i=0;i<4;i++) {
    if (interface == PROXY) {
      while(!Serial.available());
      value = Serial.read();
    } else {
      while(!client.available());
      value = client.read();
    }
    if (value != 'X')
      len[sz++] = (char)value;
  }
  len[sz] = 0;
  
  sz = atoi(len);
  buf = (char*)malloc(sizeof(char)*sz + 6);
  for (int i = 0; i< sz; i++) {
    if (interface == PROXY) {
      while(!Serial.available());
      buf[i] = (char)Serial.read();
    } else {
      
    }
  }
  buf[sz] = 0;
  return buf;
}

/*
 * Encode a buffer, puts 4 byte length of trailing buffer leading 0 is X and
 * and appends the rest of the buffer. For example {"a":"b"} is
 * encoded to: XXX9{"a":"b"}
 */
 
char* encode(char* data) {
   char len[6];
   char* newData = (char*)malloc((strlen(data)+4) * sizeof(char));
   sprintf(len,"%04d",strlen(data));
   for (int i=0;i<4;i++) {
     if (len[i] == '0')
       len[i] = 'X';
     else
       break;
   }
   len[5] = 0;
   strcpy(newData,len);
   strcat(newData,data);
   return newData;
}

/*
 * A demonstration function for use with the 'call' command.
 */
int testme(char* rets, char* param) {
    strcpy(rets,"You have arrived");
    return 1;
}

/*
 * Send an unsolicited message to control plan
 */
void sendCPmessage(char* auth, char* plan, char* value, char* returns, int wait) {
  char* buf = (char*)malloc(sizeof(char)*512);
  sprintf(buf,"{\“map\”:{\“command\”:\”notify\”,\”auth\”,\”%s\”,\”plan\”:”%s\”,\“value\”: \“%s\”}}",
    auth,plan,value);
  encode(buf);
  transmit(buf);
  char* json = readBlock();
  strcpy(returns,json);
  free(json);
}


/*
 * Send the response to either Serial or to the Wifi shield, depending
 * on how you have it set up.
 */
void transmit(char* buf) {
   if (interface == PROXY) {
     Serial.print(buf);
   } else {
     client.print(buf);
   }
}

////////////////////// ARDUINO HARDWARE COMMANDS ///////////////////////////////

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

int getRfid(char *data) {
  data[0] = 0;
  int startTime = millis();
  while ( ! (mfrc522.PICC_IsNewCardPresent())) {
      if ((millis() - startTime) > 20000) {
           //mfrc522.PICC_HaltA();  
           //return 0;
           break;
      }   
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

    
