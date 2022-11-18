/*
 * La Box Project
 * MenuManagement Class 
 * The purpose of this class is to organize the operation menu. 
 * This is where the actions or choices selected by the user are executed.
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef MENUMANAGEMENT
#define MENUMANAGEMENT

#ifdef USE_HMI
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "hmiConfig.h"

class hmi;
class menuObject;

class MenuManagement
{
  public:
    //----- Members

    
    //----- functions  
    MenuManagement(hmi*  screen);
    void begin();
    void update();
    void BtnUpPressed();
    void BtnDownPressed();
    void BtnSelectPressed();
    void resetMenu();
  protected:
    //----- Members
    menuObject* listMenu[50];
    menuObject* baseMenu;
    menuObject* trainAddrRead;
    menuObject* onOffLine;
    menuObject* onOffLineOn;
    menuObject* onOffLineOff;
    menuObject* wifi;
    menuObject* wifiInfo;
    menuObject* wifiActivate;
    menuObject* lstEvent;
    menuObject* can;
    menuObject* can_gw;
    menuObject* can_gw_on;
    menuObject* can_gw_off;
    menuObject* canSetAddr;
    menuObject* canInfo;
    menuObject* infoSystem;
    menuObject* factoryReset;
    menuObject* factoryResetConfirm;
    menuObject* reset;
    menuObject* resetConfirm;
    menuObject* language;
    menuObject* languageFr;
    menuObject* languageEn;
    menuObject* languageSp;
    menuObject* languageDe;
    menuObject* physicalMes;
    menuObject* TrainView;
    menuObject* V1Train;
    menuObject* V2Trains;
    menuObject* V3Trains;
    menuObject* wifionLine;
    menuObject* wifiOffLine; 
/*     menuObject* resetYes;
    menuObject* resetNo; 
    menuObject* FactoryYes;
    menuObject* FactoryNo; */
    int menuState;
    menuObject* activeMenu ;
    //menuObject* activeMenu_prev ;


    Adafruit_SSD1306*  display;
    byte  nbMenuItems;
    //----- functions
    //void addMenuInList(menuObject* item);
};
#endif

#endif
