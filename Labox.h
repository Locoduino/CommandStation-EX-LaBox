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

	private:
		static bool DIAGLABOXCVS;
};
	
#endif