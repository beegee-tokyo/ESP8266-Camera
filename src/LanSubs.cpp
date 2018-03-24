#include <Setup.h>

/** WiFiUDP class for creating UDP communication */
WiFiUDP udpClientServer;

/**
	 sendBroadCast
	 send updated status over LAN
	 by UTP broadcast over local lan
	 @param shotResult
	 	Flag if taking an image was successful
*/
void sendBroadCast(boolean shotResult) {
	comLedFlashStart(0.4);

	DynamicJsonBuffer jsonBuffer;
	// Prepare json object for the response
	JsonObject& root = jsonBuffer.createObject();
	root["de"] = DEVICE_ID;

	String nowTime = String(hour()) + ":";
	if (minute() < 10) {
		nowTime += "0";
	}
	nowTime += String(minute());
	root["tm"] = nowTime;

	if (shotResult) {
		root["pi"] = 1;
		root["fi"] = String(filename);
	} else {
		root["pi"] = 0;
	}
	// Convert JSON object into a string
	String broadCast;
	root.printTo(broadCast);
	// Broadcast per UDP to LAN
	if (udpClientServer.beginPacketMulticast(multiIP, udpBcPort, ipAddr) == 0) {
	} else {
		if (udpClientServer.print(broadCast) == 0) {
		}
		udpClientServer.endPacket();
		udpClientServer.stop();
	}
	comLedFlashStop();
}

/**
	socketServer
	answer request on tcp socket server
	returns status to client
	@param tcpClient
		Instance of WiFiClient that has connected
	*		Commands:
	*		t take a picture
	*		x to reset the device
	*		y=YYYY,MM,DD,HH,mm,ss to set time and date
	*		z to format SPIFFS
	*		zloc=[40] location of the device
	*		zsec=[40] connected security device
	*		zcam=[40] connected camera device
	*		zlight=[40] connected light device
	*/
void socketServer(WiFiClient tcpClient) {
	comLedFlashStart(0.4);

	// Get data from client until he stops the connection or timeout occurs
	long timeoutStart = now();
	char rcvd[128];
	String cmd;
	byte index = 0;

	while (tcpClient.connected()) {
		if (tcpClient.available()) {
			rcvd[index] = tcpClient.read();
			index++;
			if (index >= 128) break; // prevent buffer overflow
		}
		if (now() > timeoutStart + 3000) { // Wait a maximum of 3 seconds
			break; // End the while loop because of timeout
		}
	}
	rcvd[index] = 0;

	tcpClient.flush();
	tcpClient.stop();

	// Copy received buffer into a string for easier handling
	String req(rcvd);

	if (req.length() < 1) { // No data received
		comLedFlashStop();
		return;
	}

	// Take a picture
	if (req.substring(0, 1) == "t"){
		digitalWrite(flashLED, HIGH);
		if (debugOn) {
			sendRpiDebug("Request to take a picture", OTA_HOST);
		}
		bool picResult = takeShot();
		if (picResult) {
			sendBroadCast(picResult);
		} else if (debugOn) {
			sendRpiDebug("Error taking a picture", OTA_HOST);
		}
	// Switch on debug output
	} else if (req.substring(0, 1) == "d") {
		debugOn = !debugOn;
		if (debugOn) {
			sendRpiDebug("Debug over TCP is on", OTA_HOST);
		} else {
			sendRpiDebug("Debug over TCP is off", OTA_HOST);
		}
		return;
		// Date/time received
	} else if (req.substring(0, 2) == "y=") {
		int nowYear = 0;
		int nowMonth = 0;
		int nowDay = 0;
		int nowHour = 0;
		int nowMinute = 0;
		int nowSecond = 0;

		if (isDigit(req.charAt(2))
		&& isDigit(req.charAt(3))
		&& isDigit(req.charAt(4))
		&& isDigit(req.charAt(5))
		&& isDigit(req.charAt(7))
		&& isDigit(req.charAt(8))
		&& isDigit(req.charAt(10))
		&& isDigit(req.charAt(11))
		&& isDigit(req.charAt(13))
		&& isDigit(req.charAt(14))
		&& isDigit(req.charAt(16))
		&& isDigit(req.charAt(17))
		&& isDigit(req.charAt(19))
		&& isDigit(req.charAt(20))) {
			cmd = req.substring(2, 6);
			int nowYear = cmd.toInt();
			cmd = req.substring(7, 9);
			int nowMonth = cmd.toInt();
			cmd = req.substring(10, 12);
			int nowDay = cmd.toInt();
			cmd = req.substring(13, 15);
			int nowHour = cmd.toInt();
			cmd = req.substring(16, 18);
			int nowMinute = cmd.toInt();
			cmd = req.substring(19, 21);
			int nowSecond = cmd.toInt();

			if (debugOn) {
				String debugMsg = "Changed time to " + String(nowYear) + "-" + String(nowMonth) + "-" + String(nowDay) + " " + String(nowHour) + ":" + String(nowMinute) + ":" + String(nowSecond);
				sendRpiDebug(debugMsg, OTA_HOST);
			}
			setTime(nowHour,nowMinute,nowSecond,nowDay,nowMonth,nowYear);
			gotTime = true;
		} else {
			String debugMsg = "Received wrong time format: " + req;
			sendRpiDebug(debugMsg, OTA_HOST);
		}
		// Location received
	} else if (req.substring(0,5) == "zloc=") {
		// copy location
		devLoc = req.substring(5);
		// save new location
		if (!saveConfigEntry("loc", (char *)&devLoc[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		MDNS.addServiceTxt("arduino", "tcp", "loc", devLoc);
		MDNS.update();
		return;
		// Light device ID received
	} else if (req.substring(0,7) == "zlight=") {
		// copy light device ID
		lightID = req.substring(7);
		// save new light device ID
		if (!saveConfigEntry("light", (char *)&lightID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Security device ID received
	} else if (req.substring(0,5) == "zsec=") {
		// copy security device ID
		secID = req.substring(5);
		// save new security device ID
		if (!saveConfigEntry("sec", (char *)&secID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Camera device ID received
	} else if (req.substring(0,5) == "zcam=") {
		// copy camera device ID
		camID = req.substring(5);
		// save new camera device ID
		if (!saveConfigEntry("cam", (char *)&camID[0]) && debugOn) {
			sendDebug("failed to write to config file for writing", OTA_HOST);
		}
		return;
		// Reset device
	} else if (req.substring(0, 1) == "x") {
		sendRpiDebug("Reset device", OTA_HOST);
		// Reset the ESP
		delay(3000);
		ESP.reset();
		delay(5000);
		// Format SPIFFS
	} else if (req.substring(0, 1) == "z") {
		formatSPIFFS(OTA_HOST);
	}
	comLedFlashStop();
}
