/*
 * La Box Project
 * HMI Class 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"
#include "TrackManager.h"

#ifdef USE_HMI

#include "Wire.h"

#include "hmi.h"
#include "LaboxModes.h"
#include "icons.h"
#include "menumanagement.h"
#include "hmiGlobals.h"
#include "menuobject.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <driver/adc.h>
#include <soc/sens_reg.h>
#include <soc/sens_struct.h>
#undef ADC_INPUT_MAX_VALUE
#define ADC_INPUT_MAX_VALUE 4095 // 12 bit ADC
#define pinToADC1Channel(X) (adc1_channel_t)(((X) > 35) ? (X)-36 : (X)-28)
#endif

int IRAM_ATTR local_adc1_get_raw(int channel);

#ifdef USE_HMI
 // variables must be global due to static methods
enumEvent     _HMIEvent;
enumHMIState  _HMIState;

hmi::hmi(TwoWire *twi) : Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT) // ,twi, -1) !
{
  memset(messageStack, 0, HMI_MessageSize * HMI_StackNbCarElt);
  laBoxState      = Labox_StateDCCOFF;
  _HMIState        = StateDashboard;
  positioneventlst= 0;
	stopStateMachine = false;

  // Init train list
  for(int i=0; i < HMI_NbMemorisedTrain; i++)
  {
    tabTrains[i].setHMIpointer(this);
    tabTrains[i].setInfo(0,0,0);
  }
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void hmi::begin()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::begin().. Begin");

  if (CurrentInterface == NULL) CurrentInterface = this;

/*#ifdef ARDUINO_ARCH_ESP32
  this->executionCore = xPortGetCoreID();
#endif*/

  BtnUp = new OneButton(PIN_BTN_BTNUP, true);
  BtnDown = new OneButton(PIN_BTN_BTNDWN, true);
  BtnSelect = new OneButton(PIN_BTN_SEL, true);
  
  BtnUp->attachClick(this->BtnUpPressed);
  BtnDown->attachClick(this->BtnDownPressed);
  BtnSelect->attachClick(this->BtnSelectPressed);
  _HMIEvent = noEvent;
  // Init Display
  if (!Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, HMI_I2C_ADDR)) {
    _HMIDEBUG_FCT_PRINTLN("SSD1306 allocation failed");
  }
  //display.setFont(&FreeSerif9pt7b);       // !!!!!!!!! A vérifier
  clearDisplay();
  setRotation(HMI_Rotation);
  setTextSize(1);
  setTextColor(WHITE);

  if (!LaboxModes::progMode && !LaboxModes::silentBootMode) {
    drawBitmap(0, 0, locoduino_Splash128x44, 128, 44, WHITE);
    setCursor(0, 48);
    println("LaBox | Locoduino.org");
    display();
    delay(2000);
  }

  millisEffect          = millis();
  millisRefreshData     = millisEffect;
  millisRefreshDisplay  = millisEffect;
  millisWifiEffect      = millisEffect;
  nbTrainToView = 1;
  wifiCircle = 0 ;
  // Make menu
  menu = new MenuManagement(this);
  menu->begin();

  _HMIDEBUG_FCT_PRINTLN("hmi::begin().. End");
}

