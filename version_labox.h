#ifndef version_labox_h
#define version_labox_h

#define VERSION_LABOX "2.20.0"
// 2.20.0 - Add LABOX_SERIAL_INPUT define in config.h to enable Serial2 input for LaBox. This allows to use Serial2 for input commands in LaBox, which can be useful for certain configurations or setups. The Serial2 input can be used for various purposes, such as receiving commands from an external device or interface, and can be configured with the appropriate RX and TX pins and baud rate in config.h.
//        - Fix in DC mode when the direction change is requested by an external device (Z21, WiThrottle, etc...) the change was not applied immediately, but only after the next speed change. Now, the direction change is applied immediately when requested by an external device, allowing for more responsive control of the train in DC mode.
//				- A diagnostic mesage 'HmiInterface::ChangeSpeed' was still present in the code, which was not necessary and could cause confusion. This message has been removed to clean up the code and avoid unnecessary diagnostic output.
//				- In Wifi information menu, the UDP port is now shown, allowing users to easily identify the port used for Z21 communication and troubleshoot any issues related to Z21 connectivity or communication.
//				- CommandStation-EX code base passed to version 5.6.4 : fix a bug in EXRAIL. All the other fixes are not related to LaBox.
//				- On Locoduino splash screen, the version of the LaBox is now shown, allowing users to easily identify the version of the LaBox firmware and CommandStation_EX version they are running and ensure they are using the latest version with all the latest features and bug fixes.
// 2.19.0 - Fix the DCC++ command 'c' only used by old versions of JMRI and 'DCC-EX Native Throttle' (https://github.com/RB211/DCC_Ex_Driver/)
// 2.18.0	- Fix DC mode setting speed to 0 or 1 when using select double click on the box.
//				- Fix the DC mode with multiple loco slots, as it was not working properly when more than one loco slot was used in DC mode. Now, the DC mode can handle multiple loco slots correctly, allowing to control multiple trains in DC mode without issues.
// 2.17.0	- Fix the DC address always present in the DCC driving screen...
// 200726 - In DC mode, fix the perpetual movement 0/1 of the speed on the driving screen !
//				-	The library Adafruit SSD1306 poassed to 2.5.17
//				- The ESP32 platform for Platform.IO passed to 7.0.1, but is still uncompatible with ESP32 Env 3 !
// 2.16.0	- Add a LABOX_PROG_LED define in config.h to manage a LED to show the prog mode activity. This is only useful for the configurations with only a prog track, as it allows to have a visual feedback of the prog mode activity, which is not the case when the prog track is also used as main track.
// 120526	- Improve Z21Throttle to match the work of Harald Barth in the branch Devel-z21 of CS-EX.
//				- Z21 now handles broadcasting also if LaBox restart and Z21 app does not, fixing a long delay of reactivity of the Z21 app when the LaBox is restarted while the Z21 app is running. This is done by setting the default value of broadcast flags to BROADCAST_BASE in the Z21Throttle constructor, which allows to broadcast loco and turnout infos to the app even if the app does not resend the LAN_SET_BROADCASTFLAGS after a restart of the LaBox.
//				- Z21 now handles turnout information request and creates the turnout if it does not exist, which is useful for JMRI users as it allows to have the turnout information in JMRI without having to create the turnout in DCC++ EX first.
//				- Z21 now handles sensor informations.
//				- Z21 now handles Loco functions 29 to 31 .
//				- Z21 now generates less notifications to the clients, which should improve performances. For example, when a loco speed or function is changed, only the concerned loco information is sent to the clients instead of sending all locos information. This is especially useful for users with a lot of locos, as it avoids to send a lot of unnecessary information to the clients when only one loco is concerned.
//				- The project now use the library Adafruit GFX Library in version 1.12.6.
//				- If WIFI_LED is defined in config.h , the defined pin will be turned off when the Wifi is stopped by the menu or a CV command.
//				-	New version of XPressNet interface.
//				- Add CAN_ACCESSORY command in CanMarklin interface to handle accessory commands from Marklin Central Station 3. Thank to lebelge2 for the idea and the code !
//				- Command 'Restart' removed from menu.
//				- New DC mode !
//				- CommandStation-EX code base passed to version 5.6.0 : Railcom, consist, momentum, websockets, sound, and many other improvements and bug fixes from the main branch of CommandStation-EX have been integrated in the Labox branch. This is a big step for the Labox project, as it allows to benefit from all the latest improvements and bug fixes of CommandStation-EX, and to have a more stable and performant code base. A big thank to all the contributors of CommandStation-EX for their work !
// 2.15.0 - If the button 'UP' is pressed at startup, the LaBox will start in Main mode instead of Prog mode. This is useful when the LaBox is used in a configuration with only a main track and no prog track, as it avoids to have to press the button at every startup to switch to main mode.
// 060426	- Fix the reboot mode when only a main track is declared, as it was not working anymore with the latest changes in LaboxModes. Now, if the motor shield name is "RebootProgMode", the reboot mode will be used instead of the joining mode when only a prog track is declared. This allows to use the joining mode for configurations with only a prog track, and the reboot mode for configurations with only a main track, which is more logical.
//				- In Wifi information menu, SSID and Password are now shown for both AP and STA modes.
// 2.14.0 - Add new syntax for LaBox functions : <LB ...> commands in DCC++ EX parser.
// 310326	- Add LABOX_CV_ADDRESS in config.h to manage LaBox specific CVs.
//				- Modify CV LABOX_CV_ADDRESS+1 to stop wifi and LABOX_CV_ADDRESS+7 to stop/start Railcom.
//				- Add new command 'Settings' in LaBox menu to manage wifi and Railcom settings. Thank to lebelge2 for the idea !
//				- Add new command Programmation Mode in LaBox menu to enter/exit prog mode and show every programmation activity.
//				- LABOX_PROG_MOTOR_SHIELD redefined in config.h to have its own name.	
//				- In prog mode, Railcom is paused to avoid interference.
//				- Railcom is now propagated to boosters if they are declared in config.h.
//				- Text for Wifi opening show now the Wifi mode (AP or STA) and not only 'Wifi opening'.
//				- Command Information/WIFI now show the Wifi mode and works for AP mode.
//				- Current measure gives now 0 when the DCC is off.
//				- Platform.IO : Update of used libraries :
//						mathertel/OneButton@^2.6.2
//						adafruit/Adafruit GFX Library@^1.12.5
//						adafruit/Adafruit SSD1306@^2.5.16
// 2.13.0 - Fix display of function symbol on train dashboard.
//				- Undefine HMI_DEBUG_SIMUL to avoid U and I simulation values in hmi.cpp .
// 2.12.0 - Add HMI_DELTACURRENT in config.h to calibrate the current displayed on the OLED screen.
// 				- Add YOUR_MOTOR_SHIELD_TYPE in config.h to choose the right motor shield without the need to change the .ino file.
//				- Add INVERT_BOOSTER_OUTPUT in config.h to invert booster output if needed.
//				- Platform.IO : Update of used libraries :
//						adafruit/Adafruit GFX Library@^1.12.3
//						adafruit/Adafruit SSD1306@^2.5.15
//				- Platform.IO : Update of used Expressif platform 6.7.0 -> 6.12.0
//				- The command 'Measures U and I' is now operational.
// 2.11.0 - Fixing IP Address for Wifi is now possible.
//				- Railcom last modifications from lebelge2 integrated.
//				- Big changes in LaboxModes to manage correctly the different configurations.
//				- Add defines for Sprog to fix the used serial interface inside config.h .
//				- The CV address and its value can be set by using WiThrottle or Z21 loco speed/dir interfaces. Thank to lebelge2 for the idea !
//				- Fix the pin 27 added to RMT not only for Railcom.
//				- Fix power notification on serial command input, as WebThrottle does it.
// 				- CommandStation-EX code base passed 5.4.15 !
//
// 2.10.0	- CommandStation-EX code base passed from 5.0.9 to 5.4.10 !
//				- Fix two trains dashboard graphics.
//				- Fix toggle between DCCon and DCCoff for graphic.
//				- Fix the way multi trains will be affected to graphic view.
//				- Add HMI_DASHBOARD_TRAIN_NB in config.h for initial driving view style.
//				- Add HMI_SCREEN_ROTATION to be able to orientate the screen as needed.
//				- If a main AND a prog track are declared, avoid to reboot ESP when using CV handling commands.
//				- Fix a lot of warnings/potential errors.
//				- Update of used libraries :
//						adafruit/Adafruit GFX Library@^1.12.0	
//						adafruit/Adafruit SSD1306@^2.5.14
// 2.9.1	- Menu option DCC ON/OFF is available.
//				- Some unused textes has been removed in French and English in hmiconfig.h
//				- Railcom is no more started in prog mode.
// 2.9.0	- Add POM programming in Z21 interface.
//				- Update of used libraries :
//						mathertel/OneButton@^2.6.1
//						adafruit/Adafruit GFX Library@^1.11.11	
//						adafruit/Adafruit SSD1306@^2.5.12
//				- Code has been added for DC mode, but is not functionnal for the moment.
//				- Documentation of various protocols implemented (or not yet) in LaBox 
//					has been added in Release_Notes folder.
// 2.8.0	- Add command to identify the Decoder using CV7/CV8.
//				- CAN : the version of CAN interface is added to CAN information.
//				- Add command 'Shuttle test' with a -very- small shuttle automation program.
//				- When emergency stop used, the Oled screen give 'ESTOP' as speed.
// 2.7.0	-	Add broadcast functions to EXComm
//				- EXComm/EXCommItem classes refactoring to rationalize code.
//				-	Z21Throttle class has been cleaned for scories of old code, class MYLOCOZ21 removed, 
//					and some 'notify' functions become static to be able to braodcast from EXCommItem class.
//				- Cleaning of the menu, removing empty commands.
//				- A new menu option 'Informations' has been added. Sub options give the way to show informations
//					of Labox itslef (About...), wifi status, and all EXComm items.
// 2.6.6	-	Fix AP WiFi mode for Z21 and WiThrottle.
// 2.6.5	- Reduce imprint of Railcom code in DCCRMT.cpp
//				-	Improvements in CanMarklin
//				-	First CAN message shows its version.
//				-	[PCB] New PCB with lots of changed attributes nedded by the BOM and placement files.
//				-	[PCB] Gerber files are now in Kicad/Fabrication instead of Gerber.
//				- [PCB] New Gerber files of the PCB with BOM files generated by the 'Fabrication Toolkit' plugin of Kicad for JLCPCB.
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
