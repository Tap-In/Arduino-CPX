#define INTERFACE_TYPE PROXY
#define DEVICEID       "zulu"

// If proxy these will be ignored
#define IPADDRESS            192.168.0.1
#define MACADDRESS           de.ad.be.ef
#define CONTROL_PLAN_ADDR    50,16,114,126
#define CONTROL_PLAN_PORT    8080
#define AUTHKEY              "555-1212"
#define SSID                 "SuperiorData"
#define WPA2                 "jiujitsu"

//////////////////////////////////////////////


#define SS_PIN 53
#define RST_PIN 5

#define WIFI   0
#define PROXY  1

#define temp      PSTR("{\"map\":{\"value\":\"%s\"},\"globals\":[]}")
#define iprint    PSTR("{\"iprint\":\"%s\"}")
#define tempn     PSTR("{\"map\":{\"value\":\"%d\"},\"globals\":[]}")
#define err       PSTR("{\"map\":{\"error\":true,\"value\":\"%s\"},\"globals\":[]}")