/*!
    @brief  Set prog Mode 
    @param  None
    @return None (void).
    @note
*/
/*void hmi::setProgMode()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::setProgMode().. Begin");

  LaboxModes::progMode = true;
  _HMIState = StateParametersMenu;
	if (LaboxModes::progModeType == ProgType::CV1ADDR)
	{
  	menu->setMenu(menu->trainAddrRead);
  	menu->trainAddrRead->start();
	}
	if (LaboxModes::progModeType == ProgType::CVREAD)
	{
  	menu->setMenu(menu->trainCVRead);
  	menu->trainCVRead->start();
	}
	if (LaboxModes::progModeType == ProgType::CVWRITE)
	{
  	menu->setMenu(menu->trainCVWrite);
  	menu->trainCVWrite->start();
	}

	if (LaboxModes::progModeType == ProgType::IDENTIFY)
	{
  	menu->setMenu(menu->trainIdent);
  	menu->trainIdent->start();
	}

  _HMIDEBUG_FCT_PRINTLN("hmi::setProgMode().. End");
}*/

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void hmi::update()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (CurrentInterface == NULL || core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::update().. Begin");

  HmiInterfaceLoop();

  // Update button state
  BtnUp->tick();
  BtnDown->tick();
  BtnSelect->tick();
    
  if (millis() - millisRefreshData > HMI_DataRefesh)
  {
    readVoltage();
    readCurrent();
    millisRefreshData = millis();
  }

  // Evolve the main state machine
  stateMachine();

  _HMIDEBUG_FCT_PRINTLN("hmi::update().. End");
}

/*!
    @brief  stateMachine
            Evolve the main state machine
    @param  None
    @return None (void).
    @note : 
*/
void hmi::stateMachine()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  if (this->stopStateMachine || DCCACK::isActive())
    return;

  _HMIDEBUG_FCT_PRINTLN("hmi::stateMachine().. Begin");
  // Time out in menu, back to dashboard
  if (!LaboxModes::progMode && _HMIState == StateParametersMenu && (millis() - millisParamsMenu > HMI_TimeOutMenu) )
  {
    _HMIDEBUG_SM_PRINTLN("Timeout Parameters Menu");
    _HMIState = StateExitMenu;
  }
// ---------------- Main State Diagramm - https://plantuml.com/fr/ -------------------------
//  @startuml
//    [*] -down-> StateDashboard
//    StateDashboard--> StateParametersMenu : Select Btn
//    StateDashboard --> StateBrowseEventLst : Up or Down Btn
//    StateBrowseEventLst --> StateDashboard : Select Btn\nor Time Out
//    StateBrowseEventLst --> StateBrowseEventLst : Up or Down Btn\n change viewed train
//    StateDashboardTrainView: Trains ready to run
//    StateDashboard -left-> StateDashboardTrainView : DCC On and \n no short-circuit
//    StateDashboardTrainView--> StateDashboard: Short-circuit or\n DCC Off
//    StateDashboardTrainView--> StateDashboardTrainView: Up or Down Btn\n change viewed train
//    StateDashboardTrainView: Display 1 or 3 trains
//    StateDashboardTrainView: Display train graph bars speed, \ndirection, functions
//    StateDashboardTrainView --> StateParametersMenu : Select Btn
//    StateParametersMenu --> StateParametersMenu  : Up or Down Btn\n to select item
//    StateParametersMenu --> StateDashboard: Time Out or\n exit item
//  @enduml
  //------------------------------- Event management ------------------------------------
  _HMIDEBUG_SM_PRINT("_HMIEvent : ");_HMIDEBUG_SM_PRINTLN(_HMIEvent);
  switch(_HMIEvent)
  {
    // <<<<<<<<<<<< Up button pressed >>>>>>>>>>>>>>>>
    case eventUp :
    // <<<<<<<<<< Down button pressed >>>>>>>>>>>>>>>>
    case eventDown :
      millisParamsMenu  = millis();
      switch(_HMIState)
      {
        case StateDashboard :      
          _HMIState = StateBrowseEventLst;
          _HMIEvent = noEvent ;
        break;
        case StateParametersMenu :      // We let the menu management process the event
        break;
        case StateDashboardTrainView :  // We let the dashboardTrainView function change train from the list
        break;
        case StateBrowseEventLst :      // We let the BrowseEventLst function generate the up and down of the list 
        break;
        default:;
      }
    break;
    // <<<<<<<<<< Select button pressed >>>>>>>>>>>>>>>>
    case eventSel :
      millisParamsMenu  = millis();
      switch(_HMIState)
      {
        case StateDashboard : 
          _HMIState = StateParametersMenu;
          _HMIEvent = noEvent ;     
        break;
        case StateParametersMenu :      // We let the menu management process the event
        break;
        case StateDashboardTrainView :  // We let the dashboardTrainView function change train from the list
          _HMIState = StateParametersMenu;
          _HMIEvent = noEvent ;    
        break;
        case StateBrowseEventLst :      // We let the BrowseEventLst function generate the up and down of the list
          _HMIState = StateDashboard;
          _HMIEvent = noEvent ; 
        break;
        case StateWifiWaiting :         // nothing to do because we are blocked in Wifi Connection While
        break;
        case StateNoWifi :
          _HMIState = StateParametersMenu;
          _HMIEvent = noEvent ;
        break;
        default:;
      }
    break;
    default:;
  }
  _HMIDEBUG_SM_PRINT("_HMIState : ");_HMIDEBUG_SM_PRINTLN(_HMIState);
  //----------- Action to be executed according to the state machine -----------------
  switch (_HMIState)
  {
    case StateDashboard :
        dashboard();
    break;
    case StateDashboardTrainView :
        if(needToRefreshDisplay() )
        {
          switch(nbTrainToView)   // A corriger selon les paramètres
          {
            case 1 : dashboard1TrainView();
            break;
            case 2 : dashboard2TrainsView();
            break;
            case 3 : dashboard3TrainsView();
            break;
            default : dashboard3TrainsView();
          }
        }
    break;
    case StateParametersMenu :
        //if(HMIState_prev != _HMIState) menu->resetMenu();
        ParametersMenu();
    break;
    case StateBrowseEventLst :
        BrowseEventLst();
    break;
    case StateExitMenu :
        menu->resetMenu();
        switch (laBoxState)
        {
          case Labox_StateDCCOFF  :       
          case Labox_StateSHORTCIRCUIT :  _HMIState = StateDashboard ;
            break;    
          case Labox_StateDCCON   :       
            if(tabTrains[0].addr > 0)
              _HMIState = StateDashboardTrainView ;
            else
              _HMIState = StateDashboard ;
            break;
        }
    break;
    case StateWifiWaiting :
        showWifiWaiting();
    break;
    case StateNoWifi :
        BrowseEventLst();
    break;
    default:;
  } 
  _HMIDEBUG_FCT_PRINTLN("hmi::stateMachine().. End");
}

