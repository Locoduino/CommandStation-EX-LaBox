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
	SILENTRETURNTOMAIN = 'B',
	IDENTIFY = 'I'
};

/*
According to the kind of track configuration, there is 3 possible behaviors :
- The base configuration is a prog track only, which joins the main track for normal operations.
	In that case, the program behavior is "joining". This is the default configuration.
- If there is a Main track and boosters, the program behavior is "rebooting" the system to change
	the configuration. This does not always work with Railcom.
- If there is a Main and a Prog track, the program behavior is "normal".

A Main track only configuration is not anymore necessary, as the prog track can join the main track.

Name					Config																main									prog

Prog only			NULL PROG NULL NULL 									joined								unjoined
Main only			MAIN NULL NULL NULL										reboot (not railcom)	reboot
Main+boosts		MAIN NULL BOOST1 BOOST2								reboot (not railcom)	reboot
Both					MAIN PROG NULL NULL										normal								normal
Both+boost		MAIN PROG BOOST NULL									normal								normal
*/
enum ProgBehavior {
	ProgBehaviorNone = 0,
	ProgBehaviorNormal = 1,		// a main and a prog track exists
	ProgBehaviorJoining = 2,	// only a prog track exists, joined for normal operations
	ProgBehaviorReboot = 3    // only a main track, rebooting the system to change config. DONT WORK WITH RAILCOM !
};

/** This is a class to handle external communications.
An instance of this class receive message from external world and call DCCEX API functions.
*/
class LaboxModes {
	public:
		static void GetStartingMode();
		static void begin();
		static void ChangeMode(bool inProgMode, ProgType inType = ProgType::MAIN);
		static void Restart(ProgType inType = SILENTRETURNTOMAIN);

		// True if the ESP is in programmation mode.
    static bool progMode;
		// True if the boot should not show logos and startup messages on the screen.
    static bool silentBootMode;
		// Type of programmation mode : by booting, joining, or normal mode.
    static ProgBehavior progBehavior;

	private:
	  static bool DIAGLABOXMODES;

		// Type of programmation mode.
    static ProgType progModeType;
		// Position of the byte 'M' or 'P' for Main or Prog track mode.
    static int EEPROMModeProgAddress;

		static POWERMODE memoPowerBeforeJoining;  	
		
		static void StartProgMode(bool inForce = false);
		static void StartMainMode(bool inForce = false);
		static void SetNextMode(ProgType inType = MAIN);
		static void DONOTRESTART();
};
	
#endif