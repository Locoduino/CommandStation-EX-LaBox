/*
   LaBox Project
   XpressNet part

   @Author : lebelge2
   @Organization : Locoduino.org
*/

#ifndef __XPressNet_h
#define __XPressNet_h

#include "DCC.h"
#include "EXComm.h"

#ifdef ENABLE_XPRESSNET 
const bool DIAG_XPNET = false;

class XPressNet: public EXCommItem {
public:
	XPressNet(int inRxPin, int inTxPin, int inDirPin);
	bool begin() override;
	bool loop() override;

	void getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) override;

	static void Decodage();
	static void Tx9(int dTx);
	static void F0a28();
	static void DeconnectPin();
	static void ConnectPin();
};

#endif
#endif
