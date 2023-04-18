#include "MessageStack.hpp"

MessageStack MessageStack::MessagesStack;

void MessageStack::begin(bool inMultiThread)
{
#ifdef ARDUINO_ARCH_ESP32
	if (inMultiThread)
	{
		this->xSemaphore = xSemaphoreCreateMutex();
	}
#endif
}

MessageStack::MessageStack()
{
	for (byte i = 0; i < MESSAGE_MAXNUMBER; i++)
  {
    this->messages[i][0] = 0;
  }

	this->peakCount = 0;
}

void MessageStack::FreeMessage(byte inMessageIndex)
{
	START_SEMAPHORE()
  this->messages[inMessageIndex][0] = 0;
	END_SEMAPHORE()

	this->GetCount();	// update peakCount...
}

bool MessageStack::PushMessage(const char *inMessage)
{
#if defined(DCCPP_DEBUG_VERBOSE_MODE)
		Serial.print("MessagesStack push ");
		Serial.println(inMessage);
#endif

	START_SEMAPHORE()
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
		if (this->messages[i][0] == 0)
		{
			strncpy(this->messages[i], inMessage, MESSAGE_MAXSIZE);
			this->GetCount();	// update peakCount...
			ABORT_SEMAPHORE()
			return true;
		}

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("Error : a message has been lost ! Stack is full !"));
#endif
	END_SEMAPHORE()

	this->peakCount = MESSAGE_MAXNUMBER + 1;	// at least !
	return false;
}

byte MessageStack::GetPendingMessageIndex()
{
	START_SEMAPHORE()
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
		if (this->messages[i][0] != 0)
		{
			ABORT_SEMAPHORE()
			return i;
		}

	END_SEMAPHORE()
	return 255;
}

const char *MessageStack::GetMessage(byte inMessage, char *inMessageBuffer)
{
	START_SEMAPHORE()
	strncpy(inMessageBuffer, this->messages[inMessage], MESSAGE_MAXSIZE);
	END_SEMAPHORE()

	this->FreeMessage(inMessage);

	return inMessageBuffer;
}

byte MessageStack::GetCount()
{
  int count = 0;
  START_SEMAPHORE()
  for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
    if (this->messages[i][0] != 0)
    {
      count++;
    }

	END_SEMAPHORE()

	if (count > this->peakCount)
		this->peakCount = count;

  return count;
}

#ifdef DCCPP_DEBUG_MODE
#ifdef VISUALSTUDIO
void MessageStack::Test()
{
	char buffer[MESSAGE_MAXSIZE];

	Serial.println("After all initialized");
	printStack();

	this->PushMessage("mess1");
	Serial.println("After one pushed");
	printStack();

	this->PushMessage("mess2");
	Serial.println("After second pushed");
	printStack();

	int pending = this->GetPendingMessageIndex();
	Serial.print("Message available :");
	Serial.print(pending);
	Serial.println("");

	this->GetMessage(pending, buffer);
	Serial.print("Got message :\"");
	Serial.print(buffer);
	Serial.println("\"");
	printStack();

	pending = this->GetPendingMessageIndex();
	Serial.print("Message available :");
	Serial.print(pending);
	Serial.println("");

	this->GetMessage(pending, buffer);
	Serial.print("Got message :\"");
	Serial.print(buffer);
	Serial.println("\"");
	printStack();

	this->PushMessage("mess0");
	this->PushMessage("mess1");
	this->PushMessage("mess2");
	this->PushMessage("mess3");
	this->PushMessage("mess4");
	this->PushMessage("mess5");
	this->PushMessage("mess6");
	this->PushMessage("mess7");
	this->PushMessage("mess8");
	this->PushMessage("mess9");
	this->PushMessage("mess10");	// This one cant be added !
	printStack();
}
#endif

void MessageStack::printStack()
{
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
	{
		Serial.print(i);
		Serial.print(" : \"");
		Serial.print(messages[i]);
		Serial.println("\"");
	}

	Serial.println("");
}
#endif
