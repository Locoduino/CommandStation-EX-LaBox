#include "defines.h"
#include "DCC.h"
#include "TrackManager.h"
#include "DCCRMT.h"
#include "DCCTimer.h"
#include "Railcom.h"
#include "LaboxModes.h"

#ifdef ENABLE_RAILCOM
//----------------------------------------------------------------------------
hw_timer_t * TimerCutOut = NULL;
int rmt_channel;                                                             // * Variable
int p;                                                                        // * Variable
gpio_num_t railcom_pins[4]; // Max 3 main tracks including boosters. Empty slots are set to GPIO_NUM_NC
gpio_num_t railcom_invpins[4]; // Max 3 main tracks including boosters. Empty slots are set to GPIO_NUM_NC
bool pauseRailcom = false;

void IRAM_ATTR timer_isr_CutOut() {
	if (p < 1000) {
		p++;
		for (int i = 0; railcom_pins[i] != GPIO_NUM_NC; i++) {
			switch (p) {
				case 1:
					break;
				case 2:
					gpio_matrix_out(railcom_invpins[i], 0x100, false, false);                      // * Déconnecte la pin 27 du module RMT
					gpio_set_level(railcom_invpins[i], 1);                                         // * Pin 27 à l'état haut
					break;
				case 3:
					gpio_matrix_out(railcom_invpins[i], RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 27 au module RMT en inverse
					gpio_matrix_out(railcom_pins[i], RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 33 au module RMT en inverse
					break;
				default:
					gpio_set_level(railcom_pins[i], 1); 
					gpio_matrix_out(railcom_pins[i], RMT_SIG_OUT0_IDX + rmt_channel, false, false);   // * Reconnecte la pin 33 au module RMT
					timerAlarmWrite(TimerCutOut, 160, false);                               // * Arrêt Timer
					break;
			}
		}
	}
}

void RailcomBegin() {
	// No railcom in prog mode
	if (LaboxModes::progMode)
		return;

	TimerCutOut = timerBegin(3, 80, true);  
	if (TimerCutOut == NULL)
		DIAG(F("ERROR RAILCOM CUTOUT TIMER"));

	// * Timer RailCom
	timerAttachInterrupt(TimerCutOut, &timer_isr_CutOut, true);

	for (int i = 0; i < 4; i++) {
		railcom_pins[i] = GPIO_NUM_NC;
		railcom_invpins[i] = GPIO_NUM_NC;
	}

	int pinCount = 0;

	// Even in base mode, the Prog track is joined and should be recognized as main track.
	for(const auto& md: TrackManager::getDrivers((TRACK_MODE) 0xFF)) {

		if (md == NULL) {
			DIAG("Main driver NULL found");
			continue;
		}

		if (LaboxModes::progBehavior != ProgBehaviorJoining && md->getMode() & TRACK_MODE_PROG) {
			DIAG("Prog driver found %d", md->getSignalPin().pin);
			continue; // Prog track is not supported for Railcom.
		}

		DIAG("Main driver found %d", md->getSignalPin().pin);

		pinpair p = md->getSignalPin();

		railcom_pins[pinCount] = (gpio_num_t)p.pin;
		railcom_invpins[pinCount] = (gpio_num_t)p.invpin;
		pinCount++;
	}

	for(int i = 0; railcom_pins[i] != GPIO_NUM_NC; i++) {
		DIAG(F("Railcom activated %d : pin:%d invpin:%d"), i, railcom_pins[i], railcom_invpins[i]);
	}
}

void RailcomEnd() {

	if (LaboxModes::progMode)
		return;

	if (TimerCutOut != NULL) {
		timerStop(TimerCutOut);
		timerDetachInterrupt(TimerCutOut);
		timerEnd(TimerCutOut);
		TimerCutOut = NULL;

		p = 1000;
	}

	DIAG(F("Railcom stopped."));
}

bool isRailcomEnabled() {
	return (TimerCutOut != NULL);
}

void StarTimerCutOut(rmt_channel_t channel) {            // * Start Timer 
  if (!pauseRailcom && !LaboxModes::progMode && TimerCutOut != NULL) {
		for (int i = 0; railcom_pins[i] != GPIO_NUM_NC; i++) {
		  gpio_matrix_out(railcom_pins[i], 0x100, false, false);   // * Déconnecte la pin du module RMT
 			gpio_set_level(railcom_pins[i], 1);                      // * Pin à l'état haut
		}

 		rmt_channel = channel;                               // * Mémorise n° de canal
  	timerAlarmWrite(TimerCutOut, 160, true);             // * En continu
  	timerAlarmEnable(TimerCutOut);
  	p = 0;
	}
}

int setDCCBitCutOut(rmt_item32_t* item) {                     // * Temps CutOut fixé à 488µs (TCE Max.)
  if (!pauseRailcom && !LaboxModes::progMode) {     
		item->level0    = 1;
		item->duration0 = 29;                                        // * 29µs CutOut Start (TCS)
		item->level1    = 0;
		item->duration1 = 215;                                       // * 215 + 29 + 215 = 459µs (TCE - TCS)
		return 1;
	}

	return 0;
}
#endif