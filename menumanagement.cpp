/*
 * La Box Project
 * MenuManagement Class 
 * The purpose of this class is to organize the operation menu. 
 * This is where the actions or choices selected by the user are executed.
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "hmi.h"
#include "menumanagement.h"
#include "menuobject.h"
#include "menuinformation.h"
#include "menuTrainAddrRead.h"
#include "globals.h"

extern enumHMIState  _HMIState ;
extern enumHMIState  _HMIState_prev;
extern enumEvent     _HMIEvent;

/*!
    @brief  Constructor
    @param  None
    @return None (void).
    @note
*/
MenuManagement::MenuManagement(hmi*  screen)
{
  display           = (Adafruit_SSD1306*) screen;

  baseMenu      = new menuObject(display, NULL, TXT_MenuParams, MENUTYPELIST);
  onOffLine     = new menuObject(display, baseMenu, TXT_MenuDCCOffLine, MENUTYPELIST);
    onOffLineOn = new menuObject(display, onOffLine, TXT_MenuOnLine,   0);
    onOffLineOff= new menuObject(display, onOffLine, TXT_MenuOffLine,  1);
  trainAddrRead = new menuTrainAddrRead(display, baseMenu, TXT_MenuAddrRead,  MENUTRAINADDRREAD);
  TrainView     = new menuObject(display, baseMenu, TXT_TrainView, MENUTYPELIST);
    V1Train     = new menuObject(display, TrainView, TXT_V1Train,  1);
    V2Trains    = new menuObject(display, TrainView, TXT_V2Trains, 2);
    V3Trains    = new menuObject(display, TrainView, TXT_V3Trains, 3);
  physicalMes   = new menuInformation(display, (menuObject*)baseMenu, TXT_PhysicalMes, MENUACTION);    // Classe à définir
  lstEvent      = new menuObject(display, baseMenu, TXT_LstEvent, MENUACTION);
  language      = new menuObject(display, baseMenu, TXT_Language, MENUTYPELIST);
    languageFr  = new menuObject(display, language, "Francais", 0);
    languageEn  = new menuObject(display, language, "English",  1);
    languageSp  = new menuObject(display, language, "Espanol",  2);
    languageDe  = new menuObject(display, language, "Deutsch",  3);
  wifi          = new menuObject(display, baseMenu, TXT_MenuWifi,MENUTYPELIST);
    wifiInfo    = new menuInformation(display, (menuObject*)wifi, TXT_MenuWifiInfo, MENUACTION);    // Classe à définir
    wifiActivate= new menuObject(display, wifi, TXT_MenuActivWifi, MENUTYPELIST);
      wifionLine  = new menuObject(display, wifiActivate, TXT_MenuOnLine,   0);
      wifiOffLine = new menuObject(display, wifiActivate, TXT_MenuOffLine,  1);
/*     wifiDHCP    =  new menuObject(display, wifi, TXT_MenuDHCP, MENUTYPELIST);
      DHCPonLine  = new menuObject(display, wifiActivate, TXT_MenuYes,   0);
      DHCPOffLine = new menuObject(display, wifiActivate, TXT_MenuNo,  1);
    wifiDHCP    =  new menuObject(display, wifi, TXT_MenuDHCP, MENUTYPELIST);
      DHCPonLine  = new menuObject(display, wifiActivate, TXT_MenuYes,   0);
      DHCPOffLine = new menuObject(display, wifiActivate, TXT_MenuNo,  1); */
  can           = new menuObject(display, baseMenu, TXT_MenuCAN, MENUTYPELIST);
    can_gw      = new menuObject(display, can, TXT_MenuCAN_GW, MENUTYPELIST);
      can_gw_on = new menuObject(display, can_gw, TXT_MenuOn,     1);
      can_gw_off= new menuObject(display, can_gw, TXT_MenuOff,    2);
    canInfo     = new menuInformation(display,  can,TXT_MenuCANInfo, MENUACTION);
  infoSystem    = new menuInformation(display, baseMenu, TXT_MenuInfosSys, MENUACTION);
  reset         = new menuObject(display, baseMenu, TXT_MenuSoftReset, MENUTYPELIST);
    resetConfirm= new menuObject(display, reset, TXT_MenuResetConfirm, MENUACTION);
/*       resetYes  = new menuObject(display, resetConfirm, TXT_MenuYes,  1);
      resetNo   = new menuObject(display, resetConfirm, TXT_MenuNo,   2); */
  factoryReset  = new menuObject(display, baseMenu, TXT_MenuFacReset, MENUTYPELIST);
    factoryResetConfirm  = new menuObject(display, factoryReset, TXT_MenuFacResetConfirm, MENUACTION);
/*       FactoryYes= new menuObject(display, factoryResetConfirm, TXT_MenuYes,  1);
      FactoryNo = new menuObject(display, factoryResetConfirm, TXT_MenuNo,   2); */


// Todo, ajouter liste événéemnts
//Type de vue dashboard
/* Type de vue des trains
@IP de la box, masque de sous-réseau, passerelle
Mode AP ou client
Serveur DHCP ou adressage IP statique */

}
/*!
    @brief  begin
            i.e. Setup
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::begin()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::begin.. Begin"); 

  baseMenu->begin();    // Recursive call, all sub menu will execute this fonction

  resetMenu();
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::begin.. End");   
}
  
/*!
    @brief  update
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::update()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::update.. Begin"); 

  if(activeMenu == 0) activeMenu = baseMenu;
  
  activeMenu->update();

  _HMIDEBUG_FCT_PRINTLN("MenuManagement::update.. End");   
}
/*!
    @brief  BtnUpPressed
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::BtnUpPressed()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnUpPressed.. Begin"); 

  activeMenu->eventUp();
  // Return value is important to know witch action has been Selected


  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnUpPressed.. End"); 
}
/*!
    @brief  BtnDownPressed
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::BtnDownPressed()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnDownPressed.. Begin"); 

  activeMenu->eventDown();

  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnDownPressed.. End");   
}
/*!
    @brief  BtnSelectPressed
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::BtnSelectPressed()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnSelectPressed.. Begin"); 

  int status = activeMenu->eventSelect();
	
	if (hmi::progMode) 
	{
		return;
	}

  switch(status)
  {
    case MENUEXIT :
      _HMIDEBUG_LEVEL1_PRINT("Exit menu ");_HMIDEBUG_LEVEL1_PRINTLN(activeMenu->caption);
      if(activeMenu->parent)
      {
        activeMenu->resetMenu();
        activeMenu = activeMenu->parent;
      }else
      {
        _HMIState = StateExitMenu;
      }
    break;
    case MENUCHANGETOCHILD :
      _HMIDEBUG_LEVEL1_PRINT("Change menu from ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
      _HMIDEBUG_LEVEL1_PRINT(" to ");_HMIDEBUG_LEVEL1_PRINTLN(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
      activeMenu = activeMenu->subMenu[activeMenu->SelectListIndex];
      activeMenu->parent->resetMenu();

    break;
    case MENUTRAINADDRREAD :
      _HMIDEBUG_LEVEL1_PRINT("Change menu from ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
      _HMIDEBUG_LEVEL1_PRINT(" to SPECIAL menu ");_HMIDEBUG_LEVEL1_PRINTLN(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
      activeMenu = activeMenu->subMenu[activeMenu->SelectListIndex];
      activeMenu->start();

    break;
    case MENUCHOSEN :
      _HMIDEBUG_LEVEL1_PRINT("Menu choice ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);_HMIDEBUG_LEVEL1_PRINT(" has been made : ");
      _HMIDEBUG_LEVEL1_PRINTLN(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
      activeMenu = activeMenu->subMenu[activeMenu->SelectListIndex];
      if(activeMenu == language)
      {
        _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_CRITICAL_PRINT(activeMenu->caption);
        _HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_CRITICAL_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
        _HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_CRITICAL_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");

      }else
      {
        if(activeMenu == onOffLine)
        {
          _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
          _HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
          _HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");        
        }else
        {
          if(activeMenu == wifiActivate)
          {
            _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
            _HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
            _HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
          }else
          {
            if(activeMenu == resetConfirm)
            {
              _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
              _HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
              _HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
#ifdef ARDUINO_ARCH_ESP32
              // !! Only for ESP !!
              ESP.restart();
#endif
            }else
            {
              if(activeMenu == factoryResetConfirm)
              {
                _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
                _HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
                _HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
                // Restore default setting

#ifdef ARDUINO_ARCH_ESP32
                // !! Only for ESP !!
                ESP.restart();
#endif
              }else
              {
                if(activeMenu == lstEvent)
                {
                  _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
                  //_HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
                  //_HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
                  // Restore default setting
                  _HMIState = StateBrowseEventLst ;
                  baseMenu->resetMenu();
                }else
                {
                  if(activeMenu == V1Train)
                  {
                    _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
                    //_HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
                    //_HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
                    // Restore default setting
                    ((hmi*) display)->nbTrainToView = 1 ;
                    _HMIState = StateExitMenu;
                  }else
                  {
                    if(activeMenu == V2Trains)
                    {
                      _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
                      //_HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
                      //_HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
                      // Restore default setting
                      ((hmi*) display)->nbTrainToView = 2 ;
                      _HMIState = StateExitMenu;
                    }else
                    {
                      if(activeMenu == V3Trains)
                      {
                        _HMIDEBUG_LEVEL1_PRINT("Choice menu found for ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->caption);
                        //_HMIDEBUG_LEVEL1_PRINT(": Choice is ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->caption);
                        //_HMIDEBUG_LEVEL1_PRINT(" (value : ");_HMIDEBUG_LEVEL1_PRINT(activeMenu->subMenu[activeMenu->SelectListIndex]->value);_HMIDEBUG_LEVEL1_PRINTLN(")");
                        // Restore default setting
                        ((hmi*) display)->nbTrainToView = 3 ;
                        _HMIState = StateExitMenu;
                      }else
                      {
                        _HMIDEBUG_CRITICAL_PRINT("Critical error, no choice menu found for ");_HMIDEBUG_CRITICAL_PRINTLN(activeMenu->caption);
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      baseMenu->resetMenu();      
      activeMenu = activeMenu->parent;
      _HMIEvent = noEvent;
    break;
  }
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::BtnSelectPressed.. End");   
}
/*!
    @brief  resetMenu
    @param  None
    @return None (void).
    @note
*/
void MenuManagement::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::resetMenu.. Begin"); 
  _HMIDEBUG_LEVEL1_PRINTLN("Menu Reset");

  baseMenu->resetMenu();  // Recursive call, all sub menu will execute this fonction

  activeMenu = baseMenu;
  _HMIDEBUG_FCT_PRINTLN("MenuManagement::resetMenu.. End");   
}
#endif