#ifndef Setup_h
#define Setup_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include "ESP8266mDNS.h"
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <FS.h>
#include <ESP8266FtpServer.h>
#include <TimeLib.h>

#include <DNSServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>

#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

/* We are using NodeMCU board! */
#define NODEMCUBOARD

/* Common private libraries */
#include <ntp.h>
#include <leds.h>
#include <wifi.h>
#include <wifiAPinfo.h>
#include <spiffs.h>

/* globals.h contains defines and global variables */
#include "globals.h"
/* functions.h contains all function declarations */
#include "functions.h"

#endif