/*!
    @brief  dashboard
            Show dashboard on display
    @param  None
    @return None (void).
    @note
*/
void hmi::dashboard()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::Dashboard().. Begin");
  static int toggleEffect = 0;
  int effect = 0;

/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  // --------------- Icons blinking effect -----------------------
  switch (laBoxState)
  {
    case Labox_StateDCCOFF  :       toggleEffect = 1; effect = 0;
      break;
    case Labox_StateDCCON   :       effect = HMI_LowEffect;
      break;
    case Labox_StateSHORTCIRCUIT :  effect = HMI_FastEffect;
      break;
    default:;
  }
  if (effect && millis() - millisEffect > (unsigned long) effect)
  {
    toggleEffect = !toggleEffect;
    millisEffect = millis();
  }
  // -------------------------------------------------------------

  clearDisplay();
  setTextSize(1);
  setTextColor(WHITE);
  setCursor(0, 0);
  println("La Box  Locoduino.org");
  drawFastHLine(0,10,128, WHITE);

  setCursor(0, 48);
#if defined(HMI_SHOW_CURRENT)
  sprintf(message, "U=%.0fV  |  I=%.0fmA", voltage, current);
#else
  sprintf(message, "U=%.0fV", voltage);
#endif
  println(message);

  //--- Message stack, only 4 last events
  for (int i = 0; i < 4; i++)
  {
    setCursor(0, 12 + i * 9);
    memset(message, 0, HMI_MessageSize);
    memcpy(message, messageStack[i], 14);
    println(message);
  }

  _HMIDEBUG_FCT_PRINTLN("hmi::Dashboard().. toggleEffect");
  if (toggleEffect)
  {
    switch (laBoxState)
    {
      case Labox_StateDCCOFF :
        drawBitmap(90, 13, Power, 35, 35, WHITE); // A modifier, ce n'est pas le bon icone
        break;
      case Labox_StateDCCON:
        drawBitmap(90, 13, Ok_White, 35, 35, WHITE);
        break;
      case Labox_StateSHORTCIRCUIT :
        drawBitmap(90, 13, ElecBlack, 35, 35, WHITE);
        break;
        //default :
        // TODO
      default:;
    }
  }
  display();
  _HMIDEBUG_FCT_PRINTLN("hmi::Dashboard().. End");
}
/*!
    @brief  dashboard3TrainsView
            Show dashboard whith 3 trains culumns 
    @param  None
    @return None (void).
    @note
*/
void hmi::dashboard3TrainsView()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::dashboard3TrainsView().. Begin");
    clearDisplay();
    setTextSize(1);
    // Draw tab with 3 columns
    drawFastHLine(0,48,128, WHITE);
    drawFastVLine(42, 0, 64, WHITE);
    drawFastVLine(85, 0, 64, WHITE);

    setCursor(0, 0);
    // ...For 3 columns
    for(int i=0; i<3; i++)
    {
      if(&tabTrains[i] && tabTrains[i].addr)
      {
        tabTrains[i].dashboard((i*43)+2, 0, (i*43)+38, 64);    
      }
    }
    // Quand on utilisera les menu !!!!
    _HMIEvent = noEvent;
    
    display();
    _HMIDEBUG_FCT_PRINTLN("hmi::dashboard3TrainsView().. End");
}
/*!
    @brief  dashboard2TrainsView
            Show dashboard whith 2 trains culumns 
    @param  None
    @return None (void).
    @note
*/
void hmi::dashboard2TrainsView()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::dashboard2TrainsView().. Begin");
    clearDisplay();

    // Draw tab with 3 columns
    drawFastHLine(0,48,128, WHITE);
    drawFastVLine(64, 0, 64, WHITE);
 
    setCursor(0, 0);
    // ...For 2 columns
    for(int i=0; i<2; i++)
    {
      if(&tabTrains[i] && tabTrains[i].addr)
      {
        tabTrains[i].dashboard((i*43)+2, 0, (i*43)+38, 64);    
      }
    }
    // Quand on utilisera les menu !!!!
    _HMIEvent = noEvent;
    
    display();
    _HMIDEBUG_FCT_PRINTLN("hmi::dashboard2TrainsView().. End");
}
/*!
    @brief  dashboard1TrainView
            Show dashboard whith only 1 train
    @param  None
    @return None (void).
    @note
*/
void hmi::dashboard1TrainView()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::dashboard1TrainView().. Begin");
    if(tabTrains[0].addr > 0)
      {        
        clearDisplay();
        tabTrains[0].dashboard1T(); 
        display();   
      }
    _HMIEvent = noEvent;
    
    _HMIDEBUG_FCT_PRINTLN("hmi::dashboard1TrainView().. End");
}
/*!
    @brief  ParametersMenu
            Display and manage parameter menu
    @param  None
    @return None (void).
    @note
*/
void hmi::ParametersMenu()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::ParametersMenu().. Begin");
  
  // Transmission of events to menus
  switch(_HMIEvent)
  {
    case eventUp    :  menu->BtnUpPressed();
    break;
    case eventDown  :  menu->BtnDownPressed();
    break;
    case eventSel   :  menu->BtnSelectPressed();
    break;
    case noEvent :     if (!needToRefreshDisplay() )  return;
    break;
    default:;
  }
  // If an event has been processed, we must delete it.
  _HMIEvent = noEvent;
  menu->update();
  _HMIDEBUG_FCT_PRINTLN("hmi::ParametersMenu().. End");
}
/*!
    @brief  BrowseEventLst
            Display an interface to browse event list
    @param  None
    @return None (void).
    @note
*/
void hmi::BrowseEventLst()
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::BrowseEventLst().. Begin");
  static int firtIdToShow  = 0;
  // Transmission of events to menus
  switch(_HMIEvent)
  {
    case eventUp    :  if( (HMI_StackNbCarElt-HMI_EventMaxToDisplay) > firtIdToShow) firtIdToShow++;
    break;
    case eventDown  :  if(firtIdToShow>0) firtIdToShow++;
    break;
    case noEvent :     if (!needToRefreshDisplay() )  return;
    break;
    default:;
  }
  // If an event has been processed, we must delete it.
  _HMIEvent = noEvent;
    
  clearDisplay();
  setTextSize(1);
  setCursor(5, 53);
  println(TXT_LstEvent);
  setTextColor(WHITE);
  //--- Message stack, only 5 last events
  for (int i = firtIdToShow ; i < HMI_EventMaxToDisplay; i++)
  {
    setCursor(0, 0 + i * 9);
    memset(message, 0, HMI_MessageSize);
    memcpy(message, messageStack[i], HMI_MessageSize - 1);
    println(message);
  }
  display();
  _HMIDEBUG_FCT_PRINTLN("hmi::BrowseEventLst().. End");
}
/*!
    @brief  addMessage, add a message to the HMI stack.
            This function must be called each time a command has been sent to the DCC network
    @param  order : Kind of order sent to dcc network. It's only for function without parameters
    @return None (void).
    @note
*/
void hmi::addNotification(uint8_t order)
{
  addNotification(0, order, 0);
}
/*!
    @brief  addMessage, add a message to the HMI stack.
            This function must be called each time a command has been sent to the DCC network
    @param  addr : engine address
    @param  order : Kind of order sent to dcc network
    @param  value : order value (0->255)
    @return None (void).
    @note
*/
void hmi::addNotification(int addr, uint8_t order, uint8_t value, bool functionState)
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::addNotification().. Begin");
  _HMIDEBUG_PARAMS_PRINT("hmi::addNotification, addr :  ");_HMIDEBUG_PARAMS_PRINT(addr); _HMIDEBUG_PARAMS_PRINT(", order : ");_HMIDEBUG_PARAMS_PRINT(order);_HMIDEBUG_PARAMS_PRINT(", value : ");_HMIDEBUG_PARAMS_PRINTLN(value);
  memset(message, 0, LineCarNbMax);

  switch (order)
  {
    case HMI_OrderForward :
      if (laBoxState == Labox_StateDCCON) {
        sprintf(message, "@%04d-%s%03d", addr, TXT_Forward, value);
        pushMessageOnStack(message, (uint8_t) strlen(message));
        setTrainState(addr, order, value);
      }
      break;
    case HMI_OrderBack :
      if (laBoxState == Labox_StateDCCON) {
        sprintf(message, "@%04d-%s%03d", addr, TXT_Back, value);
        pushMessageOnStack(message, (uint8_t) strlen(message));
        setTrainState(addr, order, value);
      }
      break;
    case HMI_OrderStop :
      if (laBoxState == Labox_StateDCCON) {
        sprintf(message, "@%04d-%s", addr, TXT_Stop);
        pushMessageOnStack(message, (uint8_t) strlen(message));
        setTrainState(addr, order, value);
      }
      break;
    case HMI_OrderFunction :
      if (laBoxState == Labox_StateDCCON) {
        sprintf(message, "@%04d-%s%02d", addr, TXT_Function, value);
        pushMessageOnStack(message, (uint8_t) strlen(message));
        setTrainState(addr, order, value, functionState);
      }
      break;
    case HMI_OrderStopAll :
      if (laBoxState != Labox_StateDCCOFF) {
        sprintf(message, "%s", TXT_StopAll);
        laBoxState = Labox_StateDCCOFF;
        pushMessageOnStack(message, (uint8_t) strlen(message));
//        for(int i=0; i < HMI_NbMemorisedTrain; i++)   // Est-ce que l'on veut conserver l'état de tous les trains ou on efface tout ?
//        {
//          tabTrains[i].setInfo(0,0,0);
//        }
      }else
      { // bi-stable function
        sprintf(message, "%s", TXT_StartDCC);
        laBoxState = Labox_StateDCCON;
        pushMessageOnStack(message, (uint8_t) strlen(message));
      }
      break;
    case HMI_StartDCC :
      if (laBoxState == Labox_StateDCCOFF) {
        sprintf(message, "%s", TXT_StartDCC);
        laBoxState = Labox_StateDCCON;
        pushMessageOnStack(message, (uint8_t) strlen(message));
      }
      break;
    case HMI_ShortCurcuit :
      sprintf(message, "%s", TXT_ShortCircuit);
      laBoxState = Labox_StateSHORTCIRCUIT;
      pushMessageOnStack(message, (uint8_t) strlen(message));
      break;
    case HMI_WifiWaiting :
      sprintf(message, "%s", TXT_WifiWaiting);
      _HMIState = StateWifiWaiting ;
      laBoxState = Labox_StateDCCOFF;
      pushMessageOnStack(message, (uint8_t) strlen(message));
      break;   
    case HMI_noWifi :
      sprintf(message, "%s", TXT_noWifi);
      _HMIState = StateNoWifi ;
      laBoxState = Labox_StateDCCOFF;
      pushMessageOnStack(message, (uint8_t) strlen(message));
    break; 
    case HMI_WifiOk :
      sprintf(message, "%s", TXT_WifiOk);
      _HMIState = StateDashboard ;
      laBoxState = Labox_StateDCCOFF;
      pushMessageOnStack(message, (uint8_t) strlen(message));
    break;
    
    default :
      sprintf(message, "@%04d-%s%03d", addr, "??", value);
  }
  //------------- Special case depending on the condition of the box ---------------
  if(_HMIState == StateDashboard || _HMIState == StateDashboardTrainView)
  {
    switch (laBoxState)
    {
      case Labox_StateDCCOFF  :       
      case Labox_StateSHORTCIRCUIT :  _HMIState = StateDashboard ;
      break;    
      case Labox_StateDCCON   : 
            if(tabTrains[0].addr > 0)
              _HMIState = StateDashboardTrainView ;
            else
              _HMIState = StateDashboard ;
      break;
      default:;
    }
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::addNotification().. End");
}
/*!
    @brief  setTrainState, update information in train object
            This function must be called each time that someone want to show any informations on display
            The function places the last piloted train at the top of the list
    @param  addr : train address 
    @param  order : i.e. HMI_OrderForward, HMI_OrderBack, HMI_OrderStop, HMI_OrderFunction
    @return None (void).
    @note
*/ 
void hmi::setTrainState(int addr, uint8_t order, uint8_t value, bool state)
{
/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore)
    return;
