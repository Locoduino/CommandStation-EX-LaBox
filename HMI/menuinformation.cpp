/*
 * La Box Project
 * menuInformation Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuinformation.h"
#include "globals.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
/*!
    @brief  menuInformation Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuInformation::menuInformation(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{


}
/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuInformation::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventUp.. Begin"); 
  menuObject::eventUp();

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventUp.. End");   
}
/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuInformation::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventDown.. Begin"); 
  menuObject::eventDown();

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuInformation::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventSelect.. Begin"); 
  menuObject::eventSelect();

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventSelect.. End");  
  return 0;
}
/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuInformation::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuInformation::begin.. End"); 
}
/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuInformation::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::update.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuInformation::update.. End"); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuInformation::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::resetMenu.. Begin"); 
  menuObject::resetMenu();



  _HMIDEBUG_FCT_PRINTLN("menuInformation::resetMenu.. End"); 
  
}
#endif