/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle base class>
*************************************************************/

#include "Arduino.h"
#include "DCCEX.h"
#include "DCCEXParser.h"
#include "Throttle.hpp"
#include "Throttles.hpp"

#if defined(USE_THROTTLES)
Throttle::Throttle(const String& inName, unsigned int inTimeOutDelay)
{
	// 'static' data, not updated during the run.
	this->name = inName;
	this->timeOutDelay = inTimeOutDelay;
	this->replyToCommands = false;
	this->pBuffer = NULL;
	this->type = NOTSTARTEDTHROTTLE;
	this->id = 0;

	// Default start/end characters for DCC++ syntax commands
	this->startCommandCharacter = '<';
	this->endCommandCharacter = '>';

	// Variable data
	this->commandString[0] = 0;
	this->contacted = false;
	this->lastActivityDate = 0;
	this->pNextThrottle = NULL;

	Throttles::add(this);
}

// If the message starts with '<', this is a classic DCC++ message. Otherwise, this is a throttle message with the following syntax :
// DDDSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
// Where DDD is the numerical form of the Throttle Id which have got the message, in order to answer to this throttle only.
// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSS is the classic DCC++ message received by the throttle.
void Throttle::pushMessageInStack(uint16_t inThrottleId, const String& inMessage)
{
	char buffer[MESSAGE_MAXSIZE];

	buffer[0] = '0' + inThrottleId / 100;
	buffer[1] = '0' + (inThrottleId - ((inThrottleId / 100) * 100)) / 10;
	buffer[2] = '0' + inThrottleId % 10;
	buffer[3] = 0;
	strncpy(buffer + 3, inMessage.c_str(), MESSAGE_MAXSIZE - 3);
	MessageStack::MessagesStack.PushMessage(buffer);
}

const String& Throttle::getMessageFromStackMessage(const String& inMessage, String &inToReturn)
{
	inToReturn = inMessage.substring(3);

	return inToReturn;
}

Throttle* Throttle::getThrottleFromStackMessage(const String& inMessage)
{
	uint16_t id = 0;

	id = ((inMessage[0] - '0') * 100) + ((inMessage[1] - '0') * 10) + (inMessage[2] - '0');

	return Throttles::get(id);
}

bool Throttle::getCharacter(char inC)
{
	if (this->type == NOTSTARTEDTHROTTLE)
		return false;

	//Serial.println((int)inC);
	if (inC == (char)this->startCommandCharacter)                    // start of new command
	{
		this->commandString[0] = 0;
	}
	else
	{
#ifdef DCCPP_DEBUG_MODE
		if (this->endCommandCharacter > 31 && inC <= 13)		// message, not command !
		{
			Serial.print(this->id);
			Serial.print(" Message From Throttle : ");
			Serial.println(this->commandString);
		}
#endif

		if (inC == (char)this->endCommandCharacter)               // end of new command
		{
#ifdef DCCPP_DEBUG_MODE
			Serial.print(this->id);
			Serial.print(" From Throttle : ");
			Serial.println(this->commandString);
#endif
			this->pushMessage(this->commandString);
			this->commandString[0] = 0;
			return true;
		}
		else
		{
			if (inC >= ' ' && strlen(this->commandString) < THROTTLE_COMMAND_SIZE)    // if commandString still has space, append character just read from network
				sprintf(this->commandString, "%s%c", this->commandString, inC);     // otherwise, character is ignored (but continue to look for start or end characters)}
		}
	}

	return false;
}

void Throttle::setCommandCharacters(int inStartCharacter, int inEndCharacter)
{
	this->startCommandCharacter = inStartCharacter;
	this->endCommandCharacter = inEndCharacter;
}

bool Throttle::SendNewline() const
{
	return true;
}

bool Throttle::processBuffer()
{
	bool ret = false;
	if (ret)
	{
		if (this->timeOutDelay != 0)
			this->lastActivityDate = millis();
	}
	else
	{
		if (this->lastActivityDate != 0)
		{
			if (!Throttles::AreTimeoutsSuspended())
			{
				if (millis() - this->lastActivityDate > this->timeOutDelay)
				{
					this->end();
#ifdef DCCPP_DEBUG_MODE
					Serial.print(this->id);
					Serial.println(" disconnected !");
					Throttles::printThrottles();
#endif
				}
			}
		}
	}
	return ret;
}

bool Throttle::pushMessage(const String& inpCommand)
{
	this->pushMessageInStack(this->id, inpCommand);
	if (this->timeOutDelay != 0)
		this->lastActivityDate = millis();

	return true;
}

bool Throttle::pushDccppMessage(const String& inpCommand)
{
	char comm[MESSAGE_MAXSIZE];

	comm[0] = '<';
	comm[1] = 0;
	strncpy(comm + 1, inpCommand.c_str(), MESSAGE_MAXSIZE - 2);
	comm[1 + inpCommand.length()] = '>';
	comm[2 + inpCommand.length()] = 0;

	return Throttle::pushMessage(comm);
}

void Throttle::end()
{
	this->commandString[0] = 0;

	this->startCommandCharacter = '<';
	this->endCommandCharacter = '>';

	this->contacted = false;
	this->lastActivityDate = 0;
	if (this->pBuffer != NULL)
		this->pBuffer->clear();
}

#ifdef VISUALSTUDIO
SERIAL_INTERFACE(Serial, Test);
const char* ssidTest = "VIDEOFUTUR_C56165_2.4G";
const char* passwordTest = "EenPghQD";
// the media access control (ethernet hardware) address for the shield:
//uint8_t MacTest[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
//uint8_t IpTest[] = { 192, 168, 1, 100 };

void Throttle::test()
{
#ifdef USE_TEXTCOMMAND
	ThrottleSerial throttleSerial("TestSerial 12", new SerialInterfaceTest());
	throttleSerial.setId(12);
#endif

#if !defined(ARDUINO_ARCH_ESP32)
#if USE_ETHERNET
	ThrottleEthernet throttleEthernet("TestEthernet 9", MacTest, IpTest, 2560, TCP);
	throttleEthernet.setId(9);
#endif
#endif

#ifdef USE_WIFI
	ThrottleWifi throttleWifi("TestWifi 567", 2560);
	throttleWifi.begin(TCP);
	throttleWifi.setId(567);
#endif

	Throttles::printThrottles();

	Throttle::pushMessageInStack(0, "Throttle serial !");
	Throttle::pushMessageInStack(9, "Throttle ethernet !");
	Throttle::pushMessageInStack(567, "Throttle wifi !");
	Throttle::pushMessageInStack(12, "Throttle serial !");

	MessageStack::MessagesStack.printStack();

	char buffer[MESSAGE_MAXSIZE];
	while (true)
	{
		int index = MessageStack::MessagesStack.GetPendingMessageIndex();

		if (index == 255)
			break;

		MessageStack::MessagesStack.GetMessage(index, buffer);

		Throttle* throttle = Throttle::getThrottleFromStackMessage(buffer);
		String message;
		Throttle::getMessageFromStackMessage(buffer, message);

		Serial.print(throttle->getName());
		Serial.print(" : ");
		Serial.println(message);
	}
}
#endif

#ifdef DCCPP_DEBUG_MODE
void Throttle::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print(this->name);

	Serial.println("");
}
#endif
#endif