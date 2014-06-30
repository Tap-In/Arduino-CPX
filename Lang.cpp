#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>


#include "Arduino.h"

#include <ccspi.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <stdio.h>
#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>

#include "Lang.h"
#include "Hardware.h"
#include "Config.h"

extern JsonHashTable hashTable;
//extern char* temp;;
extern int interface;
extern Adafruit_CC3000_Client client;
extern int nJumps;
extern int nLang;
extern int nFuncs;
extern int nSym;
extern jumpTYPE jumps[];
extern langTYPE lang[];
extern callTYPE functions[];  
extern symbolTYPE symbols[];
extern int getEEPROM(int address);
extern void setEEPROM(int address, int value);
extern long R;

int pingCount = 0;

void auth(char* returns, JsonHashTable json, char* text) {
  boolean auth = json.getBool("value");
  if (auth == false) {
    //Serial.println("Authorization failed, halted");
    digitalWrite(6,1);
    while(true);
  } else {
    if (interface == WIFI)
      Serial.println("Authorization Ok");
  }
  digitalWrite(4,1);
  returns[0] = 0;
}

void ping(char* returns, JsonHashTable json, char* text) { 
    sprintf(returns,"{\"map\":{\"value\":%u},\"globals\":[]}",millis()/1000);
}

void trigger(char* returns, JsonHashTable json, char* text) { 
  
  int pin = symbolRef(json,"pin");
  long to = symbolRef(json,"timeout");
  pinMode(pin, INPUT);
  delay(100);
  int old  = digitalRead(pin);
  long value = millis();
  while(digitalRead(pin) == old) {
    delay(100);
    if (to > 0 &&( to < millis() - value)) {
        sprintf_P(returns,tempn,-1);
        return;
    }
  }
  value = millis() - value;
  R = value;
  sprintf_P(returns,tempn,value);
}

void digitalwrite(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;

  v1 = symbolRef(json,"pin");
  v2 = symbolRef(json,"value");
  setDigitalValue(v1,v2);
  sprintf_P(returns,temp,"ok");
}

void digitalread(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  R = v2 = getDigitalValue(v1);
  sprintf_P(returns,tempn,v2);
  shift(json, v2);
}

void analogwrite(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  v2 = symbolRef(json,"value");
  setAnalogValue(v1,v2);
  sprintf_P(returns,temp,"ok");
}

void analogread(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"pin");
  R = v2 = getAnalogValue(v1);
  sprintf_P(returns,tempn,v2);
  shift(json, v2);
}

void geteeprom(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"address");
  R = v2 = getEEPROM(v1);
  sprintf_P(returns,temp,"ok");
  shift(json,v2);
}

void seteeprom(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
  v1 = symbolRef(json,"address");
  v2 = symbolRef(json,"value");
  setEEPROM(v1,v2);
  sprintf_P(returns,temp,"ok");
}

void delayx(char* returns, JsonHashTable json, char* text) { 
  int v1;
  v1 = symbolRef(json,"value");
  delay(v1);
  sprintf_P(returns,temp,"ok");
}

void notify(char* returns, JsonHashTable json, char* text) { 
  char returnsx[512];
  char* user = json.getString("plan-user");
  char* plan = json.getString("plan");
  char* value = json.getString("args");
  char* endpoint = json.getString("endpoint");
  long wait = json.getLong("wait");
  
  sendCPmessage(user,plan, value, endpoint, returnsx, wait);
  sprintf_P(returns,temp,returnsx);
}

void gotox(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
    char* label = json.getString("where");
    if (json.containsKey("count")) {
      int count = json.getLong("count");
      int k = getJumpCount(label);
      if (k == -1) {
        k = count - 1;
        strcpy(jumps[nJumps].label,label);
        jumps[nJumps].count = k;
        nJumps++;
      }
      if (k > 1) 
        strcpy(text,label);
      else
        text[0]=0;
    }
    else {
      strcpy(text,label);
      
    }
    sprintf_P(returns,temp,"ok");
  }
  
