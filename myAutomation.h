/* This is an automation example file.
 *  The presence of a file called "myAutomation.h" brings EX-RAIL code into
 *  the command station.
 *  The automation may have multiple concurrent tasks.
 *  A task may 
 *  - Act as a ROUTE setup macro for a user to drive over 
 *  - drive a loco through an AUTOMATION 
 *  - automate some cosmetic part of the layout without any loco.
 *  
 *  At startup, a single task is created to execute the startup sequence.
 *  This task may simply follow a route, or may START  
 *  further tasks (that is.. send a loco out along a route).
 *  
 *  Where the loco id is not known at compile time, a new task 
 *  can be created with the command:
 *  </ START [cab] route> 
 *  
 *  A ROUTE, AUTOMATION or SEQUENCE are internally identical in ExRail terms  
 *  but are just represented differently to a Withrottle user:
 *  ROUTE(n,"name") - as Route_n .. to setup a route through a layout
 *  AUTOMATION(n,"name") as Auto_n .. to send the current loco off along an automated journey
 *  SEQUENCE(n) is not visible to Withrottle.
 *  
 */

// This is the startup sequence, 
/*AUTOSTART
POWERON        // turn on track power
SENDLOCO(3,1) // send loco 3 off along route 1
SENDLOCO(10,2) // send loco 10 off along route 2
DONE     // This just ends the startup thread, leaving 2 others running.
*/

// Include optional user WiThrottle roster entries from myWiThrottleRoster.h
// This file can contain any number of WiThrottle ROSTER() entries
//#include "myWiThrottleRoster.h"

/* SEQUENCE(123) is a simple DCC shuttle by timer
 */   SEQUENCE(123) 
     FON(3)       // Set Loco Function 3, Horn on
     DELAY(1000)    // wait 1 second
     FOFF(3)      // Horn off
     FWD(50)      // Move forward at speed 50
     DELAY(3000)    // Wait 3 seconds
     STOP         // then stop
     FON(2)       // ring bell
     REV(50)      // reverse at speed 50
     DELAY(3000)    // Wait 3 seconds
     STOP         // then stop
     FOFF(2)      // Bell off 
		 DELAY(2000)
     FOLLOW(123)    // and follow sequence 1 again

/* SEQUENCE(124) is a simple DC shuttle by timer
 */   SEQUENCE(124) 
 			MOMENTUM(0)  // turn off momentum for more immediate response
    	FWD(80)      // Move at speed 50
    	DELAY(3000)    // Wait 3 seconds
    	STOP         // then stop
    	DELAY(1000)    // Wait 3 seconds
    	REV(80)      // move at speed 50
    	DELAY(3000)    // Wait 3 seconds
    	STOP         // then stop
			DELAY(1000)
    	FOLLOW(124)    // and follow sequence 1 again
