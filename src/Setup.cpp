#include <Setup.h>
#include <declarations.h>

/**
 * Initialization of GPIO pins, WiFi connection, timers and sensors
 */
void setup() {
	initLeds(blinkLED,flashLED); // COM LED -- ACT LED
	digitalWrite(flashLED,LOW);
	digitalWrite(blinkLED,LOW);

	Serial.begin(115200);
	Serial.setDebugOutput(false);

	// Connect to one of the two alternative AP's
	connectInit();
	// Give it 5 seconds to connect
	long waitStart = millis();
	while (0) {
		if ((connStatus == CON_GOTIP) || ((millis()-waitStart)>5000)) {
			break;
		}
	}

	// Now other intializations can be done, but WiFi might not be working yet

	// Start heart beat sending every 1 minute
	heartBeatTimer.attach(60, triggerHeartBeat);

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

	// Start camera connection
	// TODO put this into library!
	uint32_t foundBaud = cam.autoDetectBaudRate();
	delay(1000);

	if (foundBaud != 0) {
		if (cam.begin(foundBaud)) {
			sendRpiDebug("Camera found!", OTA_HOST);

			// Set the picture size - you can choose one of 640x480, 320x240 or 160x120
			// Remember that bigger pictures take longer to transmit!
			cam.setImageSize(VC0706_640x480);				// biggest
			// cam.setImageSize(VC0706_320x240);				// medium
			// cam.setImageSize(VC0706_160x120);					// small

			// Reset is necessary only if resolution other than 640x480 is selected
			cam.reset();
			delay(500);
			String debugMsg = "Image size = " + String(cam.getImageSize());
			sendRpiDebug(debugMsg, OTA_HOST);
			// Serial.print("Downsize = "); Serial.println(cam.getDownsize());
			cam.setCompression(255);
			debugMsg = "Compression = " + String(cam.getCompression());
			sendRpiDebug(debugMsg, OTA_HOST);

			// //	Motion detection system can alert you when the camera 'sees' motion!
			// cam.setMotionDetect(true);					 // turn it on
			// Serial.print("Motionstatus = "); Serial.println(cam.getMotionDetect());
			cam.resumeVideo();

			digitalWrite(flashLED,LOW);
		}
	} else {
		sendRpiDebug("No camera found?", OTA_HOST);
		doubleLedFlashStart(1);
		hasCamera = false;
	}

	// Start heart beat sending every 1 minutes
	heartBeatTimer.attach(60, triggerHeartBeat);

	wdt_enable(WDTO_8S);
	wdtEnabled = true;
}