void printx(char* returns, JsonHashTable json, char* text) { 
  char* value = json.getString("value");
  Serial.println("READY");
  sprintf_P(returns,iprint,value);
  Serial.println("DONE");
}

void call(char* returns, JsonHashTable json, char* text) { 
  int v1, v2;
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
  
void sendCPmessage(char* user, char* plan, char* args, char* endpoint, char* returns, int wait) {
  char* buf = (char*)malloc(sizeof(char)*1024);
  char swait[16];
  sprintf(swait,"%d",wait);
  strcpy(buf,"{\"map\":{\"proxy-command\":\"notify\",\"plan-user\":\"");
  strcat(buf,user);
  strcat(buf,"\",\"plan\":\"");
  strcat(buf,plan);
  strcat(buf,"\",\"args\":");
  strcat(buf,args);
  strcat(buf, ",\"endpoint\":\"");
  strcat(buf,endpoint);
  strcat(buf,"\",\"wait\":");
  strcat(buf,swait);
  strcat(buf,"}}");
  char* send = encode(buf);
  Serial.println(send);
  transmit(send);
  free(send);
  free(buf);
  char *json = readBlock();
  strcpy(returns,json);
  free(json);
}

int getJumpCount(char* label) {
  for (int i=0;i<nJumps;i++) {
    if (strcmp(label,jumps[i].label)==0)
      return jumps[i].count--;
  }
  return -1;
}

void getimage(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int tSize;
  int k = findSymbol(name);
  byte* bvalue;
  int* ivalue;
  double* dvalue;
  char array[1024];
  
  if (k == -1) {
    sprintf(returns,err,"unknown symbol");
    return;
  }
  int start = symbolRef(json,"start");
  int stop  = symbolRef(json,"stop");
  char value[16];
  int *imemory = (int*)&symbols[k].memory[start*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[start*getSize(symbols[k].type)];
  strcpy(array,"[");
  for (int i = 0; i<stop;i++) {
    switch(symbols[k].type) {
      case BYTE:
         sprintf(value,"%u",bmemory[i]);
         break;
      case INT:
         sprintf(value,"%i",imemory[i]);
         break;
     case DOUBLE:
        break;
     }
     strcat(array,value);
     if (i+1 < stop)
       strcat(array,",");
  }
  strcat(array,"]");
  sprintf_P(returns,temp,array);
}

long getValueAt(char* name, int index) {
  int k = findSymbol(name);
  long value;
  int *imemory = (int*)&symbols[k].memory[index*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[index*getSize(symbols[k].type)];
  switch(symbols[k].type) {
      case BYTE:
         value = bmemory[0];
         break;
      case INT:
         value = imemory[0];
         break;
     case DOUBLE:
        break;
     }
  return value;
}

int getSize(int type) {
  switch(type) {
    case INT: return sizeof(int);
    case BYTE: return sizeof(byte);
    case DOUBLE: return sizeof(double);
  }
  return -1;
}

void setimage(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int tSize;
  byte bvalue;
  int ivalue;
  double dvalue;
  int k = findSymbol(name);
  if (k == -1) {
    sprintf(returns,err,"unknown symbol");
    return;
  }
  
  int start = symbolRef(json,"start");
  JsonArray array = json.getArray("values");
  char value[16];
  int stop = array.getLength();
  int x, y;
  int *imemory = (int*)&symbols[k].memory[start*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[start*getSize(symbols[k].type)];
  for (int i = 0; i<stop;i++) {
    switch(symbols[k].type) {
      case BYTE:
         x = (char)array.getLong(i);
         bmemory[i] = x;
         break;
      case INT:
         y = array.getLong(i);
         imemory[i] = y;
         break;
     case DOUBLE:
        break;
     }
  }
  sprintf_P(returns,temp,"ok");
}

void allocate(char* returns, JsonHashTable json, char* text) { 
  char* name = json.getString("name");
  int size = symbolRef(json,"size");
  char* type = json.getString("type");
  int tSize = 1;
  
  int k = findSymbol(name);
  if (k != -1) {
      free(symbols[k].memory);
  } else {
    strcpy(symbols[nSym].name,name);
    if (strcmp(type,"byte")==0) {
      symbols[nSym].type = BYTE;
      tSize = 1;
    } 
    if (strcmp(type,"int")==0) {
      symbols[nSym].type = INT;
      tSize = sizeof(int);
    } 
    if (strcmp(type,"double")==0) {
      symbols[nSym].type = DOUBLE;
      tSize = sizeof(double);
    } 
    k = nSym;
    nSym++;
  }
  symbols[k].memory = (char*)malloc(tSize * size);
  sprintf_P(returns,temp,"ok");
}

/**
  * Find the index of a symbol in the symbol table
  */
int findSymbol(char* name) {
  for (int i=0;i<nSym;i++) {
     if (strcmp(name,symbols[i].name)==0)
      return i; 
  }
  return -1;
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
 * Read first 4 bytes, convert to a number, which is how many bytes follow.
 * For example XXX9{"a":"b"}
 */
char* readBlock() {
  char len[4];
  char* buf;
  int sz = 0;
  char returns[512];
  int value;
  for (int i=0;i<4;i++) {
    if (interface == PROXY) {
      while(!Serial.available());
      value = Serial.read();
    } else {
     int z = 0;
     while(client.connected() && !client.available()) {
         if (z++ > 1000) {
           z = 0;
          Serial.print("PING ");
          client.write("!",1);
          Serial.println(pingCount++);
    }
     }
     if (!client.connected()) {
         Serial.println("Host has disconnected");
         blink(RED,250,0);
     }
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
      while(!client.available());
      buf[i] = (char)client.read();
    }
  }
  buf[sz] = 0;
  return buf;
}

/**
  * BLINK A PIN n times
  */
void blink(int PIN, int delta, int times) {
  int k = 0;
  while(true) {
    digitalWrite(PIN,0);
    delay(delta);
    digitalWrite(PIN,1);
    delay(delta);
    if (times > 0) {
      if (k++ > times) {
          digitalWrite(PIN,0);
          return;
      }
    }
  }
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
 * Send the response to either Serial or to the Wifi shield, depending
 * on how you have it set up. If WiFi send in little chunks so you don't
 * overrun the C3000 transmit buffer.
 */
void transmit(char* buf) {
   if (interface == PROXY) {
     Serial.print(buf);
   } else {
     if (strlen(buf) > 60) {
       char block[70];
       for (int i = 0; i < strlen(buf); i+= 60) {
         strncpy(block, buf+i,60);
         block[60] = 0;
         client.fastrprint(block);
       }
     } else
       client.fastrprint(buf);
   }
}

/**
  * Dereference a symbol at this label, or if a number, simply return the number
  */
long symbolRef(JsonHashTable json, char* label) {
  long returns;
  if (label == "R")
    return R;
    
  int index = json.getLong("index");
  if (json.isNumber(label)) {
    returns = json.getLong(label);
  } else {
     returns = getValueAt(label,index);
  }
  return returns;
}

void shift(JsonHashTable json, long value) {
   char* name = json.getString("shift");
   if (name == NULL)
     return;
  int k = findSymbol(name);
  if (k == -1)
    return;
  int index = json.getLong("index");
  int *imemory = (int*)&symbols[k].memory[index*getSize(symbols[k].type)];
  byte *bmemory = (byte*)&symbols[k].memory[index*getSize(symbols[k].type)];
  switch(symbols[k].type) {
      case BYTE:
         bmemory[0] = (byte)value;
         break;
      case INT:
         imemory[0] = value;
         break;
     case DOUBLE:
        break;
     }
}


