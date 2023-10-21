/*
 * La Box Project
 * HMI Class 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef _HMI
#define _HMI

#ifdef USE_HMI
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "OneButton.h"
#include "Wire.h"
#include "hmiConfig.h"
#include "HmiInterface.hpp"

#include "hmiTrain.h"
#include "globals.h"

class MenuManagement;

class hmi : public Adafruit_SSD1306, public HmiInterface
{
  public:
    //----- Members
    hmi(TwoWire *twi);
    void begin();
    void setProgMode();
    void update();
    byte laBoxState;
    MenuManagement *menu;
    float voltage;
    float current;
    int   nbTrainToView;
		// True if the ESP is in programmation mode.
    static bool progMode;
		// Ture if the boot should not show logos and startup messages on the screen.
    static bool silentBootMode;
// Position of the byte 'M' or 'P' for Main or Prog track mode.
    static int EEPROMModeProgAddress;

    //int executionCore;
    
    //----- functions
    void addNotification(int addr, uint8_t order, uint8_t value, bool functionState=false);
    void addNotification(uint8_t order);
    void addNotification(const char* msg);
    void readVoltage();
    void readCurrent();

  protected:
    //----- Members
    char messageStack[HMI_StackNbCarElt][HMI_MessageSize];
    char message[LineCarNbMax];
    unsigned long millisEffect;
    unsigned long millisRefreshData;
    unsigned long millisParamsMenu; 
    unsigned long millisRefreshDisplay; 
    unsigned long millisWifiEffect;
    byte          positioneventlst;               // index in event list
    byte          wifiCircle ;                    // Juste for visual effect
    OneButton *BtnUp;
    OneButton *BtnDown;
    OneButton *BtnSelect;
    hmiTrain  tabTrains[HMI_NbMemorisedTrain];
    bool  needToRefreshDisplay();
  
    //----- functions
    void stateMachine();
    void pushMessageOnStack(const char *msg, uint8_t len);
    void setTrainState(int addr, uint8_t order, uint8_t value, bool state=false);
    void dashboard();
    void showWifiWaiting();
    void dashboard1TrainView();
    void dashboard2TrainsView();
    void dashboard3TrainsView();
    void ParametersMenu();
    //void MenuSelect();
    void BrowseEventLst();
    static void BtnUpPressed();
    static void BtnDownPressed();
    static void BtnSelectPressed();
    
    // HmiInterface

    bool HmiInterfaceLoop();
    void HmiInterfaceUpdateDrawing();
};
#endif
#endif
