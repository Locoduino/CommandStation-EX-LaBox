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
#ifndef SProg_h
#define SProg_h

#include "DCC.h"
#include "EXComm.h"

#ifdef ENABLE_SPROG

// Uses the protocol SProg to receive orders from compatible software : Decoder_Pro from JMRi or DCCCenter...

/*******************************************************************
SPROG configuration Word : 

Bit		Name				Feature
0			UNLOCK			Unlock the firmware ready to receive an update via the
bootloader. This bit is not stored in EEPROM and is cleared
each time SPROG is reset
1			Reserved 		SPROG II echoes all received characters if this bit is set
2			Reserved		Do not use, always set to 0 for future compatibility
3			CALC_ERROR	Set to calculate error byte for O command. If clear then
error byte must be supplied on the command line
4 		RR_MODE			Set for rolling road/test mode
5			ZTC_MODE		SPROG II uses modified DCC timing for older ZTC decoders
6			BLUELINE		Modify direct mode programming algorithm to suit Blueline decoders
7			Reserved		Do not use, always set to 0 for future compatibility
8			DIR					Direction for rolling road/test mode and booster mode. Set for reverse
9			SP14				Select 14 speed step mode for rolling road/test mode and booster mode.
10		SP28				Select 28 speed step mode for rolling road/test mode and booster mode.
11		SP128				Select 128 speed step mode for rolling road/test mode and booster mode.
12		LONG				Use long addresses in rolling road/test mode and booster mode
13-15	Reserved	Do not use, always set to 0 for future compatibility 

this data is saved in EEPROM for ulterior usage.

*******************************************************************/

#define SPROG_IDENTIFIER	"SProg by Labox"
#define WORD_EEPROM_POS	400
#define WORD_EEPROM_DATA_POS	(WORD_EEPROM_POS+strlen(SPROG_IDENTIFIER) + 1)
#define SPROG_COMMAND_BUFFER_SIZE	50
#define SPROG_MAX_COMMAND_PARAMS	6

const bool DIAG_SPROG = true;

class SProg : public EXCommItem {
public:
	// EXCommItem part
	SProg(int inRxPin, int inTxPin);

	bool begin() override { setup(); return true; }
	bool loop() override { loopInternal(); return true; }

	void getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) override;

	// End EXCommItem

	struct SProgWord {
			bool Unlock;
			bool CalcError;
			bool RRMode;
			bool ZTCMode;
			bool BlueLine;
			bool Dir;
			bool Sp14;
			bool Sp28;
			bool Sp128;
			bool Long;
	};

	enum StateCV {
		Ready,	// Ready 
		Reading, // Waiting for Read result ...
		Reading_OK, // CV read without problem
		Reading_FAIL, // CV read WITH problem 
		Writing, // Waiting for end of writing
		Written_OK, // CV written without problem
		Written_FAIL // CV written WITH problem
	};

	static int rxPin;
	static int txPin;

	static StateCV	stateCV;

  static SProgWord word;
  static byte bufferLength;
  static byte buffer[SPROG_COMMAND_BUFFER_SIZE]; 

	static int test_cab;
	static int test_speed;
	static bool test_dir;
	static bool test_func[29];

  static void setup();
	static void loopInternal();
	static void parse(byte *command);
	static int16_t splitValues(int16_t result[SPROG_MAX_COMMAND_PARAMS], const byte *cmd);

	static void ReadWord();
	static void WriteWord();
	static int16_t GetWord();
	static void SetWord(int16_t inNewWord);

  static void printAll(Print *stream);
}; // SProg

#endif
#endif
