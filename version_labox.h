#ifndef version_labox_h
#define version_labox_h

#define VERSION_LABOX "2.6.4"
// 2.6.4	- Fix XpressNet.cpp file format.
//				- Try to fix the crash on some ESP32 when RailCom and CAN are both activated.
//				  Dont know why this modification works, but it works !
// 2.6.3	- Add ReadCV and WriteCV menu options
//				-	Add XPressNet interface in EXComm format and config.Labox.h. Not tested. Thanks lebelge.
//				- Move SPROG interface from Serial2 to Serial1 to allow XPressnet existing with SPROG.
//				- Fix SPROG for rx/tx pin values and broadcasting CV values.
//				- CAN Speed in console should be correct.
//				-	Added class LaboxModes to centralize EEPROM and ESP restart behaviours.
// 2.6.2	- Fix Railcom
//				- Improve CanMarklin for better compatibility.
// 2.6.1	- Add Railcom
// 2.6.0	- New EXComm class to manage external communications.
//				- CANMarklin class added in EXComm
//				- SProg protocol EXComm class for Serial2 .
//				- Z21 Throttle converted in EXComm .
// 2.5.2	- Railcom simplification to get pins from MotorDriver
//				-	Fix Railcom in prog mode
//				- Fix Function number < 28 on Oled screen.
// 2.5.1	- Railcom fixes on MotorDriver and DCC preamble.
// 2.5.0 	-	Railcom integration
// 2.4.9	- Fix Function number < 28 on Oled screen.
// 2.4.8	- Add check of ESP Framework (got from CommandStation-EX master branch...)
// 2.4.7  - NotifyTrPw() XOR error fixed.
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
