#include "DCCEX.h"
#include "DCCRMT.h"
#include "DCCTimer.h"
#include "Railcom.h"
#include "LaboxModes.h"

#ifdef ENABLE_RAILCOM
//----------------------------------------------------------------------------
hw_timer_t * TimerCutOut = NULL;
int rmt_channel;                                                             // * Variable
int p;                                                                        // * Variable
gpio_num_t railcom_pin;
gpio_num_t railcom_invpin;

void IRAM_ATTR timer_isr_CutOut() {
  p++;
  switch (p) {
    case 1:
      break;
    case 2:
      gpio_matrix_out(railcom_invpin, 0x100, false, false);                      // * Déconnecte la pin 27 du module RMT
      gpio_set_level(railcom_invpin, 1);                                         // * Pin 27 à l'état haut
      break;
    case 3:
      gpio_matrix_out(railcom_invpin, RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 27 au module RMT en inverse
      gpio_matrix_out(railcom_pin, RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 33 au module RMT en inverse
      break;
    default:
		  gpio_set_level(railcom_pin, 1); 
      gpio_matrix_out(railcom_pin, RMT_SIG_OUT0_IDX + rmt_channel, false, false);   // * Reconnecte la pin 33 au module RMT
      timerAlarmWrite(TimerCutOut, 160, false);                               // * Arrêt Timer
      break;
  }
}

void RailcomBegin() {
	DIAG(F("Railcom activated"));
  TimerCutOut = timerBegin(3, 80, true);                                   // * Timer 3 RailCom
  timerAttachInterrupt(TimerCutOut, &timer_isr_CutOut, true);

  for(const auto& md: TrackManager::getMainDrivers()) {

		if (md == NULL)
			continue;

    pinpair p = md->getSignalPin();

		// Railcom protocol will work only for the first driver...
		railcom_pin = (gpio_num_t)p.pin;
		railcom_invpin = (gpio_num_t)p.invpin;
		break;
  }
}

void StarTimerCutOut(rmt_channel_t channel) {                                                   // * Start Timer 
  if (!LaboxModes::progMode) {
	  gpio_matrix_out(railcom_pin, 0x100, false, false);            // * Déconnecte la pin 33 du module RMT
 		gpio_set_level(railcom_pin, 1);                               // * Pin 33 à l'état haut
 		rmt_channel = channel;                                              // * Mémorise n° de canal
  	timerAlarmWrite(TimerCutOut, 160, true);          // * En continu
  	timerAlarmEnable(TimerCutOut);
  	p = 0;
	}
}

int setDCCBitCutOut(rmt_item32_t* item) {                     // * Temps CutOut fixé à 488µs (TCE Max.)
  if (!LaboxModes::progMode) {     
		item->level0    = 1;
		item->duration0 = 29;                                        // * 29µs CutOut Start (TCS)
		item->level1    = 0;
		item->duration1 = 215;                                       // * 215 + 29 + 215 = 459µs (TCE - TCS)
		return 1;
	}

	return 0;
}
#endif