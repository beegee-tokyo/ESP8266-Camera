#ifndef Setup_h
#define Setup_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <Ticker.h>
#include <pgmspace.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <TimeLib.h>

#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>

/* We are using NodeMCU board! */
#define NODEMCUBOARD

/* Common private libraries */
#include <ntpLib.h>
#include <ledLib.h>
#include <wifiLib.h>
#include <spiffsLib.h>

/* globals.h contains defines and global variables */
#include "globals.h"
/* functions.h contains all function declarations */
#include "functions.h"

#endif
