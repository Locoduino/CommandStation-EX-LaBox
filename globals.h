/*
 * La Box Project
 * Parameters
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <arduino.h>

//************************ Global variables ****************************
//***********************************************************************


enum enumHMIState :byte {StateDashboard, StateParametersMenu, StateDashboardTrainView, StateBrowseEventLst, StateExitMenu, StateWifiWaiting, StateNoWifi } ;
enum enumEvent    :byte {noEvent, eventUp, eventDown, eventSel};
enum enumWifiMode :byte {wifiAp, wifiClient}; 

// variables must be global due to static methods
//enumEvent     _HMIEvent      = noEvent;

//************************** DEBUG LOG **********************************
//***********************************************************************
// Comment or uncomment #define if you want to see debugging information.
// Take care, to much informations is not good for the ESP32.
#define _HMIDEBUG                 // Global debugging traces. <<< Log levels only work if _HMIDEBUG is active >>>
#define _HMIDEBUG_CRITICAL           // Critical error level
#define _HMIDEBUG_ERROR              // Error level 
//#define _HMIDEBUG_FUNCTION           // Log begin and exit function
//#define _HMIDEBUG_PARAMS             // Log parameters at function beginning 
//#define _HMIDEBUG_LEVEL1             // Level 1 Log
//#define _HMIDEBUG_LEVEL2             // Level 2 Log
//#define _HMIDEBUG_LEVEL3             // Level 3 Log
//#define _HMIDEBUG_SIMUL              // simulates physical measurements
//#define _HMIDEBUG_STATE_MACHINE      // State Machine Log
//***********************************************************************

//*********************************************************************************
//------------------ Traces functions only if _HMIDEBUG is active --------------------
//*********************************************************************************
#ifdef _HMIDEBUG_FUNCTION
  #define _HMIDEBUG_FCT_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_FCT_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_FCT_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_FCT_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_FCT_PRINTLN(x)
  #define _HMIDEBUG_FCT_PRINTLN2(x,y)
  #define _HMIDEBUG_FCT_PRINT(x)
  #define _HMIDEBUG_FCT_PRINT2(x,y)
#endif 

#ifdef _HMIDEBUG_PARAMS
  #define _HMIDEBUG_PARAMS_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_PARAMS_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_PARAMS_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_PARAMS_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_PARAMS_PRINTLN(x)
  #define _HMIDEBUG_PARAMS_PRINTLN2(x,y)
  #define _HMIDEBUG_PARAMS_PRINT(x)
  #define _HMIDEBUG_PARAMS_PRINT2(x,y)
#endif
  
#ifdef _HMIDEBUG_LEVEL1
  #define _HMIDEBUG_LEVEL1_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_LEVEL1_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_LEVEL1_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_LEVEL1_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_LEVEL1_PRINTLN(x)
  #define _HMIDEBUG_LEVEL1_PRINTLN2(x,y)
  #define _HMIDEBUG_LEVEL1_PRINT(x)
  #define _HMIDEBUG_LEVEL1_PRINT2(x,y)
#endif 

#ifdef _HMIDEBUG_LEVEL2
  #define _HMIDEBUG_LEVEL2_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_LEVEL2_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_LEVEL2_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_LEVEL2_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_LEVEL2_PRINTLN(x)
  #define _HMIDEBUG_LEVEL2_PRINTLN2(x,y)
  #define _HMIDEBUG_LEVEL2_PRINT(x)
  #define _HMIDEBUG_LEVEL2_PRINT2(x,y)
#endif 

#ifdef _HMIDEBUG_LEVEL3
  #define _HMIDEBUG_LEVEL3_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_LEVEL3_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_LEVEL3_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_LEVEL3_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_LEVEL3_PRINTLN(x)
  #define _HMIDEBUG_LEVEL3_PRINTLN2(x,y)
  #define _HMIDEBUG_LEVEL3_PRINT(x)
  #define _HMIDEBUG_LEVEL3_PRINT2(x,y)
#endif
// simulates physical measurements
#ifdef _HMIDEBUG_SIMUL
  #define _HMIDEBUG_SIMUL_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_SIMUL_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_SIMUL_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_SIMUL_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_SIMUL_PRINTLN(x)
  #define _HMIDEBUG_SIMUL_PRINTLN2(x,y)
  #define _HMIDEBUG_SIMUL_PRINT(x)
  #define _HMIDEBUG_SIMUL_PRINT2(x,y)
#endif
// simulates physical measurements
#ifdef _HMIDEBUG_STATE_MACHINE
  #define _HMIDEBUG_SM_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_SM_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_SM_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_SM_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_SM_PRINTLN(x)
  #define _HMIDEBUG_SM_PRINTLN2(x,y)
  #define _HMIDEBUG_SM_PRINT(x)
  #define _HMIDEBUG_SM_PRINT2(x,y)
#endif

#ifdef _HMIDEBUG_CRITICAL
  #define _HMIDEBUG_CRITICAL_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_CRITICAL_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_CRITICAL_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_CRITICAL_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_CRITICAL_PRINTLN(x)
  #define _HMIDEBUG_CRITICAL_PRINTLN2(x,y)
  #define _HMIDEBUG_CRITICAL_PRINT(x)
  #define _HMIDEBUG_CRITICAL_PRINT2(x,y)
#endif

#ifdef _HMIDEBUG_ERROR
  #define _HMIDEBUG_ERROR_PRINTLN(x) Serial.println(x)
  #define _HMIDEBUG_ERROR_PRINTLN2(x,y) Serial.println(x,y)
  #define _HMIDEBUG_ERROR_PRINT(x) Serial.print(x)
  #define _HMIDEBUG_ERROR_PRINT2(x,y) Serial.print(x,y)
#else
  #define _HMIDEBUG_ERROR_PRINTLN(x)
  #define _HMIDEBUG_ERROR_PRINTLN2(x,y)
  #define _HMIDEBUG_ERROR_PRINT(x)
  #define _HMIDEBUG_ERROR_PRINT2(x,y)
#endif
#endif
