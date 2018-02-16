#include <Setup.h>

/** Counter for "I am alive" red LED blinking in loop() */
long liveCnt = 0;

void loop() {
	wdt_reset();
	// Handle OTA updates
	ArduinoOTA.handle();

	if (otaRunning) { // If the OTA update is active we do nothing else here in the main loop
		return;
	}

	// Resync time every 12 hours
	// if (now() > lastSyncTime+43200) {
	// 	tryGetTime(false);
	// 	lastSyncTime = now();
	// }

	wdt_reset();
	// Handle new request on tcp socket server if available
	WiFiClient tcpClient = tcpServer.available();
	if (tcpClient) {
		socketServer(tcpClient);
		digitalWrite(flashLED, LOW);
	}

  // // Check if motion was detected
  // if (hasCamera) {
  //   if (cam.motionDetected()) {
  //     cam.setMotionDetect(false);
  //     takeShot();
  //     cam.setMotionDetect(true);
  //   }
  // }

	wdt_reset();
	// Give a "I am alive" signal
	liveCnt++;
	if (liveCnt >= 100000) { // 100000
		digitalWrite(blinkLED, !digitalRead(blinkLED));
		liveCnt = 0;
	}

	wdt_reset();
	if (heartBeatTriggered) {
		if (debugOn) {
			sendRpiDebug("heartBeatTriggered", OTA_HOST);
		}
		heartBeatTriggered = false;
		if (!WiFi.isConnected()) {
			wdt_disable();
			WiFi.reconnect();
			wdt_enable(WDTO_8S);
		}
		// if (!wmIsConnected) { // Connection to WiFi lost, retry to connect
		// 	// Try to connect to WiFi with captive portal
		// 	wdt_disable();
		// 	ipAddr = connectWiFi(ipAddr, ipGateWay, ipSubNet, "ESP8266 CAM 1");
		// 	wdt_enable(WDTO_8S);
		// }
		// In case we don't have a time from NTP or local server, retry
		if (!gotTime) {
			sendRpiDebug("tryGetTime", OTA_HOST);
			wdt_disable();
			tryGetTime(false);
			wdt_enable(WDTO_8S);
		}

		// Stop the tcp socket server
		tcpServer.stop();
		// Handle OTA updates
		ArduinoOTA.handle();
		// Restart the tcp socket server to listen on port tcpComPort
		tcpServer.begin();
		// Give a "I am alive" signal
		sendBroadCast(false);
	}

	// TODO find out why sometimes the flashLED is on!!!!!!!!!!!!
	digitalWrite(flashLED, LOW);
}
