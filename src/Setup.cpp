#include <Setup.h>
#include <declarations.h>

/** Timer for heart beat */
Ticker heartBeatTimer;

void setup() {
	inSetup = true;
  initLeds(blinkLED,flashLED); // COM LED -- ACT LED
	digitalWrite(flashLED,LOW);
	digitalWrite(blinkLED,LOW);

	Serial.begin(115200);

	Serial.setDebugOutput(false);
	Serial.println("");
	Serial.println("Hello from ESP8266 home security camera");
	Serial.print("SW build: ");
	Serial.println(compileDate);

	// Initialize file system.
	bool spiffsOK = false;
	if (!SPIFFS.begin())
	{
		if (SPIFFS.format()){
			spiffsOK = true;
		}
	} else { // SPIFFS ready to use
		spiffsOK = true;
	}
	if (spiffsOK) {
		char tmpLoc[40];
		if (getConfigEntry("loc", tmpLoc)) {
			devLoc = String(tmpLoc);
		}
		if (getConfigEntry("light", tmpLoc)) {
			lightID = String(tmpLoc);
		}
		if (getConfigEntry("cam", tmpLoc)) {
			camID = String(tmpLoc);
		}
		if (getConfigEntry("sec", tmpLoc)) {
			secID = String(tmpLoc);
		}
	}

	// Create device ID from MAC address
	String macAddress = WiFi.macAddress();
	hostApName[8] = OTA_HOST[4] = macAddress[0];
	hostApName[9] = OTA_HOST[5] = macAddress[1];
	hostApName[10] = OTA_HOST[6] = macAddress[9];
	hostApName[11] = OTA_HOST[7] = macAddress[10];
	hostApName[12] = OTA_HOST[8] = macAddress[12];
	hostApName[13] = OTA_HOST[9] = macAddress[13];
	hostApName[14] = OTA_HOST[10] = macAddress[15];
	hostApName[15] = OTA_HOST[11] = macAddress[16];

	Serial.println(hostApName);

	// resetWiFiCredentials();
	// Add parameter for the wifiManager
	WiFiManagerParameter wmDevLoc("loc","House",(char *)&devLoc[0],40);
	wifiManager.addParameter(&wmDevLoc);
	WiFiManagerParameter wmLightID("light","Light",(char *)&lightID[0],40);
	wifiManager.addParameter(&wmLightID);
	WiFiManagerParameter wmCamID("cam","Camera",(char *)&camID[0],40);
	wifiManager.addParameter(&wmCamID);
	WiFiManagerParameter wmSecID("sec","Security",(char *)&secID[0],40);
	wifiManager.addParameter(&wmSecID);
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	// Try to connect to WiFi with captive portal
	ipAddr = connectWiFi(hostApName);
	// ipAddr = connectWiFi(ipAddr, ipGateWay, ipSubNet, hostApName);

	sendRpiDebug("Reboot", OTA_HOST);

	// Check if the configuration data has changed
	if (shouldSaveConfig) {
		if (wmDevLoc.getValueLength() != 0) {
			devLoc = String(wmDevLoc.getValue());
			saveConfigEntry("loc", devLoc);
		}
		if (wmLightID.getValueLength() != 0) {
			lightID = String(wmLightID.getValue());
			saveConfigEntry("light", lightID);
		}
		if (wmCamID.getValueLength() != 0) {
			camID = String(wmCamID.getValue());
			saveConfigEntry("cam", camID);
		}
		if (wmSecID.getValueLength() != 0) {
			secID = String(wmSecID.getValue());
			saveConfigEntry("sec", secID);
		}
	}

	// Start the tcp socket server to listen on port tcpComPort
	tcpServer.begin();

	// Initialize file system.
	boolean foundStatus = SPIFFS.begin();
	if (foundStatus) { // File system found
		// Try to get last saved image
		File imgFile = SPIFFS.open("/last.jpg", "r");
		if (imgFile != 0) {
			String debugMsg = "Saved image found: " + String(imgFile.size()) + "bytes";
			sendRpiDebug(debugMsg, OTA_HOST);
			imgFile.close();
		} else {
			foundStatus = false;
		}
	} else {
		sendRpiDebug("Filesystem failure", OTA_HOST);
	}
	if (!foundStatus) // Could not get last status or file system not ready
	{
		sendRpiDebug("SPIFFS failure, try to format", OTA_HOST);
		if (formatSPIFFS(OTA_HOST)){
			sendRpiDebug("SPIFFS formatted", OTA_HOST);
		} else {
			sendRpiDebug("SPIFFS format failed", OTA_HOST);
		}
	}

	// Set initial time
	tryGetTime(false);

	// Prepare NTP time update timer
	lastSyncTime = now();

  // Start camera connection
  // TODO put this into library!
  uint32_t foundBaud = cam.autoDetectBaudRate();
  delay(1000);

  if (foundBaud != 0) {
    if (cam.begin(foundBaud)) {
			sendRpiDebug("Camera found!", OTA_HOST);

      // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
      // Remember that bigger pictures take longer to transmit!
      cam.setImageSize(VC0706_640x480);        // biggest
      // cam.setImageSize(VC0706_320x240);        // medium
      // cam.setImageSize(VC0706_160x120);          // small

			// Reset is necessary only if resolution other than 640x480 is selected
			cam.reset();
			delay(500);
			String debugMsg = "Image size = " + String(cam.getImageSize());
			sendRpiDebug(debugMsg, OTA_HOST);
			// Serial.print("Downsize = "); Serial.println(cam.getDownsize());
      cam.setCompression(255);
			debugMsg = "Compression = " + String(cam.getCompression());
			sendRpiDebug(debugMsg, OTA_HOST);

      // //  Motion detection system can alert you when the camera 'sees' motion!
      // cam.setMotionDetect(true);           // turn it on
      // Serial.print("Motionstatus = "); Serial.println(cam.getMotionDetect());
			cam.resumeVideo();

			digitalWrite(flashLED,LOW);
    }
  } else {
		sendRpiDebug("No camera found?", OTA_HOST);
    doubleLedFlashStart(1);
    hasCamera = false;
  }

	// Prepare OTA update listener
	ArduinoOTA.onStart([]() {
		wdt_disable();
		String debugMsg = "OTA start";
		sendRpiDebug(debugMsg, OTA_HOST);
		Serial.println(debugMsg);
		doubleLedFlashStart(0.1);
		WiFiUDP::stopAll();
		WiFiClient::stopAll();
		tcpServer.close();
		otaRunning = true;
	});

	// Start OTA server.
	ArduinoOTA.setHostname(hostApName);
	ArduinoOTA.begin();

	MDNS.addServiceTxt("arduino", "tcp", "board", "ESP8266");
	MDNS.addServiceTxt("arduino", "tcp", "type", camDevice);
	MDNS.addServiceTxt("arduino", "tcp", "id", String(hostApName));
	MDNS.addServiceTxt("arduino", "tcp", "service", mhcIdTag);
	MDNS.addServiceTxt("arduino", "tcp", "loc", String(devLoc));

	// Start heart beat sending every 1 minutes
	heartBeatTimer.attach(60, triggerHeartBeat);
	sendBroadCast(false);
	inSetup = false;
	wdt_enable(WDTO_8S);
}
