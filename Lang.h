#define NAMELEN 16

typedef struct
{
    int code;               // is -1 if not a# reserved word
    char name[16];
    void (*functionPtr)(char* returns, JsonHashTable json, char* text);
} langTYPE;

typedef struct {
    char label[NAMELEN];
    int count;
} jumpTYPE;


typedef struct
{
    char name[NAMELEN];
    int (*functionPtr)(char*,char*);
} callTYPE;

char* testme(char*);           // a sample internal function

void ping(char* returns, JsonHashTable json, char* text);

void ping(char* returns, JsonHashTable json, char* text);
void readrfid(char* returns, JsonHashTable json, char* text);
void digitalwrite(char* returns, JsonHashTable json, char* text);
void digitalread(char* returns, JsonHashTable json, char* text);
void analogread(char* returns, JsonHashTable json, char* text);
void analogwrite(char* returns, JsonHashTable json, char* text);
void delayx(char* returns, JsonHashTable json, char* text);
void notify(char* returns, JsonHashTable json, char* text);
void gotox(char* returns, JsonHashTable json, char* text);
void printx(char* returns, JsonHashTable json, char* text);
void call(char* returns, JsonHashTable json, char* text);

void sendCPmessage(char* auth, char* plan, char* value, char* returns, int wait);
int getJumpCount(char* label);
int findFunction(char* name);
int findIndex(char* label,JsonArray commands);
char* encode(char* data);
char* readBlock();
void transmit(char* buf);
