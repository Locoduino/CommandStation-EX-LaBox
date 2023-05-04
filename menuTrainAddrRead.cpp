/*
 * La Box Project
 * menuTrainAddrRead Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuTrainAddrRead.h"
#include "globals.h"
#include "hmi.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

int locoID = 0;
bool displayInProgress;
char message[21];
void locoIdCallback(int16_t inId)
{
  locoID = inId;
  DIAG(F("locoIdCallback called %d !"), locoID);
  displayInProgress = false;
}

void menuTrainAddrRead::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::start.. Begin"); 

  if (!hmi::progMode) {
    if (EEPROM.readByte(hmi::EEPROMModeProgAddress) != 'P') {
      EEPROM.writeByte(hmi::EEPROMModeProgAddress, 'P');
      EEPROM.commit();
    }
    delay(500);
    ESP.restart();
  }

  locoID = 0;
  void (*ptr)(int16_t) = &locoIdCallback;
  DCC::getLocoId(ptr);
  displayInProgress = false;
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::start.. End"); 
}

/*!
    @brief  menuTrainAddrRead Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuTrainAddrRead::menuTrainAddrRead(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
  resetMenu();
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventUp.. Begin"); 
  menuObject::eventUp();

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventUp.. End");   
}
/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventDown.. Begin"); 
  menuObject::eventDown();

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuTrainAddrRead::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventSelect.. Begin"); 
  menuObject::eventSelect();

  // Return to 'Main' mode.
  if (EEPROM.readByte(hmi::EEPROMModeProgAddress) != 'M') {
    EEPROM.writeByte(hmi::EEPROMModeProgAddress, 'M');
    EEPROM.commit();
  }
  delay(500);
  // Reboot ESP32 !
  ESP.restart();

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventSelect.. End");  
  return MENUEXIT;
}
/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::update()
{
  _HMIDEBUG_FCT_PRINTLN(F("menuTrainAddrRead::update.. Begin")); 

  if(!displayInProgress)
  {
    display->clearDisplay();
    displayInProgress = true;
  }

  display->setTextSize(1);
  display->setCursor(5, 53);
  display->println(TXT_MenuAddrRead);
  display->display(); 
  if(locoID > 0)
  {
    sprintf(message,"%04d",locoID);
  }
  else
    sprintf(message,"----");

  display->setTextColor(WHITE);
  display->setTextSize(3);
  display->setCursor(20, 15);
  display->println(message);  

  display->display();   
  _HMIDEBUG_FCT_PRINTLN(F("menuTrainAddrRead::update.. End")); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::resetMenu.. Begin"); 
  menuObject::resetMenu();

  displayInProgress = false;

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::resetMenu.. End"); 
  
}
#endif