#include <ESP8266WiFi.h>


/** Hostname & AP name created from device function & 1. and 4. to 6. part of MAC address */
extern char hostApName[];
/** Debug name created from last part of hostname */
extern String OTA_HOST;
/** IP address of this module */
extern IPAddress ipAddr;
/** ID for monitor to be replaced by function in the future */
#define DEVICE_ID "cm1"

/** Build time */
extern const char compileDate[];
/** WiFiServer class to create TCP socket server on port tcpComPort */
extern WiFiServer tcpServer;
/** FTP client */
extern WiFiClient ftpClient;
/** External FTP server for data transfer*/
extern WiFiClient ftpDataClient;
/** External FTP server IP */
extern IPAddress ftpDataServer;
/** External FTP server port */
extern uint16_t ftpDataPort;

/** Buffer for received/sent data */
extern char ftpBuf[];
/** Counter for sent/received data */
extern char ftpCount;


/** Bug capture trial year of last good NTP time received */
extern int lastKnownYear;

/** Flag for OTA update running */
extern boolean otaRunning;
/** Flag for TCP debugging */
extern bool debugOn;
/** Last time NTP sync was performed */
extern time_t lastSyncTime;
/** Flag if heart beat was triggered */
extern boolean heartBeatTriggered;
/** Timer for heart beat */
extern Ticker heartBeatTimer;
/** Flag for broadcast status & consumption */
extern boolean sendUpdateTriggered;
/** Flag for broadcast status */
extern boolean sendUpdateTriggered;

// On ESP: camera TX connected to GPIO15, camera RX to GPIO13:
// extern SoftwareSerial cameraconnection;
extern SoftwareSerial cameraconnection;
// Camera connection
extern Adafruit_VC0706 cam;
// Flag if camera was found
extern boolean hasCamera;
/** Name of the last saved picture */
extern char filename[];

/** Flashlight LED output */
extern int flashLED;
/** Blinking LED output */
extern int blinkLED;
