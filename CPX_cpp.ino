
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

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <SPI.h>
#include <EEPROM.h>

#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include "Lang.h"
#include "Config.h"
#include "Hardware.h"

////////////////////////////////////////////////////////////////
int INTERFACE_TYPE  =       WIFI;

// If proxy these will be ignored
#define CONTROL_PLAN_ADDR    "50.16.114.126"
//#define CONTROL_PLAN_ADDR   "192.168.1.15"
//#define CONTROL_PLAN_ADDR   "192.168.1.3"
#define CONTROL_PLAN_PORT    6666
////////////////////////////////////////////////////////////////

#define WLAN_SECURITY        WLAN_SEC_WPA2
char USER[32] =               {"BEN"};
char ID[32]   =               {"5551212"};
char WLAN_SSID[32] =          {"SuperiorCourtData"};
char WLAN_PASS[32]  =         {"jiujitsu"};
////////////////////////////////////////////////////////////////

JsonParser<128> parser;
JsonHashTable hashTable;
int interface = INTERFACE_TYPE;

Adafruit_CC3000_Client client;
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

int nJumps = 0;
int nFuncs = 0;
int nSym = 0;
jumpTYPE jumps[8];
langTYPE lang[] = {
  { "ping", &ping },
  {"auth", &auth }, 
  {"trigger", &trigger }, 
  {"digitalwrite",&digitalwrite},
  {"digitalread",&digitalread},
  {"analogread",&analogread},
  {"analogwrite",&analogwrite},
  {"delay", &delayx},
  {"notify",&notify},
  {"goto",&gotox},
  {"print", &printx},
  {"call",&call},
  {"allocate", &allocate},
  {"getimage",&getimage},
  {"setimage", &setimage},
  {"seteeprom",&seteeprom},
  {"geteeprom",&geteeprom},
  {"send-callback", &callback}
}; 
callTYPE functions[8];         // a list of 8 possible functions
symbolTYPE symbols[8];

/*
 * Internal functions of the CPX
 */
void doWiFi();
bool displayConnectionDetails(void);
int getRfid(char *data);
void doCommand(char* returns, JsonHashTable json, char* label);

int testme(char*, char*);           // a sample internal function
int lamp(char*, char*);
void construct(char*,char*[]);
uint32_t getIp(char *address) ;
long lastTime;

long R;

void setup() {
  
  pinMode(RED,OUTPUT);
  pinMode(GREEN,OUTPUT);
  SPI.begin();
  Serial.begin(19200);
  
  
  for (int i=0;i<256;i++)
    EEPROM.write(i,0);
    
  readConfigFromProm();
  
  
  digitalWrite(RED,0);
  digitalWrite(GREEN,1);
  delay(100);
  
  char returns[512];
  
  digitalWrite(GREEN,0);
  fade(returns,"1");
  lastTime = millis();
  strcpy(functions[0].name,"testme");
  functions[0].functionPtr = testme;
  strcpy(functions[1].name,"lamp");
  functions[1].functionPtr = lamp;
  strcpy(functions[2].name,"fade");
  functions[2].functionPtr = fade;
  nFuncs = 3;
 

  // RGB SETUP
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  pinMode(RGB_RED, OUTPUT);
  digitalWrite(RGB_GREEN, HIGH);
  digitalWrite(RGB_BLUE, HIGH);
  digitalWrite(RGB_RED, HIGH);
  ///////////////////////////////////////////////////
  
  
/*
 * Comment out if you don't want the light show on Peter's fun box
 */
  int mode = 1;
  for (int i=0;i<6;i++) {
    digitalWrite(RED,!mode);
    digitalWrite(GREEN,!mode);
    delay(250);
    if (mode == 1)
      mode = 0;
    else
      mode = 1;
      
  }
  digitalWrite(RED,1);
  digitalWrite(GREEN,0);
  
  doWiFi();
  digitalWrite(RED,0);
  digitalWrite(GREEN,1);
  delay(200);
  digitalWrite(RGB_RED,1);
  digitalWrite(RGB_GREEN,1);
  digitalWrite(RGB_BLUE,1);
  
  digitalWrite(GREEN,1);
/************************************************************/
 if (interface == WIFI) {
     Serial.println("CPX .1 started");
     char* values[] = {"\"user\"","\"",USER,"\"","\"id\"","\"",ID,"\""};
     char buf[128];
     sprintf(buf,"{\"map\":{\"user\":\"%s\",\"id\":\"%s\"}}",USER,ID);
     //construct(returns,values);
     char *send = encode(buf);
     Serial.println(send);
     transmit(send);
     free(send);
  } else {
    sprintf_P(returns,iprint,"CPX .1 started");
    char *send = encode(returns);
    Serial.write(send);
    free(send);
  }
  sprintf(returns,"!Boot/%s/%s",USER,ID);
  char *send = encode(returns);
  Serial.write(send);
  free(send);
}
void construct(char* returns,char *vals[]) {
  strcpy(returns,"{\"map\":{");
  int i = 0;
  char **p = vals;
  while(*p != NULL) {
    strcat(returns,*p++);
    strcat(returns,":");
    strcat(returns,*p++);
    if (*p != NULL)
      strcat(returns,",");
  }
  strcat(returns,"}}");
}

