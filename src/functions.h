#ifndef functions_h
#define functions_h

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

//Function definitions

// Communication functions
void sendBroadCast(boolean shotResult);
void socketServer(WiFiClient tcpClient);
void initOTA();

// Camera functions
boolean takeShot();

// FTP client functions
byte ftpReceive();
bool ftpConnect();

// Ticker functions
void triggerHeartBeat();

// SPIFFS functions
bool formatSPIFFS();

#endif