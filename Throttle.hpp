//-------------------------------------------------------------------
#ifndef __Throttle_hpp__
#define __Throttle_hpp__
//-------------------------------------------------------------------

#include "DCCEX.h"

#if defined(USE_THROTTLES)
#include "CircularBuffer.hpp"
#include "MessageStack.hpp"

#define THROTTLE_COMMAND_SIZE		64
#define THROTTLE_UDPBYTE_SIZE		64

#define NOTSTARTEDTHROTTLE			'.'

/** This is a class to handle throttle communications.
An instance of this class receive message from external world and push it in one of the buffers.
*/
class Throttle
{
protected:
	String name;
	int startCommandCharacter, endCommandCharacter;
	uint16_t id;
	char commandString[THROTTLE_COMMAND_SIZE + 1];
	bool replyToCommands;	// if false, the Throttle will not send replies to the commands sending throttle.

public:
	char type;										/**< ASCII char identifying the type of throttle and set by begin(): 
																'W' for Wifi, 'A' for automation, 'E' for Ethernet, 'S' for Serial
																or '.' (NOTSTARTEDTHROTTLE) for throttle with begin not called. !*/
	Throttle* pNextThrottle;			/**< Address of the next object of this class. NULL means end of the list of Throttles. Do not change it !*/
	CircularBuffer* pBuffer;			/**< Associated buffer for the bytes reception. !*/
	bool contacted;								/**< If true, this throttle has established contact. !*/
	unsigned int lastActivityDate;	/**< If 0 and timeOutDelay != 0, the throttle is not connected... !*/
	unsigned int timeOutDelay;			/**< Delay in millseconds or 0 if no timeout available. !*/

	/** Create a new instance for only one register.
	@param inName	throttle new name.
	@param inTimeOutDelay		value of timeout delay in ms. A value of 0 means no timeout at all.
	*/
	Throttle(const String& inName, unsigned int inTimeOutDelay = 0);

	/** Dispose the throttle.
	*/
	virtual ~Throttle() 
	{
		this->pNextThrottle = NULL;
		if (this->pBuffer != NULL)
			delete this->pBuffer;
	}

	/** Set the timeout delay. 0 for no timeout.
	@param inTimeOutDelay		value of timeout delay in ms. A value of 0 means no timeout at all.
	*/
	void setTimeOutDelay(unsigned int inTimeOutDelay) { this->timeOutDelay = inTimeOutDelay; }

	/** Set if the Throttle should reply to the controller.
	@param inReplyToCommands		False if the commands do not answer to the throttle.
	*/
	void setReplyToCommands(bool inReplyToCommands) { this->replyToCommands = inReplyToCommands; }

	/** Sets the throttle id
	@param inId		Throttle new id.
	*/
	void setId(uint16_t inId) { this->id = inId; }

	/** Gets the throttle id.
	@return	Throttle id.
	*/
	uint16_t getId() const { return this->id; }

	/** Sets the Throttle name
	@param inName	Throttle new name.
	*/
	void setName(const String& inName) { this->name =  inName; }

	/** Gets the Throttle name.
	@return	Throttle name.
	*/
	const String& getName() const { return this->name; }

	/** Push one message in the global message stack.
	*/
	static void pushMessageInStack(uint16_t inThrottleId, const String& inMessage);
	/** Gets the older message from the global message stack.
	*/
	static const String& getMessageFromStackMessage(const String& inMessage, String& inToReturn);
	/** Extract the throttle id from a message.
	*/
	static Throttle* getThrottleFromStackMessage(const String& inMessage);

	/** Start the usage of this throttle.
	If the begin() is not called at least one time, the throttle is not started and will not work !
	@return True if the begin() has been executed without problem, otherwise false.
	*/
	virtual bool begin() = 0;
	/** Ends the usage of the throttle. The throttle is now ready to restart...
	*/
	virtual void end();
	/** Function to call in the main execution loop to receive bytes.
	@return True if the loop() has been executed without problem, otherwise false.
	*/
	virtual bool loop() = 0;

	/** Treat received characters in text commands
	@param	inC	Received character
	@remark	Three cases : 
					either inC is the StartCharacter, so the current command string is emptied, 
					either inC is the ending character, so the current string is send to the message stack,
					any other character is added to the current string.
	*/
	bool getCharacter(char inC);

	/** Treat messages.
	@return True if the function has been executed without problem, otherwise false.
	*/
	virtual bool processBuffer();
	/** Send a text message to this throttle.
	@param inMessage	text message to send.
	@return True if the message has been sent, otherwise false.
	*/
	virtual bool sendMessage(const String& inMessage) = 0;
	/** Checks connection.
	@return True if the throttle is connected, otherwise false.
	*/
	virtual bool isConnected() = 0;
	/** Sets the starting and ending text message characters.
	@param inStartCharacter	Starting character, typically '<' for DCC++ syntax.
	@param inEndCharacter	Ending character, typically '>' for DCC++ syntax.
	*/
	virtual void setCommandCharacters(int inStartCharacter, int inEndCharacter);
	/** Checks if the throttle must send a newline character when sending a text to the throttle.
	@return True if the message must be followed by a newline, otherwise false.
	*/
	virtual bool SendNewline() const;
	/** Send a binary message to this throttle.
	@param inpData	Start of the byte buffer to send.
	@param inLengthData	Number of bytes to send.
	*/
	virtual void write(byte* inpData, int inLengthData) {}
	/** Gets the IP address of the throttle.
	@return IP address.
	*/
	virtual IPAddress remoteIP() {	return INADDR_NONE; }

	/** Checks if the throttle has been contacted by the sm�artphone application if a timezout delay is fixed.
	@return True if the throttle has been contacted, otherwise false.
	*/
	bool isContacted() { return this->contacted || this->lastActivityDate != 0; }

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	virtual void printThrottle();
#endif

protected:
	bool pushDccppMessage(const String& inMessage);
	virtual bool pushMessage(const String& inMessage);
};

#endif
#endif