#include <Setup.h>

/** Counter for "I am alive" red LED blinking in loop() */
long liveCnt = 0;
/** Flag if initialization is done */
bool notInitialized = true;

void loop() {
	wdt_reset();
	// Handle OTA updates
	ArduinoOTA.handle();

	if (otaRunning) { // Do nothing anymore
		wdt_reset();
		return;
	}

	wdt_reset();
	// Check current WiFi status
	checkWiFiStatus();

	// Give a "I am alive" signal
	liveCnt++;
	if (liveCnt >= 100000) { // 100000
		digitalWrite(blinkLED, !digitalRead(blinkLED));
		liveCnt = 0;
	}

	if (connStatus == CON_GOTIP) {
		wdt_reset();
		if (notInitialized) {
			// Initialize OTA
			initOTA();
			
			// Set initial time
			if (!tryGetTime(debugOn)) {
				tryGetTime(debugOn); // Failed to get time from NTP server, retry
			}
			if (gotTime) {
				lastKnownYear = year();
			} else {
				lastKnownYear = 0;
			}

			String debugMsg = "Reconnected to " + WiFi.SSID();
			debugMsg += " " + digitalClockDisplay();
			sendRpiDebug(debugMsg, OTA_HOST);
			// We are connected, check if we need to do some initializations
			// Start UDP listener
			startListenToUDPbroadcast();
			// Start the tcp socket server to listen on port tcpComPort
			tcpServer.begin();
			// Send Security restart/reconnect message
			sendBroadCast(false);
			 
			notInitialized = false;
		}
		// Here comes some tasks that make only sense if we are connected
		// Try to catch time changing Bug
		wdt_reset();
		if ((lastKnownYear != 0) && (year() != lastKnownYear) && gotTime) {
			if (!tryGetTime(debugOn)) {
				tryGetTime(debugOn);
			}
			if (gotTime) {
				lastKnownYear = year();
			}
		}

		wdt_reset();
		// Handle new request on tcp socket server if available
		WiFiClient tcpClient = tcpServer.available();
		if (tcpClient) {
			if (debugOn) {
				sendDebug("tcpClient connected", OTA_HOST);
			}
			comLedFlashStart(0.2);
			socketServer(tcpClient);
			comLedFlashStop();
		}

		wdt_reset();
		if (heartBeatTriggered) {

			if (!gotTime) { // Got no time from the NTP server, retry to get it
				if (!tryGetTime(debugOn)) {
					tryGetTime(debugOn); // Failed to get time from NTP server, retry
				}
			}
			heartBeatTriggered = false;
			// Give a "I am alive" signal
			sendBroadCast(false);
		}

		// Check if broadcast arrived
		int udpMsgLength = udpListener.parsePacket();
		if (udpMsgLength != 0) {
			if (getIdFromUDPbroadcast(udpMsgLength)) {
				if (debugOn) {
					String debugMsg = "Found: Camera IP: " + camIp.toString()
													+ " Security IP: " + secIp.toString()
													+ " Light IP: " + lightIp.toString();
					sendDebug(debugMsg, OTA_HOST);
				}
			}
		}
	} else if (connStatus == CON_LOST) {
		wdt_reset();
		// We lost connection, check what we need to stop and then retry to connect
		// Stop UDP listener
		stopListenToUDPbroadcast();
		// Stop the tcp socket server
		tcpServer.stop();
		notInitialized = true;
	}

	wdt_reset();

	// TODO find out why sometimes the flashLED is on!!!!!!!!!!!!
	digitalWrite(flashLED, LOW);

	// // Check if motion was detected
	// if (hasCamera) {
	//	 if (cam.motionDetected()) {
	//		 cam.setMotionDetect(false);
	//		 takeShot();
	//		 cam.setMotionDetect(true);
	//	 }
	// }
}