void doWiFi() {
  if (interface != WIFI)
    return;
  if (!cc3000.begin())
  {
    Serial.println(F("CC3000 Couldn't begin(, hardware error"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
   // Try looking up the website's IP address
  
  uint32_t ip = getIp(CONTROL_PLAN_ADDR);
  
  Serial.print(F("Connecting to ")); Serial.print(CONTROL_PLAN_ADDR); Serial.print("... ");
  while(true) {
    client = cc3000.connectTCP(ip, CONTROL_PLAN_PORT);
    if (client.connected()) {
      Serial.println(F("Wifi Connected"));
      blink(GREEN,250,3);
      return;
    }
    delay(5000);
  }
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
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
  if (json == NULL) {
    Serial.println("Host disconnected, need to reboot");
    blink(RED,250,0);
    return;
  }
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
              char* send = encode("{\"map\":{\"value\":\"ok\"},\"globals\":[]}"); 
              transmit(send);
              free(json);;
              free(send);
              while(client.available()) client.read();
              return;
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
  
  if (strlen(returns) == 0) {                   // No need to answer
      free(json);
      return;
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
  langTYPE* l = lang;
  while(l != 0) {
    if (strcmp(l->name,command)==0) {
      l->functionPtr(returns,json,text);
      return;
    }
    l++;
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

int lamp(char* returns, char* param) {
  int offset;
  int n;
  int k = 0;
  char* data = param;
  while (1 == sscanf(data, " %d%n", &n, &offset)) {
        data += offset;
        
        if (k == 0) {
          k = n;
        } else {
          Serial.print("K "); Serial.println(k);
          Serial.print("X "); Serial.println(5000/k);
          double x = (double)n / 100;
          if (x < 0) {
            n = -1 *n;
           digitalWrite(7,0);
           for (int z= 0;z<5000/k; z+= n) {
              digitalWrite(9,1);
              delay(n);
              digitalWrite(9,0);
              delay(n);
           }
            
          } else {
           digitalWrite(9,0);
           for (int z= 0;z<5000/k; z+= n) {
              digitalWrite(7,1);
              delay(n);
              digitalWrite(7,0);
              delay(n);
           }
        }
        }
  }

  strcpy(returns,"lamp changed");
  return 1;
}

void setEEPROM(int address, int value) {
   EEPROM.write(address, value);
}

int getEEPROM(int address) {
   return EEPROM.read(address);
}


void sendPing() {
  return;
  if (lastTime + 15000 < millis()) {
    if (interface != PROXY) {
      if(client.connected()) {
       client.write("!",1); 
      }
    }
    lastTime = millis();
  }
}

uint32_t getIp(char *address) {
  uint32_t ip = 0;
  int k = 0;
  
  int a, b, c, d;
  k = sscanf(address,"%u.%u.%u.%u",&a,&b,&c,&d);
  if (k == 4) {
    ip = a;
    ip = ip << 8 | b;
    ip = ip << 8 | c;
    ip = ip << 8 | d;
    
    Serial.print("A = " ); Serial.println(a);
    Serial.print("B = " ); Serial.println(b);
    Serial.print("C = " ); Serial.println(c);
    Serial.print("D = " ); Serial.println(d);

    
    Serial.print("Remote IP address: "); Serial.print(ip); Serial.println();
    return ip;
  } else {
    while (ip == 0) {
      if (! cc3000.getHostByName(address, &ip)) {
        Serial.println(F("Couldn't resolve, using dotted decimal"));
      }
      delay(500);
      if (k++ > 20)
        break;
    }
  }
  if (k < 20) {
     Serial.print("Remote IP address: "); Serial.print(ip); Serial.println();
    return ip;
  }
  Serial.println("IP address resolution failed, Halt!");
  while(true);
}





