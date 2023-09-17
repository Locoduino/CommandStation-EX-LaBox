/*
 * La Box Project
 * HMITrain Class 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#include "DCCEX.h"
#include "hmi.h"
#include "hmiTrain.h"
#include "icons.h"
#include "globals.h"

#ifdef USE_HMI
extern enumHMIState  _HMIState ;
extern enumHMIState  _HMIState_prev;
extern enumEvent     _HMIEvent;
/*!
    @brief  hmiTrain Constructor
    @param  _hmi, an hmi object to permit to display our menu
    @return None (void).
    @note
*/
hmiTrain::hmiTrain(hmi* _hmi)
{
	display = _hmi;
  addr = 0;
}
/*!
    @brief  hmiTrain Constructor (default)
    @return None (void).
    @note
*/
hmiTrain::hmiTrain()
{
  display = 0;
  addr = 0;
  lastFunction = -1;
}
/*!
    @brief  setHMIpointer
    @param  _hmi, an hmi object to permit to display our menu
    @return None (void).
    @note
*/
void hmiTrain::setHMIpointer(hmi* _hmi)
{
  display = _hmi;
}
/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void hmiTrain::update()
{
  // nothing to do ?
  // One possibility would be to remove code from the HMI class and put it here in this function 
  // which would be called in the update of the hmi class. 
  // !!!!!! To see later. !!!!!!!!!!
}
/*!
    @brief  setInfo, update information as mode and speed
    @param  trainMode like 
    @param  trainSpeed (0->128)
    @param  functionState
    @return None (void).
    @note
*/
void hmiTrain::setInfo(int _addr, uint8_t _order, uint8_t _value, bool _functionState)
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::setInfo.. Begin"); 
  _HMIDEBUG_PARAMS_PRINT("hmiTrain::setInfo, addr :  ");_HMIDEBUG_PARAMS_PRINT(_addr); _HMIDEBUG_PARAMS_PRINT(", order : ");_HMIDEBUG_PARAMS_PRINT(_order);_HMIDEBUG_PARAMS_PRINT(", value : ");_HMIDEBUG_PARAMS_PRINTLN(_value);
  addr    = _addr ; 
  switch(_order)
  {
    case HMI_OrderForward :
    case HMI_OrderBack :
      speed = _value;
      mode = speed ? _order : HMI_OrderStop;
    break;
    case HMI_OrderStop :
      speed = 0 ;
      mode = _order;
    break;
    case HMI_OrderFunction :
      if(_value <= HMITrain_NbMaxFunctions && function[_value] != _functionState)
      {
        function[_value] = _functionState;
        if(_value!=0 ) lastFunction = _value ;   //   Exception for function 0 (lights)
        //_HMIDEBUG_LEVEL1_PRINT("setInfo, reception commande function n°");_HMIDEBUG_LEVEL1_PRINT(_value);_HMIDEBUG_LEVEL1_PRINT(", nouvelle valeur : ");_HMIDEBUG_LEVEL1_PRINTLN(function[_value] ? "True" : "False");
      }
    break;
    default :;
  }
  //_HMIDEBUG_FCT_PRINT(">> hmiTrain::setInfo, speed" );_HMIDEBUG_FCT_PRINTLN(speed );
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::setInfo.. End"); 
}
/*!
    @brief  dashboard
            Show train dashboard on multi train display
    @param  None
    @return None (void).
    @note
*/
void hmiTrain::dashboard(int pos_x1, int pos_y1, int pos_x2, int pos_y2)
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::dashboard.. Begin"); 
  _HMIDEBUG_PARAMS_PRINT("hmiTrain::dashboard, x1 :  ");_HMIDEBUG_PARAMS_PRINT(pos_x1); _HMIDEBUG_PARAMS_PRINT(", y1 : ");_HMIDEBUG_PARAMS_PRINT(pos_y1);_HMIDEBUG_PARAMS_PRINT(", x2 : ");_HMIDEBUG_PARAMS_PRINT(pos_x2);_HMIDEBUG_PARAMS_PRINT(", y2 : ");_HMIDEBUG_PARAMS_PRINTLN(pos_y2);
  //-------------------- Barres graph -------------------------
  int offsetYBarresGraph = 22 ;
  int offsetXBarresGraph = 3 ;
  //int nbRect = (pos_y2 - pos_y1 - 15) / (HMITrain_RectHeight+HMITrain_RectSpace) ;
  int step = 1;
  int nbStep = (pos_y2 - pos_y1 - 15) / (HMITrain_RectHeight+HMITrain_RectSpace) ;
  int ratioSpeed = HMITrain_NbStep / nbStep;
  int y = pos_y2-offsetYBarresGraph ;//-3

  while( (step*(HMITrain_RectHeight+HMITrain_RectSpace)) < (pos_y2 - pos_y1 -HMITrain_RectSpace/*- offset_barres_graph-2*/) )
  {
    if(speed <= ratioSpeed*(step-1))
    {
      display->drawRect(offsetXBarresGraph + pos_x1+(pos_x2-pos_x1)/2-step, y, ((pos_x2-pos_x1)/4)+2*step, HMITrain_RectHeight, WHITE);
    }else
    {
      display->fillRect(offsetXBarresGraph + pos_x1+(pos_x2-pos_x1)/2-step, y, ((pos_x2-pos_x1)/4)+2*step, HMITrain_RectHeight, WHITE);
    }      
    y-= HMITrain_RectHeight+HMITrain_RectSpace ;
    step++;

  }
  _HMIDEBUG_LEVEL2_PRINT("hmiTrain::dashboard, speed :  ");_HMIDEBUG_LEVEL2_PRINT(speed);_HMIDEBUG_LEVEL2_PRINT(", ratioSpeed : ");_HMIDEBUG_LEVEL2_PRINTLN(ratioSpeed);
  //-------------------- Train Address -------------------------
  display->setCursor( pos_x1+8, pos_y2-11);
  sprintf(message, "%04d", addr);
  display->println(message);
  //--------------------- Speed value --------------------------
  display->setCursor( pos_x1, pos_y2-25);
  sprintf(message, "%03d", speed);
  display->println(message);
  //-------------------- Arrows drawing ------------------------
  if(mode == HMI_OrderForward)
  {
    for(int j=0; j<3;j++)   //Drawing of the right arrow  
    {
      display->drawLine(pos_x2-4+j, pos_y2-12, pos_x2-0+j, pos_y2-8, WHITE);
      display->drawLine(pos_x2-4+j, pos_y2-4 , pos_x2-0+j, pos_y2-8, WHITE);      
    }
  }else
  {
    if(mode == HMI_OrderBack)
    {
      for(int j=0; j<3;j++)   //Drawing of the left arrow
      {
        display->drawLine(pos_x1+j, pos_y2-8, pos_x1+4+j, pos_y2-12, WHITE);
        display->drawLine(pos_x1+j, pos_y2-8, pos_x1+4+j, pos_y2-4, WHITE);
      }
    }
  }
  //------------------- Light symbol (F0)-------------------------
  int center_x = pos_x1 + 6 ;
  int center_y = pos_y2 - 35 ;

  if(function[0] )   // Function 0      //DEBUG DEBUG DEBUG
  {
    drawLightSymbol(center_x, center_y, 6);
  }
  //--------------------------------------------------------------
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::dashboard.. End"); 
}
/*!
    @brief  dashboard1T
            Show train dashboard on display for only one train (full screen)
    @param  coordonnates of window
    @return None (void).
    @note
*/
void hmiTrain::dashboard1T()
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::dashboard1T.. Begin"); 
  display->setTextSize(1);
  //-------------------- Barres graph -------------------------
  int offsetYBarresGraph = 46 ;
  int offsetXBarresGraph = 45 ;
  //int nbRect = (SCREEN_WIDTH - offsetXBarresGraph - 15) / (HMITrain_RectWidth+HMITrain_RectSpace) ;
  int step = 1;
