#include <ESP8266WiFi.h>

// Function definitions

// Communication functions
void sendBroadCast(boolean shotResult);
void socketServer(WiFiClient tcpClient);

// Camera functions
boolean takeShot();

// FTP client functions
byte ftpReceive();
bool ftpConnect();

// Ticker functions
void triggerHeartBeat();

// SPIFFS functions
bool formatSPIFFS();
