/*
 *  © 2022 Chris Harlow
 *  © 2022 Harald Barth
 *  All rights reserved.
 *  
 *  This file is part of DCC++EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "TrackManager.h"
#include "FSH.h"
#include "DCCWaveform.h"
#include "DCC.h"
#include "MotorDriver.h"
#include "DCCTimer.h"
#include "DIAG.h"
#include "hmi.h"
// Virtualised Motor shield multi-track hardware Interface
#define FOR_EACH_TRACK(t) for (byte t=0;t<=lastTrack;t++)
    
#define APPLY_BY_MODE(findmode,function) \
        FOR_EACH_TRACK(t) \
            if (trackMode[t]==findmode) \
                track[t]->function;

const int16_t HASH_KEYWORD_PROG = -29718;
const int16_t HASH_KEYWORD_MAIN = 11339;
const int16_t HASH_KEYWORD_OFF = 22479;  
const int16_t HASH_KEYWORD_DC = 2183;  
const int16_t HASH_KEYWORD_DCX = 6463; // DC reversed polarity 
const int16_t HASH_KEYWORD_EXT = 8201; // External DCC signal
const int16_t HASH_KEYWORD_A = 65; // parser makes single chars the ascii.   

MotorDriver * TrackManager::track[MAX_TRACKS];
TRACK_MODE TrackManager::trackMode[MAX_TRACKS];
int16_t TrackManager::trackDCAddr[MAX_TRACKS];

POWERMODE TrackManager::mainPowerGuess=POWERMODE::OFF;
byte TrackManager::lastTrack=0;
bool TrackManager::progTrackSyncMain=false; 
bool TrackManager::progTrackBoosted=false; 
int16_t TrackManager::joinRelay=UNUSED_PIN;
#ifdef ARDUINO_ARCH_ESP32
byte TrackManager::tempProgTrack=MAX_TRACKS+1;
#endif


// The setup call is done this way so that the tracks can be in a list 
// from the config... the tracks default to NULL in the declaration                 
void TrackManager::Setup(const FSH * shieldname,
        MotorDriver * track0, MotorDriver * track1, MotorDriver * track2,
        MotorDriver * track3, MotorDriver * track4,  MotorDriver * track5,
        MotorDriver * track6, MotorDriver * track7 ) {       
    addTrack(0,track0);
    addTrack(1,track1);
    addTrack(2,track2);
    addTrack(3,track3);
    addTrack(4,track4);
    addTrack(5,track5);
    addTrack(6,track6);
    addTrack(7,track7);
    
    // Default the first 2 tracks (which may be null) and perform HA waveform check.
    setTrackMode(0,TRACK_MODE_MAIN);
    setTrackMode(1,TRACK_MODE_PROG);
  
  // TODO Fault pin config for odd motor boards (example pololu)
  // MotorDriver::commonFaultPin = ((mainDriver->getFaultPin() == progDriver->getFaultPin())
  //				 && (mainDriver->getFaultPin() != UNUSED_PIN));
  DCC::begin(shieldname);   
}

void TrackManager::addTrack(byte t, MotorDriver* driver) {
     trackMode[t]=TRACK_MODE_OFF;
     track[t]=driver;
     if (driver) {
         track[t]->setPower(POWERMODE::OFF);
         lastTrack=t;
     } 
}

// setDCCSignal(), called from interrupt context
// does assume ports are shadowed if they can be
void TrackManager::setDCCSignal( bool on) {
  HAVE_PORTA(shadowPORTA=PORTA);
  HAVE_PORTB(shadowPORTB=PORTB);
  HAVE_PORTC(shadowPORTC=PORTC);
  APPLY_BY_MODE(TRACK_MODE_MAIN,setSignal(on));
  HAVE_PORTA(PORTA=shadowPORTA);
  HAVE_PORTB(PORTB=shadowPORTB);
  HAVE_PORTC(PORTC=shadowPORTC);
}

void TrackManager::setCutout( bool on) {
    (void) on;
    // TODO Cutout needs fake ports as well
    // TODO      APPLY_BY_MODE(TRACK_MODE_MAIN,setCutout(on));
}

// setPROGSignal(), called from interrupt context
// does assume ports are shadowed if they can be
void TrackManager::setPROGSignal( bool on) {
  HAVE_PORTA(shadowPORTA=PORTA);
  HAVE_PORTB(shadowPORTB=PORTB);
  HAVE_PORTC(shadowPORTC=PORTC);
  APPLY_BY_MODE(TRACK_MODE_PROG,setSignal(on));
  HAVE_PORTA(PORTA=shadowPORTA);
  HAVE_PORTB(PORTB=shadowPORTB);
  HAVE_PORTC(PORTC=shadowPORTC);
}

// setDCSignal(), called from normal context
// MotorDriver::setDCSignal handles shadowed IO port changes.
// with interrupts turned off around the critical section
void TrackManager::setDCSignal(int16_t cab, byte speedbyte) {
  FOR_EACH_TRACK(t) {
    if (trackDCAddr[t]!=cab) continue;
    if (trackMode[t]==TRACK_MODE_DC) track[t]->setDCSignal(speedbyte);
    else if (trackMode[t]==TRACK_MODE_DCX) track[t]->setDCSignal(speedbyte ^ 128);
  }
}    

bool TrackManager::setTrackMode(byte trackToSet, TRACK_MODE mode, int16_t dcAddr) {
  if (trackToSet>lastTrack || track[trackToSet]==NULL) return false;

  //DIAG(F("Track=%c"),trackToSet+'A');
  // DC tracks require a motorDriver that can set brake!
  if ((mode==TRACK_MODE_DC || mode==TRACK_MODE_DCX)
        && !track[trackToSet]->brakeCanPWM()) {
            DIAG(F("Brake pin can't PWM: No DC"));
            return false; 
  }

#ifdef ARDUINO_ARCH_ESP32
  // remove pin from MUX matrix and turn it off
  pinpair p = track[trackToSet]->getSignalPin();
    //DIAG(F("Track=%c remove  pin %d"),trackToSet+'A', p.pin);
    gpio_reset_pin((gpio_num_t)p.pin);
    pinMode(p.pin, OUTPUT); // gpio_reset_pin may reset to input
    if (p.invpin != UNUSED_PIN) {
      //DIAG(F("Track=%c remove ^pin %d"),trackToSet+'A', p.invpin);
      gpio_reset_pin((gpio_num_t)p.invpin);
      pinMode(p.invpin, OUTPUT); // gpio_reset_pin may reset to input
    }
#endif
  if (mode==TRACK_MODE_PROG) {
      // only allow 1 track to be prog
      FOR_EACH_TRACK(t)
	if (trackMode[t]==TRACK_MODE_PROG && t != trackToSet) {
	  track[t]->setPower(POWERMODE::OFF);
	  trackMode[t]=TRACK_MODE_OFF;
	  track[t]->makeProgTrack(false);     // revoke prog track special handling
	}
      track[trackToSet]->makeProgTrack(true); // set for prog track special handling
    } else {
      track[trackToSet]->makeProgTrack(false); // only the prog track knows it's type
    }
    trackMode[trackToSet]=mode;
    trackDCAddr[trackToSet]=dcAddr;
    
    // When a track is switched, we must clear any side effects of its previous 
    // state, otherwise trains run away or just dont move.

    // This can be done BEFORE the PWM-Timer evaluation (methinks)
    if (!(mode==TRACK_MODE_DC || mode==TRACK_MODE_DCX)) {
      // DCC tracks need to have set the PWM to zero or they will not work.
      track[trackToSet]->detachDCSignal();
      track[trackToSet]->setBrake(false);
    }

    // EXT is a special case where the signal pin is
    // turned off. So unless that is set, the signal
    // pin should be turned on
    track[trackToSet]->enableSignal(mode != TRACK_MODE_EXT);

#ifndef ARDUINO_ARCH_ESP32
    // re-evaluate HighAccuracy mode
    // We can only do this is all main and prog tracks agree
    bool canDo=true;
    FOR_EACH_TRACK(t) {
      // DC tracks must not have the DCC PWM switched on
      // so we globally turn it off if one of the PWM
      // capable tracks is now DC or DCX.
      if (trackMode[t]==TRACK_MODE_DC || trackMode[t]==TRACK_MODE_DCX) {
	if (track[t]->isPWMCapable()) {
	  canDo=false;    // this track is capable but can not run PWM
	  break;          // in this mode, so abort and prevent globally below
	} else {
	  track[t]->trackPWM=false; // this track sure can not run with PWM
	  //DIAG(F("Track %c trackPWM 0 (not capable)"), t+'A');
	}
      } else if (trackMode[t]==TRACK_MODE_MAIN || trackMode[t]==TRACK_MODE_PROG) {
	track[t]->trackPWM = track[t]->isPWMCapable(); // trackPWM is still a guess here
	//DIAG(F("Track %c trackPWM %d"), t+'A', track[t]->trackPWM);
	canDo &= track[t]->trackPWM;
      }
    }
    if (!canDo) {
      // if we discover that HA mode was globally impossible
      // we must adjust the trackPWM capabilities
      FOR_EACH_TRACK(t) {
	track[t]->trackPWM=false;
	//DIAG(F("Track %c trackPWM 0 (global override)"), t+'A');
      }
      DCCTimer::clearPWM(); // has to be AFTER trackPWM changes because if trackPWM==true this is undone for  that track
    }
#else
    // For ESP32 we just reinitialize the DCC Waveform
    DCCWaveform::begin();
#endif

    // This block must be AFTER the PWM-Timer modifications
    if (mode==TRACK_MODE_DC || mode==TRACK_MODE_DCX) {
        // DC tracks need to be given speed of the throttle for that cab address
        // otherwise will not match other tracks on same cab.
        // This also needs to allow for inverted DCX
        applyDCSpeed(trackToSet);
    }

    // Normal running tracks are set to the global power state 
    track[trackToSet]->setPower(
        (mode==TRACK_MODE_MAIN || mode==TRACK_MODE_DC || mode==TRACK_MODE_DCX || mode==TRACK_MODE_EXT) ?
        mainPowerGuess : POWERMODE::OFF);
    //DIAG(F("TrackMode=%d"),mode);
    return true; 
}

void TrackManager::applyDCSpeed(byte t) {
  uint8_t speedByte=DCC::getThrottleSpeedByte(trackDCAddr[t]);
  if (trackMode[t]==TRACK_MODE_DCX)
    speedByte = speedByte ^ 128; // reverse direction bit
  track[t]->setDCSignal(speedByte);
}

bool TrackManager::parseJ(Print *stream, int16_t params, int16_t p[])
{
    
    if (params==0) { // <=>  List track assignments
        FOR_EACH_TRACK(t)
            if (track[t]!=NULL) {
                StringFormatter::send(stream,F("<= %c "),'A'+t);
                switch(trackMode[t]) {
                    case TRACK_MODE_MAIN:
                        StringFormatter::send(stream,F("MAIN"));
			if (track[t]->trackPWM)
			  StringFormatter::send(stream,F("+"));
                        break;
                    case TRACK_MODE_PROG:
                        StringFormatter::send(stream,F("PROG"));
			if (track[t]->trackPWM)
			  StringFormatter::send(stream,F("+"));
                        break;
                    case TRACK_MODE_OFF:
                        StringFormatter::send(stream,F("OFF"));
                        break;
                    case TRACK_MODE_EXT:
                        StringFormatter::send(stream,F("EXT"));
                        break;
                    case TRACK_MODE_DC:
                        StringFormatter::send(stream,F("DC %d"),trackDCAddr[t]);
                        break;
                    case TRACK_MODE_DCX:
                        StringFormatter::send(stream,F("DCX %d"),trackDCAddr[t]);
                        break;
                    default:
                        break; // unknown, dont care    
                    }
                StringFormatter::send(stream,F(">\n"));
                }
        return true;
    }
    
    p[0]-=HASH_KEYWORD_A;  // convert A... to 0.... 

    if (params>1 && (p[0]<0 || p[0]>=MAX_TRACKS)) 
        return false;
    
    if (params==2  && p[1]==HASH_KEYWORD_MAIN) // <= id MAIN>
        return setTrackMode(p[0],TRACK_MODE_MAIN);
    
    if (params==2  && p[1]==HASH_KEYWORD_PROG) // <= id PROG>
        return setTrackMode(p[0],TRACK_MODE_PROG);
    
    if (params==2  && p[1]==HASH_KEYWORD_OFF) // <= id OFF>
        return setTrackMode(p[0],TRACK_MODE_OFF);

    if (params==2  && p[1]==HASH_KEYWORD_EXT) // <= id EXT>
        return setTrackMode(p[0],TRACK_MODE_EXT);

    if (params==3  && p[1]==HASH_KEYWORD_DC && p[2]>0) // <= id DC cab>
        return setTrackMode(p[0],TRACK_MODE_DC,p[2]);
    
    if (params==3  && p[1]==HASH_KEYWORD_DCX && p[2]>0) // <= id DCX cab>
        return setTrackMode(p[0],TRACK_MODE_DCX,p[2]);

    return false;
}

byte TrackManager::nextCycleTrack=MAX_TRACKS;

void TrackManager::loop() {
    DCCWaveform::loop(); 
    DCCACK::loop(); 
    bool dontLimitProg=DCCACK::isActive() || progTrackSyncMain || progTrackBoosted;
    nextCycleTrack++;
    if (nextCycleTrack>lastTrack) nextCycleTrack=0;
    if (track[nextCycleTrack]==NULL) return;
    MotorDriver * motorDriver=track[nextCycleTrack];
    bool useProgLimit=dontLimitProg? false: trackMode[nextCycleTrack]==TRACK_MODE_PROG;
    motorDriver->checkPowerOverload(useProgLimit, nextCycleTrack);   
}

MotorDriver * TrackManager::getProgDriver() {
    FOR_EACH_TRACK(t)
        if (trackMode[t]==TRACK_MODE_PROG) return track[t];
    return NULL;
} 

#ifdef ARDUINO_ARCH_ESP32
std::vector<MotorDriver *>TrackManager::getMainDrivers() {
  std::vector<MotorDriver *>  v;
  FOR_EACH_TRACK(t)
    if (trackMode[t]==TRACK_MODE_MAIN) v.push_back(track[t]);
  return v;
}
#endif

void TrackManager::setPower2(bool setProg,POWERMODE mode) {
    if (!setProg) mainPowerGuess=mode; 
    FOR_EACH_TRACK(t) {
        MotorDriver * driver=track[t]; 
        if (!driver) continue; 
        switch (trackMode[t]) {
            case TRACK_MODE_MAIN:
                if (setProg) break; 
                // toggle brake before turning power on - resets overcurrent error
                // on the Pololu board if brake is wired to ^D2.
		// XXX see if we can make this conditional
                driver->setBrake(true);
                driver->setBrake(false); // DCC runs with brake off
                driver->setPower(mode);  
#ifdef USE_HMI
              	if (hmi::CurrentInterface != NULL)
                {
                  if (mode == POWERMODE::ON)
                    hmi::CurrentInterface->DCCOn();
                  if (mode == POWERMODE::OFF)
                    hmi::CurrentInterface->DCCOff();
                }
#endif
                break; 
            case TRACK_MODE_DC:
            case TRACK_MODE_DCX:
                if (setProg) break; 
                driver->setBrake(true); // DC starts with brake on
                applyDCSpeed(t);        // speed match DCC throttles
                driver->setPower(mode);
                break;  
            case TRACK_MODE_PROG:
                if (!setProg) break; 
                driver->setBrake(true);
                driver->setBrake(false);
                driver->setPower(mode);
                break;  
            case TRACK_MODE_EXT:
	        driver->setBrake(true);
	        driver->setBrake(false);
		driver->setPower(mode);
	        break;
            case TRACK_MODE_OFF:
                break;
        }
    }
}
  
POWERMODE TrackManager::getProgPower() {
    FOR_EACH_TRACK(t)
        if (trackMode[t]==TRACK_MODE_PROG) 
                return track[t]->getPower();
    return POWERMODE::OFF;   
  }
   
void TrackManager::setJoinRelayPin(byte joinRelayPin) {
  joinRelay=joinRelayPin;
  if (joinRelay!=UNUSED_PIN) {
    pinMode(joinRelay,OUTPUT);
    digitalWrite(joinRelay,LOW);  // LOW is relay disengaged
  }
}

void TrackManager::setJoin(bool joined) {
#ifdef ARDUINO_ARCH_ESP32
  if (joined) {
    FOR_EACH_TRACK(t) {
      if (trackMode[t]==TRACK_MODE_PROG) {
	tempProgTrack = t;
	setTrackMode(t, TRACK_MODE_MAIN);
	break;
      }
    }
  } else {
    if (tempProgTrack != MAX_TRACKS+1) {
      setTrackMode(tempProgTrack, TRACK_MODE_PROG);
      tempProgTrack = MAX_TRACKS+1;
    }
  }
#endif
  progTrackSyncMain=joined;
  if (joinRelay!=UNUSED_PIN) digitalWrite(joinRelay,joined?HIGH:LOW);
}
