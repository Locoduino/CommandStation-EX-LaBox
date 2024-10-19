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
/* SEQUENCE(1) is a simple shuttle between 2 sensors      
 *  S20 and S21 are sensors on arduino pins 20 and 21 
 *  S20                    S21                   
 *  === START->================
 */
   SEQUENCE(123) 
     FON(3)       // Set Loco Function 3, Horn on
     DELAY(1000)    // wait 1 second
     FOFF(3)      // Horn off
     FWD(20)      // Move forward at speed 80
     DELAY(1500)    // Wait 5 seconds
     STOP         // then stop
     FON(2)       // ring bell
     REV(20)      // reverse at speed 60
     DELAY(1500)    // Wait 5 seconds
     STOP         // then stop
     FOFF(2)      // Bell off 
		 DELAY(2000)
     FOLLOW(123)    // and follow sequence 1 again
   
