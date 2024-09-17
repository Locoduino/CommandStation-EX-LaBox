/*
 * LaBox Project
 * Railcom part
 *
 * @Author : lebalge2
 * @Organization : Locoduino.org
 */

#pragma once

#ifdef ENABLE_RAILCOM

#include "DCCRMT.h"

void RailcomBegin();
void StarTimerCutOut(rmt_channel_t channel);
int setDCCBitCutOut(rmt_item32_t* item);
#endif