#endif*/

  _HMIDEBUG_FCT_PRINTLN("hmi::setTrainState().. Begin");
  _HMIDEBUG_PARAMS_PRINT("hmi::setTrainState, addr :  ");_HMIDEBUG_PARAMS_PRINT(addr); _HMIDEBUG_PARAMS_PRINT(", order : ");_HMIDEBUG_PARAMS_PRINT(order);_HMIDEBUG_PARAMS_PRINT(", value : ");_HMIDEBUG_PARAMS_PRINTLN(value);
  int i = 0;
  // Search train in train tab
  while(i < HMI_NbMemorisedTrain)
  {
    if(&tabTrains[i])  // safety
    {
      if(tabTrains[i].addr == addr)
      { 
        break; // Found !
      }
    }
    i++ ;
  }
  // Has the object been found?
  if(i == HMI_NbMemorisedTrain) i--; 
  switch(i)
  {
    case 0 : // The train that we pilot is already the first at the top of the list
      tabTrains[0].setInfo(addr, order, value, state);
    break;
    case HMI_NbMemorisedTrain : 
      i--; // We remove one to place on the last index of the table
	   [[fallthrough]];
    default :

#ifdef _HMIDEBUG_LEVEL3_PRINTLN
      for(int z=0; z < HMI_NbMemorisedTrain;z++)
      { // Stack shift to update the updated object at the very top
        _HMIDEBUG_LEVEL3_PRINT("Fonction setTrainState, Tableau de train : "); _HMIDEBUG_LEVEL3_PRINT(z);_HMIDEBUG_LEVEL3_PRINT(", Valeur addr : "); _HMIDEBUG_LEVEL3_PRINTLN(tabTrains[z].addr );
      }
#endif  
      // Reorder train tab, this order became the first, the last is erased
      for(int j=i; j>0;j--)
      { // Stack shift to update the updated object at the very top
        tabTrains[j] = tabTrains[j-1] ;
      }
      // Update information
      tabTrains[0].setInfo(addr, order, value, state);
#ifdef _HMIDEBUG_LEVEL3_PRINTLN
      for(int z=0; z < HMI_NbMemorisedTrain;z++)
      { // Stack shift to update the updated object at the very top
        _HMIDEBUG_LEVEL3_PRINT("Fonction setTrainState, Nouveau Tableau de train : "); _HMIDEBUG_LEVEL3_PRINT(z);_HMIDEBUG_LEVEL3_PRINT(", Valeur addr : "); _HMIDEBUG_LEVEL3_PRINTLN(tabTrains[z].addr );
      }
#endif  
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::setTrainState().. End");  
}
/*!
    @brief  addMessage, add a message to the HMI stack.
            This function must be called each time that someone want to show any informations on display
    @param  msg : String/0, 11 char max
    @return None (void).
    @note
*/
void hmi::addNotification(const char* msg)
{
  _HMIDEBUG_FCT_PRINTLN("hmi::addNotification().. Begin");  
  if (msg)
  {
    memset(message, 0, LineCarNbMax);
    memcpy(message, msg, HMI_MessageSize - 1);
    pushMessageOnStack(message, (uint8_t) strlen(message) );
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::addNotification().. End");  
}
/*!
    @brief  readVoltage
            read the voltage value from analog pin
    @param  None
    @return None (void).
    @note
*/
void hmi::readVoltage()
{
  MotorDriver *  mainDriver=NULL;
  for(const auto& md: TrackManager::getMainDrivers()) {
    mainDriver=md;
  }

  if (mainDriver == NULL)
  {
    voltage = 0;
    return;
  }

  _HMIDEBUG_FCT_PRINTLN("hmi::readVoltage().. Begin");  
  voltage = local_adc1_get_raw(pinToADC1Channel(PIN_VOLTAGE_MES)) * HMI_VoltageK;

#ifdef _HMIDEBUG_SIMUL
  voltage = ((float)random(175, 185)) / 10;
#endif
  _HMIDEBUG_FCT_PRINTLN("hmi::readVoltage().. End");  
}
/*!
    @brief  readCurrent
            Read the curent value from analog pin
    @param  None
    @return None (void).
    @note
*/
void hmi::readCurrent()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::readCurrent().. Begin");    
  MotorDriver *  mainDriver=NULL;
  for(const auto& md: TrackManager::getMainDrivers()) {
    mainDriver=md;
  }

  if (mainDriver == NULL || !mainDriver->canMeasureCurrent())
  {
    current = 0;
    return;
  }

  //current = mainDriver->getCurrentRaw() * HMI_CurrentK;
  // DB : remplacé par une moyenne de 50 mesures 
  float base = 0;
	for (int j = 0; j < 50; j++)
	{
		float val = (float)analogRead(mainDriver->getCurrentPin());
		base += val;
	}
	current = (float) (((base / 50) * HMI_CurrentK) - HMI_deltaCurrent);
#ifdef _HMIDEBUG_SIMUL
  current = ((float)random(0, 410)) / 100;
#endif  
  _HMIDEBUG_FCT_PRINTLN("hmi::readCurrent().. End");  
}

