/*
 *  © 2023 Thierry Paris

 *  All rights reserved.
 *  
 *  This file is part of CommandStation-EX-LaBox
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

/**********************************************************************
SPROG hardware communicate through serial connection (USB) with software
using a protocol.
Copy of the SPROG user manual Version 1.4 Nov 2011 © Copyright 2008-2011 SPROG DCC
SPROG II User Guide for DecoderPro 2.12 64 :

The virtual COM port should be set for 8 bits, no parity, one stop bit at a
speed of 9600 baud. SPROG does not echo characters that are sent to it. All
commands must be entered on a single line terminated by carriage return.
Maximum input line length is 64 characters, including carriage return. Format
of parameters is dependent upon the command. The maximum number of
parameters on any line is 6.

General Commands
	M [n] – Display [Set] operating mode
	R – Read mode from EEPROM
	S – Display Status
	W – Write mode to EEPROM
	Z [n] – ZTC Compatibility Mode
	? - Display Help
	ESC - immediately shutdown power to track

Programmer Commands
	C CV [Val] - Read [program] CV using direct bit mode
	V CV [Val] - Read [program] CV using paged mode

Rolling Road Tester Commands
	A [n] – Display [Set] Address
	O byte [byte] [byte] [byte] - Output bytes as DCC packet.
	+ - Track power on
	- - Track power off
	<[step | <] - Reverse speed step[s]
	>[step | >] - Forward speed step[s]

Bootloader Command
	B a b c – Start Bootloader

Input Format
Input values are always parsed as decimal, unless overridden with 'b' or 'h'
prefix for binary or hexadecimal, respectively. E.g. h15 is equivalent to 21
decimal. The O command is an exception to this rule

**********************************************************************/

#include "SProg.h"
#include "EEPROM.h"
#include "EEStore.h"
#include "TrackManager.h"
#include "DCCWaveform.h"
#include "EXComm.h"

#ifdef ENABLE_SPROG

SProg::SProgWord SProg::word;
byte SProg::bufferLength = 0;
byte SProg::buffer[SPROG_COMMAND_BUFFER_SIZE]; 
SProg::StateCV	SProg::stateCV;

int SProg::rxPin = 128;
int SProg::txPin = 128;

int SProg::test_cab;
int SProg::test_speed;
bool SProg::test_dir;
bool SProg::test_func[];

int SPROGCvValue = -1;
int SPROGCvAddress = 0;

SProg::SProg(int inRxPin, int inTxPin) : EXCommItem("SPROG") {
	rxPin = inRxPin;
	txPin = inTxPin;
	Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);

	this->MainTrackEnabled = false;
	this->ProgTrackEnabled = true;
	this->AlwaysLoop = true;
}

void SProg::setup() {
 	DIAG(F("[SPROG] Serial1 Txd:%d   Rxd:%d"), txPin, rxPin);
	test_cab = 3;
	test_speed = 0;
	test_dir = true;
	for (int i = 0; i < 28; i++)
		test_func[i] = false;
}

void SProg::ReadWord() {
	String id = EEPROM.readString(WORD_EEPROM_POS);
	if (id == SPROG_IDENTIFIER)
		EEPROM.get(WORD_EEPROM_DATA_POS, SProg::word);
	else {
		SProg::word.Unlock = false;
		SProg::word.CalcError = false;
		SProg::word.RRMode = false;
		SProg::word.ZTCMode = false;
		SProg::word.BlueLine = false;
		SProg::word.Dir = true;
		SProg::word.Sp14 = false;
		SProg::word.Sp28 = false;
		SProg::word.Sp128 = true;
		SProg::word.Long = false;
	}

	// Should always be zero after loading.
	SProg::word.Unlock = false;
}

void SProg::WriteWord() {
	EEPROM.writeString(WORD_EEPROM_POS, SPROG_IDENTIFIER);
	EEPROM.put(WORD_EEPROM_DATA_POS, SProg::word);
}

int16_t SProg::GetWord() {
	int16_t localWord = 0;

	localWord += SProg::word.Unlock ? 1 : 0;
	localWord += SProg::word.CalcError ? 1 << 3 : 0;
	localWord += SProg::word.RRMode ? 1 << 4 : 0;
	localWord += SProg::word.ZTCMode ? 1 << 5 : 0;
	localWord += SProg::word.BlueLine ? 1 << 6 : 0;
	localWord += SProg::word.Dir ? 1 << 8 : 0;
	localWord += SProg::word.Sp14 ? 1 << 9 : 0;
	localWord += SProg::word.Sp28 ? 1 << 10 : 0;
	localWord += SProg::word.Sp128 ? 1 << 11 : 0;
	localWord += SProg::word.Long ? 1 << 12 : 0;

	return localWord;
}

