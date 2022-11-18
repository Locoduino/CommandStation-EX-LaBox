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
#include "menuspecial.h"
#include "globals.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
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

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventSelect.. End");  
  return 0;
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

int locoID = 0;
void locoId(int16_t inId)
{
  locoID = inId;
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::update.. Begin"); 

  if(!displayInProgress)
  {
    display->clearDisplay();
  }
  display->setTextSize(1);
  display->setCursor(5, 53);
  display->println(TXT_MenuAddrRead);
  display->display(); 
  if(!displayInProgress)
  {
    digitalWrite(PIN_LEDBUILTIN, true);
    displayInProgress = true;
    millisDisplay = millis();
    void (*ptr)(int16_t) = &locoId;
    DCC::getLocoId(ptr);
    digitalWrite(PIN_LEDBUILTIN, false);
  }
  if(locoID > 0)
    sprintf(message,"%4d",locoID);
  else
    sprintf(message,"ERROR");
  display->setTextColor(WHITE);
  display->setTextSize(3);
  display->setCursor(20, 15);
  display->println(message);  

  display->display();   
  // Waiting
  if(millis() - millisDisplay > 10000)
  {
      displayInProgress = false ;
      exitAsk = true;
  }

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::update.. End"); 
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

  millisDisplay = 0;
  displayInProgress = false ;

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::resetMenu.. End"); 
  
}
#endif