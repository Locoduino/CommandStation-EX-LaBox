/*
 * LaBox Project
 * menuDecoderIdentification Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuDecoderIdentification.h"
#include "hmi.h"
#include "LaboxModes.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

const char *countries[] = { NULL, "ARG", "AT", "AU", "AUS", "BE",	// 0 - 5
												 "BRA", "CA", "CH", "CN", "CZE",					// 6 - 10
												 "DE", "DK", "ESP", "FR", "HKG",					// 11 - 15
												 "HUN", "JP", "POL", "SE", "TWN",					// 16 - 20
												 "UK", "US", "NL", "ROM", "ZAF"};					// 21 - 25

struct brandItem
{
	char brand[21];
	uint8_t countryIndex;
};

const brandItem NMRA922[] =
{
	{ "", 0 },			// 0
	{ "CML", 21 },
	{ "Train Technology", 5 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },			// 5
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },			// 10
	{ "NCE Corporation", 22 },
	{ "Wangrow", 22},
	{ "Public Domain/DIY", 0},
	{ "PSI/Dynatrol", 55},
	{ "Ramfixx", 7},	// 15
	{ "", 0 },
	{ "Advance IC Eng.", 22},
	{ "JMRI", 22},
	{ "AMW", 2},
	{ "T4T Tech. for Trains", 11},		// 20
 	{ "Kreischer", 11},
	{ "KAM Industries", 22},
	{ "S Helper Service", 22},
	{ "MoBaTron.de", 11},
	{ "Team Digital, LLC", 22},		// 25
	{ "MBTronik/PiN GITmBH", 11},
	{ "MTH Electric Trains", 22},
	{ "Heljan A/S", 12},
	{ "Mistral Train Models", 5},
	{ "Digsight", 7},					 	// 30
	{ "Brelec", 5},
	{ "Regal Way Co. Ltd", 15},
	{ "Praecipuus", 7},
	{ "Aristo-Craft Trains" , 22},
	{ "Elec. & Model Prod.", 19},			// 35
	{ "DCCconcepts", 3},
	{ "NAC Services, Inc", 22},
	{ "Broadway Limited Imp", 22},
	{ "Educational Computer", 22},
	{ "KATO", 17}, 										// 40
	{ "Passmann", 11},
	{ "Digikeijs", 23},
	{ "Ngineering", 22},
	{ "SPROG-DCC", 21},
	{ "ANE Model Co, Ltd", 20},				//45
	{ "GFB Designs", 21},
	{ "Capecom", 3},
	{ "Hornby Hobbies Ltd", 21},
	{ "Joka Electronic", 11},
	{ "N&Q Electronics", 13},		 			//50
	{ "DCC Supplies, Ltd", 21},
	{ "Krois-Modell", 2},
	{ "Rautenhaus Digital", 11},
	{ "TCH Technology", 22},
	{ "QElectronics", 11},			 			//55
	{ "LDH", 1},
	{ "Rampino", 11},
	{ "KRES", 11},
	{ "Tam Valley Depot", 22},
	{ "Bluecher-Electronic", 11},			//60
	{ "TrainModules", 16},
	{ "Tams Elektronik", 11},
	{ "Noarail", 4},
	{ "Digital Bahn", 11},
	{ "Gaugemaster", 21},		  				//65
	{ "Railnet Sol. LLC", 22},
	{ "Heller Modenlbahn", 11},
	{ "MAWE Elektronik", 8},
	{ "E-Modell", 11},
	{ "Rocrail", 11},			 						//70
	{ "New York Byano", 15},
	{ "MTB Model", 10},
	{ "Electric Railroad", 22},
	{ "PpP Digital", 13},
	{ "Digitools", 16},								//75
	{ "Auvidel", 11},
	{ "LS Models Sprl", 5},
	{ "train-O-matic",24},
	{ "Hattons", 21},
	{ "Spectrum", 22},						 		//80
	{ "GooVerModels", 5},
	{ "HAG Modelleisenbahn", 8},
	{ "JSS-Elektronic", 11},
	{ "Railflyer", 7},
	{ "Uhlenbrock", 11},							//85
	{ "Wekomm", 11},
	{ "RR-Cirkits", 22},
	{ "HONS Model", 15},
	{ "Pojezdy.EU", 10},
	{ "Shourt Line", 22},							//90
	{ "Railstars Ltd", 22},
	{ "Tawcrafts", 21},
	{ "Kevtronics", 25},
	{ "Electroniscript", 22},
	{ "Sanda Kan Industrial", 15},		//95
	{ "PRICOM Design", 22},
	{ "Doehler & Haas", 11},
	{ "Harman DCC", 21},
	{ "Lenz", 11},
	{ "Trenes Digitales", 1},					//100
	{ "Bachmann", 22},
	{ "Int. Signal Sys.", 22},
	{ "Nagasue", 17},
	{ "TrainTech", 23},
	{ "Computer Dialysis", 14},	//105
	{ "Opherline1", 14},
	{ "Phoenix Snd Systems", 22},
	{ "Nagoden", 17},
	{ "Viessmann", 11},
	{ "AXJ Electronics", 9},					//110
	{ "Haber & Koenig", 2},
	{ "LSdigital", 11},
	{ "QS Industries", 22},
	{ "Benezan Electronics", 13},
	{ "Dietz Model.", 11},	//115
	{ "MyLocoSound", 4},
	{ "cT Elektronik", 2},
	{ "MÜT", 11},
	{ "W. S. Ataras", 22},
	{ "csikos-muhely", 16},						//120
	{ "", 0 },
	{ "Berros", 23},
	{ "Massoth", 11},
	{ "DCC-Gaspar-Elec.", 16},
	{ "ProfiLok", 11},	//125
	{ "Möllehem", 19},
	{ "Atlas", 22},
	{ "Frateschi", 6},
	{ "Digitrax", 22},
	{ "cmOS", 4},					//130
	{ "Trix", 11},
	{ "ZTC", 21},
	{ "ICC", 22},
	{ "LaisDCC", 9},
	{ "CVP Products", 22},						//135
	{ "NYRS", 22},
	{ "", 0 },
	{ "Train ID Systems", 22},
	{ "RealRail Effects", 22},
	{ "Desktop Station", 17},					//140
	{ "Soundtraxx", 22},
	{ "SLOMO", 17},
	{ "MRC", 22},
	{ "DCC Train Automation", 21},
	{ "Zimo", 2},					//145
	{ "Rails Europ Express", 14},
	{ "Umelec Ing. Buero", 8},
	{ "BLOCKsignalling", 21},
	{ "Rock Junc. Controls", 22},
	{ "Wm. K. Walthers", 22},		//150
	{ "Electronic Solutions", 11},
	{ "Digi-CZ", 10},
	{ "TCS", 22},
	{ "Dapol", 21},
	{ "Fleischmann", 11},							//155
	{ "Nucky", 17},
	{ "Kuehn", 11},
	{ "Fucik", 10},
	{ "LGB", 11},
	{ "MD Electronics", 11},					//160
	{ "Modelleisenbahn/Roco", 2},
	{ "PIKO", 11},
	{ "WP Railshops", 7},
	{ "drM", 20},
	{ "MERG", 21},	//165
	{ "Maison de DCC", 17},
	{ "Helvest Systems", 8},
	{ "Model Train Tech.", 22},
	{ "AE Electronic", 9},
	{ "AuroTrains", 22},							//170
	{ "", 0 },
	{ "", 0 },
	{ "Arnold/Rivarossi", 11},
	{ "", 0 },
	{ "", 0 },												//175
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												
	{ "", 0 },												//180
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//185
	{ "BRAWA", 11},
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//190
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//195
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//200
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "Con-Com", 2},
	{ "", 0 },												//205
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//210
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//215
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//220
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "Blue Digital", 18},						//225
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//230
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },
	{ "", 0 },												//235
	{ "", 0 },
	{ "", 0 },
	{ "NMRA Reserved", 22}
};

int identCV8Value = -1;
int identCV7Value = -1;
bool identDisplayInProgress;
char identMessage[21];

enum StateIdent
{
	//																									Select								Up								Down
	Reading7,				// Waiting for Read cv7 result...		--					--				--
	Reading7Ok,			// Reading 7 finished and ok.				--					--				--
	Reading7Error,	// Reading 7 finished and failed.		--					--				--
	Reading8,				// Waiting for Read cv8 result...		--					--				--
	Reading8Ok,			// Reading 8 finished and ok.				--					--				--
	Reading8Error,	// Reading 8 finished and failed.		--					--				--
	MenuRetry, 			// Select retry/quit								retry		option retry	--
	MenuQuit, 			// Select retry/quit								quit		option retry	--
};

StateIdent	identState;

void diagPrint()
{
	switch(identState)
	{
		case Reading7:					DIAG(F("Reading7"));			break;
		case Reading7Ok:				DIAG(F("Reading7Ok %d"), identCV7Value);			break;
		case Reading7Error:			DIAG(F("Reading7Error"));			break;
		case Reading8:					DIAG(F("Reading8"));			break;
		case Reading8Ok:				DIAG(F("Reading8Ok %d"), identCV8Value);			break;
		case Reading8Error:			DIAG(F("Reading8Error"));			break;
		case MenuRetry:					DIAG(F("MenuRetry"));			break;
		case MenuQuit:					DIAG(F("MenuQuit"));			break;
	}
}

#define DIAGIDENT(FCT)		diagPrint()

void cvValueIdentCallback(int16_t inValue)
{
	int cv = 0;
	if (identState == Reading7)
	{
		cv = 7;
		if (inValue >= 0)
		{
			identCV7Value = inValue;
			identState = Reading7Ok;
		}
		else
			identState = Reading7Error;
	}

	if (identState == Reading8)
	{
		cv = 8;
		if (inValue >= 0)
		{
			identCV8Value = inValue;
			identState = Reading8Ok;
		}
		else
			identState = Reading8Error;
	}

	DIAGIDENT();
  DIAG(F("cvValueCallback called %d = %d !"), cv, inValue);
  identDisplayInProgress = true;
}

void menuDecoderIdentification::start()
{
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::start.. Begin"); 

  if (!LaboxModes::progMode) {
		LaboxModes::Restart(IDENTIFY);
  }
  
  identState = StateIdent::Reading7;
  identCV7Value = -1;
  void (*ptr)(int16_t) = &cvValueIdentCallback;
	DCC::readCV(7, ptr);
	identDisplayInProgress = true;
	this->update();

	// wait until Ok or error...
	while (identState == Reading7)
		DCC::loop();

	if (identState == Reading7Ok)
	{
		DIAG("start read cv8");
		identState = StateIdent::Reading8;
		identDisplayInProgress = true;
		this->update();
		identCV8Value = -1;
		void (*ptr)(int16_t) = &cvValueIdentCallback;
		DCC::readCV(8, ptr);
	}

	identDisplayInProgress = true;

	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::start.. End"); 
}

/*!
    @brief  menuTrainCvRead Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuDecoderIdentification::menuDecoderIdentification(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
	resetMenu();
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuDecoderIdentification::eventUp()
{
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventUp.. Begin");
	DIAGIDENT("up begin");

	menuObject::eventUp();

	if (identState == MenuQuit)
	{
		identState = MenuRetry;
	  identDisplayInProgress = true;
	}

	DIAGIDENT("up end");
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuDecoderIdentification::eventDown()
{
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventDown.. Begin"); 
	DIAGIDENT("down begin");
	menuObject::eventDown();

	if (identState == MenuRetry)
	{
		identState = MenuQuit;
    identDisplayInProgress = true;
	}
	DIAGIDENT("down end");
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuDecoderIdentification::eventSelect()
{
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventSelect.. Begin"); 
	DIAGIDENT("select begin");

	if (identState == MenuRetry)
	{
		this->start();
  }
	else
		if (identState == MenuQuit)
		{
			LaboxModes::Restart(SILENTRETURNTOMAIN);
		}

	DIAGIDENT("select end");
	_HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::eventSelect.. End");  
	return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuDecoderIdentification::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::begin.. End"); 
}

void centerIdentMessage(const char *Text)
{
	int len = strlen(Text);
	memset(identMessage, ' ', 20);
	memcpy(identMessage + (10-(len/2)), Text, len);
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuDecoderIdentification::update()
{
  _HMIDEBUG_FCT_PRINTLN(F("menuDecoderIdentification::update.. Begin")); 
	char buffer[21];

  if(!identDisplayInProgress)
  {
		return;
	}

  display->clearDisplay();
  identDisplayInProgress = false;  

  display->setTextSize(1);
  display->setCursor(5, 6);
  display->println(TXT_IDENT_LOGO);

  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(5, 20);
	switch (identState)
	{
		case Reading7Error:
    	centerIdentMessage(TXT_IDENT_ERRORCV7);
		  display->println(identMessage);
			identState = MenuRetry;
			break;
		case Reading8Error:
    	sprintf(buffer,TXT_IDENT_ERRORCV8, identCV7Value);
    	centerIdentMessage(buffer);
		  display->println(identMessage);
			identState = MenuRetry;
			break;
		case Reading8Ok:
			if (strlen(NMRA922[identCV8Value].brand)==0)
			{
	    	sprintf(buffer, TXT_IDENT_UNKNOWNID, identCV8Value);				
	    	centerIdentMessage(buffer);
		  	display->println(identMessage);

			  display->setCursor(5, 29);
	    	centerIdentMessage(TXT_IDENT_SEENMRA);				
		  	display->println(identMessage);
			}
			else
			{
		  	centerIdentMessage(NMRA922[identCV8Value].brand);
		  	display->println(identMessage);
			  display->setCursor(5, 29);
	    	sprintf(buffer, "(%s)", countries[NMRA922[identCV8Value].countryIndex]);
	    	centerIdentMessage(buffer);
		  	display->println(identMessage);
			  display->setCursor(5, 38);
	    	sprintf(buffer, TXT_IDENT_VERSION, identCV7Value, identCV8Value);
	    	centerIdentMessage(buffer);
		  	display->println(identMessage);
			}
			identState = MenuQuit;
			break;

		/*default:
	    sprintf(buffer, "%d", (int) identState);
	    centerIdentMessage(buffer);
		  display->println(identMessage);
			break;*/
	}

  display->setTextSize(1);
  display->setCursor(5, 55);
	switch(identState)
	{
		case Reading7:
			display->println(TXT_IDENT_READING7);
			break;

		case Reading8:
			display->println(TXT_IDENT_READING8);
			break;

		case MenuRetry:
			sprintf(identMessage,">%s<",TXT_MenuAddrRetry);
			display->println(identMessage);
			display->setCursor(64, 55);
			sprintf(identMessage," %s ",TXT_MenuAddrQuit);
			display->println(identMessage);
			break;

		case MenuQuit:
			sprintf(identMessage," %s ",TXT_MenuAddrRetry);
			display->println(identMessage);
			display->setCursor(64, 55);
			sprintf(identMessage,">%s<",TXT_MenuAddrQuit);
			display->println(identMessage);
			break;
	}

  display->display();   

  _HMIDEBUG_FCT_PRINTLN(F("menuDecoderIdentification::update.. End")); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuDecoderIdentification::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::resetMenu.. Begin"); 

  menuObject::resetMenu();

  identDisplayInProgress = false;

  _HMIDEBUG_FCT_PRINTLN("menuDecoderIdentification::resetMenu.. End"); 
  
}
#endif