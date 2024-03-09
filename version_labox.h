#ifndef version_labox_h
#define version_labox_h

#define VERSION_LABOX "2.4.6"
// 2.4.6  - Stop button for Z21 apps are now correctly handled. (Thanks Gilles/gbo95)
//        - The power state is transmitted to all connected Z21 apps. (Thanks Gilles/gbo95)
// 2.4.5  - New delay Z21_TIMEOUT to disconnect z21 throttle without any communication... .
//        - Black Z21 app works now, after removing a surprising 1000 bytes UDP message !
//        - Big UDP packet with multiple Z21 messages are now correctly handled.
//        - Turnout switching works now.
// 2.4.4  - Add AUTOMATIC_POWER_RESTORE to allow power restart on the first speed/direction/function change after POWEROFF_ONDELAY .
// 2.4.3  - Fix CV write on Z21 app.
//        - First try to implement turnout setting by Z21 app. Not yet fonctionnal !
//        - Creation of this file !
// 2.4.2  - Merge with CommandStation-EX 5.0.9
//        - Fix long address reading with 16384...
// 2.4.1  - Fix use of define USE_HMI for compilation.
//        - Fix value of HMI_deltaCurrent to 0
// 2.4.0  - First operationnal version on ESP32.

#endif
