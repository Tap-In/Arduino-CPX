#define NAMELEN 16

#define BYTE    0
#define INT     1
#define DOUBLE  2
typedef struct
{
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

typedef struct  {
    char name[NAMELEN];
    char* memory;
    int length;
    int type;
}
symbolTYPE;

void auth(char* returns, JsonHashTable json, char* text);
void ping(char* returns, JsonHashTable json, char* text);
void trigger(char* returns, JsonHashTable json, char* text);
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
void allocate(char* returns, JsonHashTable json, char* text);
void getimage(char* returns, JsonHashTable json, char* text);
void setimage(char* returns, JsonHashTable json, char* text);
void seteeprom(char* returns, JsonHashTable json, char* text);
void geteeprom(char* returns, JsonHashTable json, char* text);

void sendCPmessage(char* user, char* plan, char* args, char* endpoint, char* returns, int wait);
int getJumpCount(char* label);
int findFunction(char* name);
int findIndex(char* label,JsonArray commands);
int findSymbol(char* name);
char* encode(char* data);
char* readBlock();
void transmit(char* buf);
void blink(int PIN, int delay, int times);

int getSize(int type);

long symbolRef(JsonHashTable json, char* label);
long getValueAt(char* name, int index);
void shift(JsonHashTable json, long value);
