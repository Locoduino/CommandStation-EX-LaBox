#include "DCCEX.h"
#include "DCCRMT.h"
#include "DCCTimer.h"
#include "Railcom.h"

#ifdef ENABLE_RAILCOM
//----------------------------------------------------------------------------
hw_timer_t * TimerCutOut = NULL;
extern int rmt_channel;                                                             // * Variable
volatile int p;                                                                        // * Variable

void IRAM_ATTR timer_isr_CutOut() {
  p++;
  switch (p) {
    case 1:
      break;
    case 2:
      gpio_matrix_out(RAILCOM_PININV, 0x100, false, false);                      // * Déconnecte la pin 27 du module RMT
      gpio_set_level(RAILCOM_PININV, 1);                                         // * Pin 27 à l'état haut
      break;
    case 3:
      gpio_matrix_out(RAILCOM_PININV, RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 27 au module RMT en inverse
      gpio_matrix_out(RAILCOM_PIN, RMT_SIG_OUT0_IDX + rmt_channel, true, false);    // * Reconnecte la pin 33 au module RMT en inverse
      break;
    default:
		  gpio_set_level(RAILCOM_PIN, 1); 
      gpio_matrix_out(RAILCOM_PIN, RMT_SIG_OUT0_IDX + rmt_channel, false, false);   // * Reconnecte la pin 33 au module RMT
      timerAlarmWrite(TimerCutOut, 160, false);                               // * Arrêt Timer
      break;
  }
}

void RailcomBegin() {
//  Serial.print("Version IDF: ");
//  Serial.println(esp_get_idf_version());                             
  TimerCutOut = timerBegin(3, 80, true);                                   // * Timer 3 RailCom
  timerAttachInterrupt(TimerCutOut, &timer_isr_CutOut, true);
}

void StarTimerCutOut() {                                                   // * Start Timer 
  timerAlarmWrite(TimerCutOut, 160, true);          // * En continu
  timerAlarmEnable(TimerCutOut);
  p = 0;
}

void setDCCBitCutOut(rmt_item32_t* item) {                     // * Temps CutOut fixé à 488µs (TCE Max.)
  item->level0    = 1;
  item->duration0 = 29;                                        // * 29µs CutOut Start (TCS)
  item->level1    = 0;
  item->duration1 = 215;                                       // * 215 + 29 + 215 = 459µs (TCE - TCS)
}
#endif