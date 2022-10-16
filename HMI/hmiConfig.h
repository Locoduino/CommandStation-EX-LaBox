/*
 * La Box Project
 * Parameters
 * @Author : Cedric Bellec
 *	@Organization : Locoduino.org
 */

#ifndef HMICONFIG_H
#define HMICONFIG_H

#ifdef USE_HMI
//---------------------- HMI section ------------------------------------
#define HMI_I2C_ADDR            0x3C
#define SCREEN_WIDTH            128         // OLED display width, in pixels
#define SCREEN_HEIGHT           64          // OLED display height, in pixels
#define HMI_Rotation            2           // 1 : 90°, 2 : 180°, 3 : 270°

// Pins
#define PIN_LEDBUILTIN           2
#define PIN_BTN_SEL             18
#define PIN_BTN_BTNUP           23
#define PIN_BTN_BTNDWN          19
#define PIN_CURRENT_MES         36
#define PIN_VOLTAGE_MES         34
// U/I correctors
#define HMI_VoltageK            0.0068      // Voltage scaling coefficient
#define HMI_CurrentK            0.9        	// Current scaling coefficient
#define HMI_deltaCurrent		100			// Current value shift with I=0
// Stack / string allocation sizes
#define HMI_StackNbCarElt       20          // 10 elements into the stack
#define HMI_EventMaxToDisplay   5           // We can display in full screnn 7 lines of event
#define HMI_MessageSize         20          // 12 characters for on message // Voir si il faut maintenir
#define HMI_MenueMessageSize    20          // 18 characters for un caption in menu
#define LineCarNbMax            20          // 1 ligne with FreeSerif9pt7b police, you can show 20 caracters max.
#define HMI_NbMemorisedTrain    10          // 
#define nbCycleToWaitWifi       50        
//--- Orders
#define HMI_OrderForward        20      
#define HMI_OrderBack           21
#define HMI_OrderStop           22 
#define HMI_OrderFunction       23
#define HMI_OrderStopAll        24  
#define HMI_StartDCC            25
#define HMI_ShortCurcuit        26
#define HMI_WifiWaiting         27
#define HMI_noWifi              28
#define HMI_WifiOk              29
//--- Display effects
#define HMI_LowEffect           500
#define HMI_FastEffect          100
#define HMI_DataRefesh          1000
#define HMI_DisplayRefesh       100
#define HMI_TimeOutMenu         30000     // in milliseconds
//--- HMITrain
#define HMITrain_NbMaxFunctions 28
#define HMITrain_NbStep         128
#define HMITrain_RectHeight     5
#define HMITrain_RectWidth      8
#define HMITrain_RectSpace      1
//--- Menu
#define HMI_Menu_OnLine         201
#define HMI_Menu_OffLine        202
#define HMI_Menu_WifiOn         203
#define HMI_Menu_WifiOff        204
#define NbMaxLineVisible        5

#define MENUNOTINIT             300
#define MENUEXIT                301
#define MENUCHANGETOCHILD       302
#define MENUCHOSEN              303
#define MENUSPECIAL             304
#define MENUTYPECOMEBCK         0x8000
#define MENUTYPELIST            0x7999
#define MENUACTION              0x7998
//#define MENUTYPEITEM            312


    // Message by language : French
#define TXT_Forward             "AV"
#define TXT_Back                "AR"
#define TXT_Forwardl            "Avance"
#define TXT_Backl               "Recule"
#define TXT_Stop                "Stop"
#define TXT_Function            "F"
#define TXT_StopAll             "<ARRET DCC>"
#define TXT_StartDCC            "<DCC On>"
#define TXT_ShortCircuit        "COURT-CIRCUIT"
#define TXT_MenuDCCOffLine      "Stop DCC" // Les ? ne passent pas
#define TXT_MenuOnLine          "On line"
#define TXT_MenuOffLine         "Off line"
#define TXT_MenuActivWifi       "Activation Wifi"
#define TXT_MenuDHCP            "DHCP"
#define TXT_MenuResetConfirm    "Confirmation Reset"
#define TXT_MenuFacResetConfirm "Factory Reset?"
#define TXT_MenuYes             "Oui"
#define TXT_MenuNo              "Non"
#define TXT_MenuParams          "Parametres"
#define TXT_MenuWifi            "Wifi"
#define TXT_MenuWifiInfo        "Infos WiFi"
#define TXT_MenuCANInfo         "Infos CAN"
#define TXT_MenuCAN             "Reseaux CAN"
#define TXT_MenuCAN_GW          "Passerelle CAN/Wifi"
#define TXT_MenuOn              "On"
#define TXT_MenuOff             "Off"
#define TXT_MenuCANAddr         "Adresse CAN"
#define TXT_MenuInfosSys        "Info Systeme"
#define TXT_MenuSoftReset       "Redemarrage"
#define TXT_MenuFacReset        "Factory Reset"
#define TXT_MenuBack            "< Retour >"
#define TXT_MenuAddrRead        "Lecture addr Train"
#define TXT_PhysicalMes         "Mesures U et I"
#define TXT_Language            "Langue"
#define TXT_TrainView           "Type de vue trains"
#define TXT_V1Train             "1 train"
#define TXT_V2Trains            "2 trains"
#define TXT_V3Trains            "3 trains"
#define TXT_LstEvent            "Liste evenements"
#define TXT_WifiWaiting         "Connexion WiFi..."
#define TXT_noWifi              " !! No WiFi !!"
#define TXT_WifiOk              "...Wifi ok..."
#define TXT_BoxAddr             "Box address :"
//#define TXT_LanguageRequest    "Choix de la Langue"

    // Message by language : English
//#define TXT_Forward             "FW"
//#define TXT_Back                "BK"
//#define TXT_Forwardl            "Forward"
//#define TXT_Backl               "Back"
//#define TXT_Stop                "Stop"
//#define TXT_Function            "F"
//#define TXT_StopAll             "!STOP ALL!"
//#define TXT_StartDCC            "<DCC On>"
//#define TXT_ShortCircuit        "SHORT-CIRCUIT"
//#define TXT_MenuOnOffLine       "On / Off line ?"
//#define TXT_MenuOnLine          "On line"
//#define TXT_MenuOffLine         "Off line"
//#define TXT_MenuActivWifi       "Wifi Activation"
//#define TXT_MenuResetConfirm    "Reset confirmation ?"
//#define TXT_MenuFacResetConfirm "Factory Reset?"
//#define TXT_MenuYes             "Yes"
//#define TXT_MenuNo              "No"


//---------------------- Global section ---------------------------------
#define Labox_StateDCCOFF        101
#define Labox_StateDCCON         102
#define Labox_StateSHORTCIRCUIT  103


#endif
#endif