#include "Setup.h"

/**
 * Sets flag heartBeatTriggered to true for handling in loop()
 * called by Ticker heartBeatTimer
 * will initiate sending out a status message from loop()
 */
void triggerHeartBeat() {
	heartBeatTriggered = true;
}
