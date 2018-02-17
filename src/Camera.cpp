#include <Setup.h>

/**
	Take camera shot and save to FTP & SPIFFS
	@return <code>boolean</code>
			true if photo was taken and saved to FTP & SPIFFS
			false if error occured
*/
boolean takeShot() {
	String debugMsg;
	wdt_reset();
	if (debugOn) {
		sendRpiDebug("takeShot started", OTA_HOST);
	}
	comLedFlashStart(0.1);
	digitalWrite(flashLED, HIGH);
	wdt_reset();
	if (!cam.takePicture()) {
		sendRpiDebug("Failed to take picture!", OTA_HOST);
		wdt_reset();
		cam.resumeVideo();
		comLedFlashStop();
		digitalWrite(flashLED, LOW);
		return false;
	}
	// Get the size of the image (frame) taken
	uint16_t jpglen = cam.frameLength();
	if (debugOn) {
		debugMsg = "Got image with size " + String(jpglen);
		sendRpiDebug(debugMsg, OTA_HOST);
	}

	wdt_reset();
	if (jpglen <= 5000) {
		debugMsg = "Image size wrong: " + String(jpglen);
		sendRpiDebug(debugMsg, OTA_HOST);
		cam.resumeVideo();
		comLedFlashStop();
		digitalWrite(flashLED, LOW);
		return false;
	}

	// Create an image with the name MM-DD-hh-mm-ss.JPG
	// char filename[19];
	String dateTime = getDigits(month());
	dateTime += "-" + getDigits(day());
	dateTime += "-" + getDigits(hour());
	dateTime += "-" + getDigits(minute());
	dateTime += "-" + getDigits(second());
	for (int index=0; index < 14; index ++) {
		filename[index] = dateTime[index];
	}
	filename[14] = '.';
	filename[15] = 'j';
	filename[16] = 'p';
	filename[17] = 'g';
	filename[18] = 0;

	if (debugOn) {
		debugMsg = "Saving " + String(filename) + " Image size: " + String(jpglen);
		sendRpiDebug(debugMsg, OTA_HOST);
	}

	wdt_reset();
	// Prepare file to save image
	bool fileOpen = true;
	File imgFile = SPIFFS.open("/last.jpg", "w");
	if (imgFile == 0) {
		fileOpen = false;
		sendRpiDebug("Failed to open file /last.jpg", OTA_HOST);
		return false; // Without a file there is nothing to do here
	}

	// Switch off the flash light
	digitalWrite(flashLED, LOW);

	uint32_t bytesWrittenFS = 0;

	// Read all the data up to jpglen # bytes!
	uint32_t startTime = millis();
	uint32_t timeOut = millis(); // timout counter
	uint8_t bytesToRead;
	uint8_t readFailures = 0;
	while (jpglen > 0) {
		uint8_t *buffer;
		if (jpglen < 32) {
			bytesToRead = jpglen;
		} else {
			bytesToRead = 32;
		}
		wdt_reset();
		buffer = cam.readPicture(bytesToRead);
		if (buffer == 0) {
			readFailures++;
			if (readFailures > 1000) { // Too many read errors, better to stop
				sendRpiDebug("Read from camera failed", OTA_HOST);
				jpglen = 0;
				if (fileOpen) {
					wdt_reset();
					imgFile.close();
				}
				return false;
				break;
			}
		} else {
			wdt_reset();
			bytesWrittenFS += imgFile.write((const uint8_t *) buffer, bytesToRead);
			jpglen -= bytesToRead;
		}
	}

	if (fileOpen) {
		wdt_reset();
		imgFile.close();
	}

	uint32_t endTime = millis();
	digitalWrite(flashLED, LOW);
	if (debugOn) {
		float transRate = ((float)bytesWrittenFS/1024)/((float)(endTime-startTime)/1000.0);
		debugMsg = "Read from camera finished: " + String(bytesWrittenFS) + " bytes in " + String(endTime-startTime) + " ms -> " + String(transRate) + " kB/s";
		sendRpiDebug(debugMsg, OTA_HOST);
	}

	// Restart camera
	wdt_reset();
	cam.resumeVideo();

	if (fileOpen) {
		// Prepare file to read image
		wdt_reset();
		bool fileOpen = true;
		File imgFile = SPIFFS.open("/last.jpg", "r");
		if (imgFile == 0) {
			fileOpen = false;
			sendRpiDebug("Failed to open file /last.jpg", OTA_HOST);
		} else {
			jpglen = imgFile.size();
			uint32_t bytesReadFS = 0;
			#define bufSizeFTP 1440 // depends on available space
			uint8_t clientBuf[bufSizeFTP];
			// size_t clientCount = 0;
			uint32_t bytesWrittenFTP = 0;
			// Prepare FTP connection
			bool ftpConnected = true;
			if (debugOn) {
				sendRpiDebug("Connecting to FTP", OTA_HOST);
			}
			wdt_reset();
			if (WiFi.isConnected()) {
				if (!ftpConnect()) {
					ftpConnected = false;
					sendRpiDebug("Connecting to FTP failed", OTA_HOST);
				}
			} else {
				ftpConnected = false;
			}

			if (ftpConnected) {
				// Prepare data upload
				ftpClient.println(F("CWD /var/www/html/1s"));
				// Check result
				wdt_reset();
				if (!ftpReceive()) {
					debugMsg = "FTP: CD failed: " + String(ftpBuf);
					sendRpiDebug(debugMsg, OTA_HOST);
					ftpDataClient.stop();
					ftpClient.stop();
					ftpConnected = false;
					// Maybe we lost WiFi connection!
					wmIsConnected = false;
				} else {
					ftpClient.print(F("STOR "));
					ftpClient.println(filename);
					// Check result
					wdt_reset();
					if (!ftpReceive()) {
						debugMsg = "FTP: Passive mode not available: " + String(ftpBuf);
						sendRpiDebug(debugMsg, OTA_HOST);
						ftpDataClient.stop();
						ftpClient.stop();
						ftpConnected = false;
						// Maybe we lost WiFi connection!
						wmIsConnected = false;
					}
				}
			}

			if (ftpConnected) {
				startTime = millis();
				// Get data from file and send to FTP
				for (int blocks = 0; blocks < ((jpglen/1440)+1); blocks++) {
					bytesReadFS = imgFile.read(clientBuf, 1440);
					bytesWrittenFTP += ftpDataClient.write((const uint8_t *) clientBuf, bytesReadFS);
					wdt_reset();
				}
				endTime = millis();
				imgFile.close();

				if (debugOn) {
					float transRate = ((float)bytesWrittenFTP/1024)/((float)(endTime-startTime)/1000.0);
					debugMsg = "Save to FTP finished: " + String(bytesWrittenFTP) + " bytes in " + String(endTime-startTime) + " ms -> " + String(transRate) + " kB/s";
					sendRpiDebug(debugMsg, OTA_HOST);
				}

				// Close FTP connection
				wdt_reset();
				ftpDataClient.stop();
				ftpClient.println("QUIT");
				// Check result
				wdt_reset();
				if (!ftpReceive()) {
					debugMsg = "FTP: Disconnect failed: " + String(ftpBuf);
					sendRpiDebug(debugMsg, OTA_HOST);
					// Maybe we lost WiFi connection!
					wmIsConnected = false;
				}
				if (debugOn) {
					sendRpiDebug("STOP FTP", OTA_HOST);
				}
				ftpClient.stop();
			}
		}
	}

	comLedFlashStop();
	return true;
}
