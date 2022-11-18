/*
 * La Box Project
 * menuSetValue Classes 
 * 
 * !!!!!! not yet implemented !!!!!!
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menusetvalue.h"
#include "globals.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*!
    @brief  menuSetValue Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuSetValue::menuSetValue(Adafruit_SSD1306* screen, menuObject* p, const char* title, int _min, int _max, int _nbDigit): menuObject(screen, p, title,0)
{



}
/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuSetValue::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuSetValue::begin.. End"); 
}
/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuSetValue::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::update.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuSetValue::update.. End"); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuSetValue::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::resetMenu.. Begin"); 
  menuObject::resetMenu();



  _HMIDEBUG_FCT_PRINTLN("menuSetValue::resetMenu.. End"); 
}
/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuSetValue::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventUp.. Begin"); 
  menuObject::eventUp();

  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventUp.. End");   
}
/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuSetValue::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventDown.. Begin"); 
  menuObject::eventDown();

  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuSetValue::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventSelect.. Begin"); 
  menuObject::eventSelect();
  _HMIDEBUG_FCT_PRINTLN("menuSetValue::eventSelect.. End");  
  return 0;
}
#endif