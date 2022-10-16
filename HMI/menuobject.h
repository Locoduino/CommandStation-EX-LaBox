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

#ifndef MENUOBJECT
#define MENUOBJECT

#ifdef USE_HMI
#include "hmiConfig.h"

#define nbMaxSubmenu        15
class Adafruit_SSD1306;

class menuObject
{
  public:
    //----- Members
    menuObject*   parent;
    menuObject*   subMenu[nbMaxSubmenu];
    menuObject*   selectedMenu ;      
    int           nbSubMenu ;
    int           value ;
    bool          exitAsk;
    int           position ;
    char          caption[HMI_MenueMessageSize];
    int           firstListIndex;
    int           ListIndexMaxVisible;
    int           SelectListIndex; 
    //----- functions
    menuObject();
    menuObject(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);

    virtual void begin();
    virtual void update();
    virtual void addChild(menuObject* sub);    
    virtual void eventUp();
    virtual void eventDown();
    virtual int  eventSelect();
    virtual void resetMenu();

  protected:
    //----- Members
    Adafruit_SSD1306*        display;
    menuObject* endObject;
    
    //----- functions

};

#endif
#endif