void SProg::SetWord(int16_t inNewWord) {
	SProg::word.Unlock = inNewWord & 1 << 0;
	SProg::word.CalcError = inNewWord &  1 << 3;
	SProg::word.RRMode = inNewWord & 1 << 4;
	SProg::word.ZTCMode = inNewWord & 1 << 5;
	SProg::word.BlueLine = inNewWord & 1 << 6;
	SProg::word.Dir = inNewWord & 1 << 8;
	SProg::word.Sp14 = inNewWord & 1 << 9;
	SProg::word.Sp28 = inNewWord & 1 << 10;
	SProg::word.Sp128 = inNewWord & 1 << 11;
	SProg::word.Long = inNewWord & 1 << 12;
}

///////////////////////////////////////////////////////////////////////////////
//
// prints word status to stream
//
///////////////////////////////////////////////////////////////////////////////

void SProg::printAll(Print *stream) {
}

void SProg::getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) 
{
	char mess[maxSize*2];

	sprintf(mess, "[SPROG] Serial1");
	*pMess1 = mess;

	sprintf(mess, "[SPROG] Tx:%d Rx:%d", txPin, rxPin);
	*pMess2 = mess;
}

#include "StringFormatter.h"

#define BROADCAST(args...)		StringFormatter::send(&Serial1, args)

///////////////////////////////////////////////////////////////////////////////
// Object method to directly change the input state, for sensors such as LCN which are updated
//  by means other than by polling an input.

char buffer[256];

void SProg::loopInternal() {

	if (stateCV == StateCV::Reading || stateCV == StateCV::Writing)
		return;

	if (stateCV == StateCV::Reading_OK) {
		BROADCAST(F("= %x"), SPROGCvValue);
		stateCV = StateCV::Ready;
	}

	if (stateCV == StateCV::Reading_FAIL) {
		BROADCAST(F("= !E")); 
		stateCV = StateCV::Ready;
	}

	if (stateCV == StateCV::Written_OK) {
		BROADCAST(F("= %d %x"), SPROGCvAddress, SPROGCvValue); 
		stateCV = StateCV::Ready;
	}

	if (stateCV == StateCV::Written_FAIL) {
		BROADCAST(F("= !E")); 
		stateCV = StateCV::Ready;
	}

	while (Serial1.available()) {
			char ch = Serial1.read();
			//Serial.println((int)ch);
			if (ch < 0x20) {
					buffer[bufferLength] = '\0';
					SProg::parse(buffer); 
					if (DIAG_SPROG && buffer[0] != 'S') DIAG(F("[SPROG] << %s"), buffer);
					bufferLength = 0;
					buffer[0] = '\0';
					break;
			}

			if (bufferLength < (SPROG_COMMAND_BUFFER_SIZE-1)) buffer[bufferLength++] = ch;
	}
}

enum NumberType {
	binary,
	hexa,
	decimal
};

int16_t SProg::splitValues(int16_t result[SPROG_MAX_COMMAND_PARAMS], const byte *cmd)
{
    byte state = 1;
    byte parameterCount = 0;
    int16_t runningValue = 0;
    const byte *remainingCmd = cmd + 1; // skips the opcode
    bool signNegative = false;
		NumberType numberType = NumberType::decimal;
		int16_t binaryValueLength = 0;
		char binaryValue[17];

    // clear all parameters in case not enough found
    for (int16_t i = 0; i < SPROG_MAX_COMMAND_PARAMS; i++)
        result[i] = 0;

    while (parameterCount < SPROG_MAX_COMMAND_PARAMS)
    {
        byte hot = *remainingCmd;

        switch (state)
        {
        case 1: // skipping spaces before a param
						numberType = cmd[0] == 'O' ? NumberType::hexa : NumberType::decimal;
            if (hot == ' ')
                break;
            if (hot == '\0' || hot == '>')
                return parameterCount;
            state = 2;
            continue;

        case 2: // checking sign
            signNegative = false;
            runningValue = 0;
            state = 3;
            if (hot != '-')
                continue;
            signNegative = true;
            break;

        case 3: // building a parameter
						if (hot == 'b') {
							numberType = NumberType::binary;
							break;
						}
						if (hot == 'h') {
							numberType = NumberType::hexa;
							break;
						}

						if (numberType == NumberType::binary) {						
							if (hot >= '0' && hot <= '1')	{
								binaryValue[binaryValueLength++] = hot;
								break;
							}

	    	    	for (int16_t pos = binaryValueLength - 1; pos >= 0; pos--)
  	    	    	if (binaryValue[pos] == '1')
    	    	    	runningValue += 1 << ((binaryValueLength - 1) - pos);

	            result[parameterCount] = runningValue * (signNegative ? -1 : 1);
  	          parameterCount++;
    	        state = 1;
      	      continue;
						}

            if (hot >= '0' && hot <= '9')
            {
                runningValue = (numberType == NumberType::hexa ? 16 : 10) * runningValue + (hot - '0');
                break;
            }
            if (hot >= 'a' && hot <= 'z') hot=hot-'a'+'A'; // uppercase a..z
            if (numberType == NumberType::hexa && hot>='A' && hot<='F') {
                // treat A..F as hex not keyword
                runningValue = 16 * runningValue + (hot - 'A' + 10);
                break;
            }
            if (hot=='_' || (hot >= 'A' && hot <= 'Z'))
            {
                // Since JMRI got modified to send keywords in some rare cases, we need this
                // Super Kluge to turn keywords into a hash value that can be recognised later
                runningValue = ((runningValue << 5) + runningValue) ^ hot;
                break;
            }
            result[parameterCount] = runningValue * (signNegative ? -1 : 1);
            parameterCount++;
            state = 1;
            continue;
        }
        remainingCmd++;
    }
    return parameterCount;
}

