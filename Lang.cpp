#include "Arduino.h"

#include <stdio.h>
#include <JsonArray.h>
#include <JsonParser.h>
#include <JsonObjectBase.h>
#include <JsonHashTable.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <MFRC522.h>
#include "Lang.h"
#include "Hardware.h"
#include "Config.h"

extern JsonHashTable hashTable;
//extern char* temp;;
extern int interface;
extern WiFiClient client;
extern int nJumps;
extern int nLang;
extern int nFuncs;
extern jumpTYPE jumps[];
extern langTYPE lang[];
extern callTYPE functions[];  
extern int getRfid(char *data);

void ping(char* returns, JsonHashTable json, char* text) { 
    sprintf(returns,temp,"ok");
}

void readrfid(char* returns, JsonHashTable json, char* text) { 
  char id[16];
  if (getRfid(id))
      sprintf(returns,temp,id);
   else
      sprintf(returns,err,"RFID reader timed out");
} 

void digitalwrite(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;

  v1 = json.getLong("pin");
  v2 = json.getLong("value");
  setDigitalValue(v1,v2);
  sprintf(returns,temp,"ok");
}

void digitalread(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;
  command = json.getString("command");
  v1 = json.getLong("pin");
  v2 = getDigitalValue(v1);
  sprintf(returns,tempn,v2);
}

void analogwrite(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;
  v1 = json.getLong("pin");
  v2 = json.getLong("value");
  setAnalogValue(v1,v2);
  sprintf(returns,temp,"ok");
}

void analogread(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;
  v1 = json.getLong("pin");
  v2 = getAnalogValue(v1);
  sprintf(returns,tempn,v2);
}

void delayx(char* returns, JsonHashTable json, char* text) { 
  int v1;
  v1 = json.getLong("value");
  delay(v1);
  sprintf(returns,temp,"ok");
}

void notify(char* returns, JsonHashTable json, char* text) { 
  char* command;
  int v1, v2;
  char returnsx[512];
  char* auth = json.getString("auth");
  char* plan = json.getString("plan");
  char* value = json.getString("value");
  int wait = json.getLong("wait");
  sendCPmessage(auth, plan, value, returnsx, wait);
  sprintf(returns,temp,returns);
}

void gotox(char* returns, JsonHashTable json, char* text) { 
  char* command;
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
    sprintf(returns,temp,"ok");
  }
  
void printx(char* returns, JsonHashTable json, char* text) { 
  char* command;
  char* value = json.getString("value");
    sprintf(returns,iprint,value);
}

void call(char* returns, JsonHashTable json, char* text) { 
  char* command;
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
  
void sendCPmessage(char* auth, char* plan, char* value, char* returns, int wait) {
  char* buf = (char*)malloc(sizeof(char)*512);
 // sprintf(buf,"{\"map\":{\"command\":\"notify\",\"auth\",\”%s\",\"plan\":”%s\",\"value\": \"%s\"}}",
 //   auth,plan,value);
  encode(buf);
  transmit(buf);
  char* json = readBlock();
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


