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
		static bool ChangeCVs(int address, int value)
#ifdef LABOX_CV_ADDRESS
;
#else
{ return false; }
#endif

		static bool ParseLB(Print *stream, int16_t params, int16_t p[])
#ifdef LABOX_CV_ADDRESS
;
#else
{ return false; }
#endif

		static bool DIAGLABOXDC;
		static bool DIAGLABOXCVS;
};	

class LaboxDC {
	public:
		static void begin()
#ifdef LABOX_DC_CAB
;
#else
{}
#endif
		static void SetSpeed(int val)
#ifdef LABOX_DC_CAB
;
#else
{}
#endif
		static bool SetDirection(bool forward)
#ifdef LABOX_DC_CAB
;
#else
{ return false; }
#endif
		static void Stop()
#ifdef LABOX_DC_CAB
;
#else
{}
#endif
		static bool Power(bool on)
#ifdef LABOX_DC_CAB
;
#else
{ return false; }
#endif
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