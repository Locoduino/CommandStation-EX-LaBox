/*
 * La Box Project
 * menu Classes 
 *
 * Base class of the menu library. 
 * It allows recursively to select entries from lists and to chain menus. 
 * The up and down button allows you to scroll through the choices and the select button validates the choice.
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#include "DCCEX.h"

#ifdef USE_HMI
#include "globals.h"
#include "menuobject.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/*!
    @brief  menuObject Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuObject::menuObject()
{
  parent = NULL;
}
menuObject::menuObject(Adafruit_SSD1306* screen, menuObject* p, const char* title, int val)
{
  parent = p;
  display = screen;
  selectedMenu  = NULL ;        
  nbSubMenu     = 0;
  value         = val;
  position      = 0;
  memset(caption, 0, HMI_MenueMessageSize);

  if(title)
  {
    memcpy(caption, title, HMI_MenueMessageSize - 1);
  }
  if(parent)
  {
    _HMIDEBUG_LEVEL1_PRINT("menuObject: Add child to my parent : ");_HMIDEBUG_LEVEL1_PRINT(parent->caption);_HMIDEBUG_LEVEL1_PRINT("/");_HMIDEBUG_LEVEL1_PRINTLN(caption);
    parent->addChild( (menuObject*) this);
  }
}
/*!
    @brief  addChild, add a child to this menu object
    @param  sub, a menuObject 
    @return None (void).
    @note
*/
void menuObject::addChild(menuObject* sub)
{
    if(sub)
    {
      if(nbSubMenu < nbMaxSubmenu)
      {
        subMenu[nbSubMenu++] = sub;
      }else
      {
        _HMIDEBUG_CRITICAL_PRINT("Critical error, you want add to much sub menus ! Menu  ");_HMIDEBUG_CRITICAL_PRINT(caption);
        _HMIDEBUG_CRITICAL_PRINT(", sub menu ");_HMIDEBUG_CRITICAL_PRINTLN(sub->caption);        
      }
    }
}
/*!
    @brief  begin
    @param  None
    @return None (void).
    @note
*/
void menuObject::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuObject::begin.. Begin"); 
  resetMenu();
  // Recursive call
  for(int i=0; i < nbSubMenu; i++)
  {
    if(subMenu[i])
    {
      subMenu[i]->begin();
    }
  }
  // Add endobject <come back> for lists 
  if(value == MENUTYPELIST)
  { 
    endObject = new menuObject(display, this, TXT_MenuBack, MENUTYPECOMEBCK) ;             
  }
  _HMIDEBUG_FCT_PRINTLN("menuObject::begin.. End"); 
}
/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuObject::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuObject::update.. Begin"); 
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(WHITE);
  display->setCursor(5, 53);
  display->println(caption);
  display->drawFastHLine(0,48,SCREEN_WIDTH, WHITE);
  
  if(SelectListIndex < NbMaxLineVisible ) 
  {
    firstListIndex = 0 ;
  }else
  {
    firstListIndex = (SelectListIndex - NbMaxLineVisible) +1; 
  }

  if(nbSubMenu > NbMaxLineVisible)
  {
    ListIndexMaxVisible = firstListIndex + NbMaxLineVisible; 
  }else
  {
      ListIndexMaxVisible = nbSubMenu;
  }
  //_HMIDEBUG_LEVEL1_PRINT("firstListIndex : ");_HMIDEBUG_LEVEL1_PRINT(firstListIndex);_HMIDEBUG_LEVEL1_PRINT(", ListIndexMaxVisible : ");_HMIDEBUG_LEVEL1_PRINT(ListIndexMaxVisible);
  //_HMIDEBUG_LEVEL1_PRINT(", SelectListIndex : ");_HMIDEBUG_LEVEL1_PRINT(SelectListIndex);_HMIDEBUG_LEVEL1_PRINT(", nbSubMenu : ");_HMIDEBUG_LEVEL1_PRINTLN(nbSubMenu);
  for(int i=firstListIndex; i < ListIndexMaxVisible; i++) 
  {
    if(subMenu[i])
    {
      if(subMenu[i] == selectedMenu) // selectedMenu
      {
        display->fillRect(0, (i-firstListIndex) * 10, SCREEN_WIDTH, 9 , WHITE);
        display->setCursor(5, 1 + (i-firstListIndex) * 10);
        display->setTextColor(BLACK);
        display->println(subMenu[i]->caption);
        display->setTextColor(WHITE);
      }else
      {
        display->setCursor(5, 0 + (i-firstListIndex) * 10);
        display->println(subMenu[i]->caption);
      }
    }
  }
  display->display();

  _HMIDEBUG_FCT_PRINTLN("menuObject::update.. End"); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuObject::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuObject::resetMenu.. Begin"); 
  // Recursive call
  for(int i=0; i < nbSubMenu; i++)
  {
    if(subMenu[i])
    {
      subMenu[i]->resetMenu();
    }
  }
  selectedMenu            = NULL ;        
  position                = 0 ;
  firstListIndex          = 0 ;  
  ListIndexMaxVisible     = 0 ;  
  SelectListIndex         = 0 ;  
  ListIndexMaxVisible     = 5;  // 5 entries max
  selectedMenu            = subMenu[firstListIndex];
  SelectListIndex = 0 ;

  _HMIDEBUG_FCT_PRINTLN("menuObject::resetMenu.. End"); 
}
/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuObject::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuObject::eventUp.. Begin"); 
  if(SelectListIndex > 0) SelectListIndex--;
  selectedMenu = subMenu[SelectListIndex];

  _HMIDEBUG_FCT_PRINTLN("menuObject::eventUp.. End");   
}
/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuObject::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuObject::eventDown.. Begin"); 
  if(SelectListIndex < nbSubMenu-1) SelectListIndex++;
  selectedMenu = subMenu[SelectListIndex];

  _HMIDEBUG_FCT_PRINTLN("menuObject::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuObject::eventSelect()
{
	_HMIDEBUG_FCT_PRINTLN("menuObject::eventSelect.. Begin"); 

	if (selectedMenu != NULL)
	{
		switch(selectedMenu->value)
		{
			case MENUTYPECOMEBCK:        // User has selected come back
				return MENUEXIT;
				break;
			case MENUTYPELIST:           // User has selected a sub menu
				return MENUCHANGETOCHILD;
				break;
			case MENUTRAINADDRREAD:           // User has selected a sub menu
				return MENUTRAINADDRREAD;
				break;
			case MENUTRAINCVREAD:           // User has selected a sub menu
				return MENUTRAINCVREAD;
				break;
			case MENUTRAINCVWRITE:           // User has selected a sub menu
				return MENUTRAINCVWRITE;
				break;
			default:
				return MENUCHOSEN;         // It's user selection like Yes or No for exemple
		}
	}

	_HMIDEBUG_FCT_PRINTLN("menuObject::eventSelect.. End"); 
	return 0;
}
/*!
    @brief  Thing to do before opening the option
    @param  None
    @return None (void).
    @note
*/
void menuObject::start()
{

}
#endif