void SPROGcvValueCallback(int16_t inValue) {
	if (inValue >= 0)
		SProg::stateCV = SProg::StateCV::Reading_OK;
	else
		SProg::stateCV = SProg::StateCV::Reading_FAIL;

	if (DIAG_SPROG) 
  	DIAG(F("[SPROG] read cv %d callback = %d !"), SPROGCvAddress, inValue);
		
  SPROGCvValue = inValue;
}

void SPROGcvWriteValueCallback(int16_t inValue)
{
	if (inValue >= 0)
		SProg::stateCV = SProg::StateCV::Written_OK;
	else
		SProg::stateCV = SProg::StateCV::Written_FAIL;
}

void SProg::parse(byte *com) {
//    Serial.print((char *) com);

    int16_t p[SPROG_MAX_COMMAND_PARAMS];
    while (/*com[0] == '<' || */com[0] == ' ')
        com++; // strip off any number of < or spaces
    byte paramCount = splitValues(p, com);

    switch(com[0])
    {
      case 'M': // Get/Set Word
				if (paramCount == 0) {
					BROADCAST(F("M= %d"), GetWord());
				}
				else {
					if (paramCount == 1)
						SetWord(p[0]);
				}
        break;
      case 'R': // Read word from EEPROM
				ReadWord();
        break;
      case 'S':	// Send status
				BROADCAST(F("OK"));
        break;
      case 'W': // Write word to EEPROM
				WriteWord();
        break;
      case 'Z':	// Set special DCC timing
				// Not implemented
					if (paramCount == 1 && p[0] == 1)
						DIAG(F("[SPROG] Invalid command Z, not implemented"));
        break;
      case '?': // Get SPROG version
        BROADCAST(F("SPROG LABOX USB Ver 2.5.0"));
        break;
      case 27: // Immediately cut power.
				TrackManager::setProgPower(POWERMODE::OFF);
        break;
      case ' ': // Send ' '
        BROADCAST(F(" "));
        break;
      case 'C':	// Read / write CV !
        if (paramCount == 1) {
					SPROGCvAddress = p[0];
					SPROGCvValue = -1;
					void (*ptr)(int16_t) = &SPROGcvValueCallback;
					DCC::readCV(SPROGCvAddress, ptr);
					stateCV = Reading;
				}
				else {
					if (paramCount == 2) {
						SPROGCvAddress = p[0];
						SPROGCvValue = p[1];
						void (*ptr)(int16_t) = &SPROGcvWriteValueCallback;
						DCC::writeCVByte(SPROGCvAddress, SPROGCvValue, ptr);
						stateCV = Writing;
					}
				}
				break;
				case '+':
					TrackManager::setProgPower(POWERMODE::ON);
					break;
				case '-':
					TrackManager::setProgPower(POWERMODE::OFF);
					break;
				case 'A':
					test_cab = p[0];
					break;
				case 'O':
					{
						byte params[10];
						Serial.print("O ");
						for (int i = 0; i < paramCount; i++)
						{
							params[i] = (byte) p[i];
							Serial.print((int) params[i]);
							Serial.print(" ");
						}
						Serial.println("");

						DCCWaveform::progTrack.schedulePacket(params, paramCount, 0);
					}
					break;
				case '<':
					test_dir = false;
					DCC::setThrottle(test_cab, p[0], test_dir);
					break;
				case '>':
					test_dir = true;
					DCC::setThrottle(test_cab, p[0], test_dir);
					break;
				default:
					Serial.print(com[0]);
					DIAG(F("[SPROG] %s : Invalid command, not implemented"), com[0]);
					break;
   }
}

#endif