/*!
    @brief  BtnUPpressed
            Call when button is pressed
    @param  None
    @return None (void).
    @note : /!\ It's a STATIC FUNCTION
*/
void hmi::BtnUpPressed()
{
  _HMIDEBUG_SM_PRINTLN("hmi::BtnUP Pressed"); 
  _HMIEvent = eventUp;
}

/*!
    @brief  BtnDownpressed
            Call when button is pressed
    @param  None
    @return None (void).
    @note : /!\ It's a STATIC FUNCTION
*/
void hmi::BtnDownPressed()
{
  _HMIDEBUG_SM_PRINTLN("hmi::BtnDown Pressed");  
  _HMIEvent = eventDown;
} 
/*!
    @brief  BtnSelectpressed
            Call when button is pressed
    @param  None
    @return None (void).
    @note : /!\ It's a STATIC FUNCTION
*/
void hmi::BtnSelectPressed()
{
  _HMIDEBUG_SM_PRINTLN("hmi::BtnSelect Pressed"); 
  _HMIEvent = eventSel;

}
/*!
    @brief  pushMessageOnStack
            This function push a new message in the stack by shifting all the elements
            and deleting the last one from the list.
    @param  msg : Short message, 12 characters maximum corresponding to the last order received.
    @param  len : string lengh
    @return None (void).
    @note
*/
void hmi::pushMessageOnStack(const char *msg, uint8_t len)
{
  _HMIDEBUG_FCT_PRINTLN("hmi::pushMessageOnStack().. Begin"); 
  if (msg)
  {
    // Stack shifting
    for (int i = HMI_StackNbCarElt - 1; i > 0 ; i--)
    {
      memcpy(messageStack[i], messageStack[i - 1], HMI_MessageSize - 1);
    }
    // Add the new message to the top of the stack
    memset(messageStack[0], 0, HMI_MessageSize);
    memcpy(messageStack[0], msg, HMI_MessageSize - 1);
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::pushMessageOnStack().. End"); 
}
/*!
    @brief  needToRefreshDisplay
            This function return true if we have to refress display
    @return bool
    @note
*/
bool hmi::needToRefreshDisplay(){
  _HMIDEBUG_FCT_PRINTLN("hmi::needToRefreshDisplay().. Begin");
  if ( (millis() - millisRefreshDisplay) > HMI_DisplayRefesh)
  {
    millisRefreshDisplay = millis();
    return true;
  }else
    return false;
  _HMIDEBUG_FCT_PRINTLN("hmi::needToRefreshDisplay().. End"); 
}
/*!
    @brief  showWifiWaiting
            This function show a display to waiting wifi connection
    @return none
    @note
*/
void hmi::showWifiWaiting()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::showWifiWaiting().. Begin");
  millisWifiEffect      = millis();
  int circleNumber ;
  
  if(wifiCircle == 0) 
    wifiCircle = 7;
  else
    wifiCircle--;
  clearDisplay();
  
  for(circleNumber = wifiCircle; circleNumber > 0 ; circleNumber-- )
  {
    if(circleNumber % 2)
      fillCircle(SCREEN_WIDTH/2, 24, 3 + circleNumber*3, WHITE );
    else
      fillCircle(SCREEN_WIDTH/2, 24, 3 + circleNumber*3, BLACK );
  }
  wifiCircle--;
  fillTriangle(36,0,SCREEN_WIDTH/2,21,127-36,0,BLACK);
  fillTriangle(36,52,SCREEN_WIDTH/2,21,127-36,52,BLACK);

  setTextSize(1);
  setCursor(5, 53);
  println(TXT_WifiWaiting);
  setTextColor(WHITE);
  
  display();

  _HMIDEBUG_FCT_PRINTLN("hmi::showWifiWaiting().. End");
  
}

#endif
#endif