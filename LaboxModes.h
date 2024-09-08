//-------------------------------------------------------------------
#ifndef __LaboxModes_hpp__
#define __LaboxModes_hpp__
//-------------------------------------------------------------------

#include <Arduino.h>
#include "defines.h"

enum ProgType {
	MAIN = 'M',
	CV1ADDR = 'P',
	CVREAD = 'Q',
	CVWRITE = 'R',
	SILENTRETURNTOMAIN = 'B'
};

/** This is a class to handle external communications.
An instance of this class receive message from external world and call DCCEX API functions.
*/
class LaboxModes {
	public:
		// True if the ESP is in programmation mode.
    static bool progMode;
		// Type of programmation mode.
    static ProgType progModeType;
		// Ture if the boot should not show logos and startup messages on the screen.
    static bool silentBootMode;
		// Position of the byte 'M' or 'P' for Main or Prog track mode.
    static int EEPROMModeProgAddress;

  	static void GetCurrentMode();
		static void SetNextMode(ProgType inType = MAIN);
		static void Restart(ProgType inType = SILENTRETURNTOMAIN);
};
	
#endif