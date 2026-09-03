//-------------------------------------------------------------------
#ifndef __LaboxCVs_hpp__
#define __LaboxCVs_hpp__
//-------------------------------------------------------------------

#include <Arduino.h>
#include "defines.h"

// LaBox specific CV addresses
#define LABOX_CV_WIFI_ENABLE				1
#define LABOX_CV_RAILCOM_ENABLE			7

class Labox {
	public:
		static bool ChangeCVs(int address, int value);
		static bool ParseLB(Print *stream, int16_t params, int16_t p[]);

		static bool DIAGLABOXDC;
		static bool DIAGLABOXCVS;
};	

class LaboxDC {
	public:
		static void begin();
		static void SetSpeed(int val);
		static bool SetDirection(bool forward);
		static void Stop();
		static bool Power(bool on);
		static bool isPowered() { return powered; }

		static bool powered;
		static bool forward;
		static bool started;
		static int speed;

		static LocoSlot *trainSlot;
		static int getSpeed() { return trainSlot ? trainSlot->getSpeedCode() & 0x7F : 0; }
		static bool getDirection() { return trainSlot ? (trainSlot->getSpeedCode() & 0x80) != 0 : true; }
};

#endif