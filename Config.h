// If proxy these will be ignored
//#define CONTROL_PLAN_ADDR    "50.16.114.126"
#define CONTROL_PLAN_ADDR    "192.168.1.15"
#define CONTROL_PLAN_PORT    6666
#define WLAN_SSID            "SuperiorCourtData"           // cannot be longer than 32 characters!
#define WLAN_PASS            "jiujitsu"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY        WLAN_SEC_WPA2
#define IDLE_TIMEOUT_MS      3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.
// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

// DIGITAL
#define GREEN  4
#define RED    6

// RGB
#define RGB_GREEN 46
#define RGB_BLUE 45
#define RGB_RED 44
#define delayTime 20
//////////////////////////////////////////////////////////////////

#define temp      PSTR("{\"map\":{\"value\":\"%s\"},\"globals\":[]}")
#define iprint    PSTR("{\"iprint\":\"%s\"}")
#define tempn     PSTR("{\"map\":{\"value\":\"%d\"},\"globals\":[]}")
#define err       PSTR("{\"map\":{\"error\":true,\"value\":\"%s\"},\"globals\":[]}")

/////////////////////////

#define WIFI   1
#define PROXY  0


#define INTERFACE_TYPE WIFI
#define USER    "\"BEN\""
#define ID      "\"5551212\""
