/*
 * La Box Project
 * HMITrain Class 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifdef USE_HMI
#ifndef _HMITrain
#define _HMITrain
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "hmiConfig.h"
class hmi;
class hmiTrain
{
  public:
    //----- Members
  hmiTrain();
  hmiTrain(hmi* _hmi);
	hmi*	display;
	byte	speed;			          // 0->128
	byte 	mode;	                // Forward, back, stop
  int   addr;                 // train address
  bool  function[HMITrain_NbMaxFunctions+1];          // function state
  int lastFunction ;
    
	//----- functions
	void begin();
  void update();
  void setHMIpointer(hmi* _hmi);
	void setInfo(int addr, uint8_t order, uint8_t value, bool functionState = false);
	void dashboard(int pos_x1, int pos_y1, int pos_x2, int pos_y2);
  void dashboard1T();
  void drawLightSymbol(int pos_x, int pos_y, int starSize);
  void drawFunctionSymbol(int pos_x, int pos_y, int functionNumber);
  hmiTrain & operator=(const hmiTrain & hmiTrain);

  protected:
    //----- Members
  char message[LineCarNbMax];
    //----- functions

};



#endif
#endif