#ifdef _HMIDEBUG_LEVEL2
  int nbStep = (SCREEN_WIDTH - offsetXBarresGraph - 15) / (HMITrain_RectWidth+HMITrain_RectSpace) ;
#endif
  int ratioSpeed = 14; // SCREEN_WIDTH / 9 blocks
  int x = offsetXBarresGraph ;

  while( (step*(HMITrain_RectWidth+HMITrain_RectSpace)) < (SCREEN_WIDTH - offsetXBarresGraph - HMITrain_RectSpace) )
  {
    if(speed < ratioSpeed*(step))
    {
      display->drawRect(x, offsetYBarresGraph - (int)(5 + step + round(exp2(step)/(step*2))), HMITrain_RectWidth, (int)(5 + step + round(exp2(step)/(step*2))), WHITE);
    }else
    {
      display->fillRect(x, offsetYBarresGraph - (int)(5 + step + round(exp2(step)/(step*2))), HMITrain_RectWidth, (int)(5 + step + round(exp2(step)/(step*2))), WHITE);
    }      
    x += HMITrain_RectWidth+HMITrain_RectSpace ;
    step++;
  }
  _HMIDEBUG_LEVEL2_PRINT("hmiTrain::dashboard1T, speed :  ");_HMIDEBUG_LEVEL2_PRINT(speed);_HMIDEBUG_LEVEL2_PRINT(", ratioSpeed : ");_HMIDEBUG_LEVEL2_PRINTLN(ratioSpeed);_HMIDEBUG_LEVEL2_PRINT(", nbStep : ");_HMIDEBUG_LEVEL2_PRINTLN(nbStep);
  //-------------------- Train Address -------------------------
  display->setTextSize(2);
  display->setCursor(43, 50);
  sprintf(message, "%04d", addr);
  display->println(message);
  //--------------------- Speed value --------------------------
  display->setTextSize(2);
  display->setCursor( 67, 3);
  sprintf(message, "%03d", speed);
  display->println(message);
  //-------------------- Arrows drawing ------------------------
  for(int k=0; k<15; k=k+6)
  { 
    if(mode == HMI_OrderForward)
    {
      for(int j=0; j<5;j++)   //Drawing of the right arrow  
      {
        display->drawLine(102+j+k, 49, 109+j+k, 56, WHITE);
        display->drawLine(102+j+k, 63, 109+j+k, 56, WHITE);      
      }
    }else
    {
      if(mode == HMI_OrderBack)
      {
        for(int j=0; j<5;j++)   //Drawing of the left arrow
        {
          display->drawLine(19+j-k, 56, 26+j-k, 49, WHITE);
          display->drawLine(19+j-k, 56, 26+j-k, 63, WHITE);   
        }
      }
    }
    k++;
  }
  //------------------ Voltage and current -----------------------
  display->setTextSize(1);
  display->setCursor(0, 0);
  sprintf(message, "%1.2f V", display->voltage);
  display->println(message);
  display->setCursor(0, 9);
  sprintf(message, "%1.2f mA",  display->current);
  display->println(message);
  //------------------- Light symbol (F0)-------------------------
  if(function[0] )   // Function 0      //DEBUG DEBUG DEBUG
  {
    drawLightSymbol(12, 39, 8);
  } 
  if(function[lastFunction] == true)
    drawFunctionSymbol(23,34, lastFunction);  //  Affiche le numéro de la fonction appelée
  
  //--------------------------------------------------------------
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::dashboard1T.. End"); 
}
/*!
    @brief  drawLightSymbol
            Draw the light symbol
    @param  pos_x : X coordinate
            pos_y : Y coordinate
    @return None (void).
    @note
*/
void hmiTrain::drawLightSymbol(int pos_x, int pos_y, int starSize)
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::drawLightSymbol().. Begin");

  if(starSize<6) starSize = 6; // 6 minimum
  
  display->fillCircle(pos_x, pos_y, starSize/3, WHITE);
  for(int i = 0; i < 8 ; i++ )
  {
    display->drawLine(  pos_x + ((int)(starSize*2/3  * cos(i*PI/4))), 
                        pos_y + ((int)(starSize*2/3  * sin(i*PI/4))), 
                        pos_x + ((int)(starSize      * cos(i*PI/4))), 
                        pos_y + ((int)(starSize      * sin(i*PI/4))), 
                        WHITE); 
                        
  }
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::drawLightSymbol().. End"); 
}
/*!
    @brief  drawFunctionSymbol
            Draw the function number with a bord
    @param  pos_x : X coordinate
            pos_y : Y coordinate
            functionNumber : number of function [0;HMITrain_NbMaxFunctions]
    @return None (void).
    @note
*/
void hmiTrain::drawFunctionSymbol(int pos_x, int pos_y, int functionNumber)
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::drawFunctionSymbol().. Begin"); 
  display->setTextSize(1);
  display->setCursor(pos_x+3, pos_y+3);
  sprintf(message, "%d", functionNumber);
  display->println(message);
  
  int width = 11;
  if(functionNumber >= 10) width = 17;  // larger for 2 digits
  
  display->drawRect(  pos_x, 
                      pos_y,
                      width,    
                      13,       // height
                      WHITE);
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::drawFunctionSymbol().. End"); 
}
/*!
    @brief  operator=
            operateur to copy an object
    @param  object hmiTrain
    @return None (void).
    @note
*/
hmiTrain & hmiTrain::operator=(const hmiTrain & _hmiTrain)
{
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::operator=.. Begin"); 
  if(&_hmiTrain != this)
  {
    display = _hmiTrain.display ;
    speed   = _hmiTrain.speed ;
    mode    = _hmiTrain.mode ;                 // Forward, back, stop
    addr    = _hmiTrain.addr ;                 // train address
    for(int i=0; i < HMITrain_NbMaxFunctions ; i++)
    {
      function[i] = _hmiTrain.function[i];
    }
  }
  _HMIDEBUG_FCT_PRINTLN("hmiTrain::operator=.. End"); 
  return *this;
}
#endif