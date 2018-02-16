#include <Setup.h>

//-------------- FTP receive
/**
	Receive response from FTP server
  @return <code>byte</code>
      Saves received data in outBuf
      returns 0 on fail on failure, 1 on success
*/
byte ftpReceive() {
  byte respCode;
  byte thisByte;

  while (!ftpClient.available()) delay(1);

  respCode = ftpClient.peek();

  ftpCount = 0;

  while (ftpClient.available()) {
    thisByte = ftpClient.read();

    if (ftpCount < 127) {
      ftpBuf[ftpCount] = thisByte;
      ftpCount++;
      ftpBuf[ftpCount] = 0;
    }
  }

  if (respCode >= '4') {
    ftpClient.println("QUIT");
    while (!ftpClient.available()) delay(1);
    while (ftpClient.available()) {
      thisByte = ftpClient.read();
      Serial.write(thisByte);
    }
    ftpClient.stop();
    return 0;
  }
  return 1;
}

//-------------- FTP connect
/**
	connect to FTP server
  @return <code>bool</code>
      Saves received data in outBuf
      returns false on fail on failure, true on success
*/
bool ftpConnect() {
	// Open connection to FTP server
	if (!ftpClient.connect(ftpDataServer,ftpDataPort)) {
    return false;
  }

  // Check result
  if (!ftpReceive()) {
    ftpClient.stop();
    return false;
  }
  // Send user name
  ftpClient.println("USER beegee");
  // Check result
  if (!ftpReceive()) {
		if (debugOn) {
			String debugMsg = "FTP: Wrong username: " + String(ftpBuf);
			sendRpiDebug(debugMsg, OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
  // Send password
  ftpClient.println("PASS teresa1963");
  // Check result
  if (!ftpReceive()) {
		if (debugOn) {
			String debugMsg = "FTP: Wrong password: " + String(ftpBuf);
			sendRpiDebug(debugMsg, OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
	// Set binary file mode
  ftpClient.println("TYPE I");
  // Check result
  if (!ftpReceive()) {
		if (debugOn) {
			String debugMsg = "FTP: Binary file mode not available: " + String(ftpBuf);
			sendRpiDebug(debugMsg, OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
	// Set streaming mode transfer
  ftpClient.println("MODE S");
  // Check result
  if (!ftpReceive()) {
		if (debugOn) {
			String debugMsg = "FTP: Streaming mode not available: " + String(ftpBuf);
			sendRpiDebug(debugMsg, OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
  // Request passive mode
  ftpClient.println("PASV");
  // Check result
  if (!ftpReceive()) {
		if (debugOn) {
			String debugMsg = "FTP: Passive mode not available: " + String(ftpBuf);
			sendRpiDebug(debugMsg, OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
  // Check passive request response
  char *tStr = strtok(ftpBuf, "(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++) {
    tStr = strtok(NULL, "(,");
    array_pasv[i] = atoi(tStr);
  }
  // Get passive mode data port
  unsigned int hiPort, loPort;
  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;
  hiPort = hiPort | loPort;

  if (!ftpDataClient.connect(ftpDataServer, hiPort)) {
		if (debugOn) {
			sendRpiDebug("FTP: Data connection failed", OTA_HOST);
		}
    ftpClient.stop();
    return false;
  }
	return true;
}
