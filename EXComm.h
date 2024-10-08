//-------------------------------------------------------------------
#ifndef __EXComm_hpp__
#define __EXComm_hpp__
//-------------------------------------------------------------------

#include <Arduino.h>
#include "defines.h"

#if defined(ENABLE_EXCOMM)

#define MAX_COMMITEMS		8

class EXCommItem;	// see below...

/** This is a class to handle all EXCommItem external communications.
*/
class EXComm {
  private:
    static bool addItem(byte t, EXCommItem* item);
    static byte lastItem;
    static byte nextCycleItem;
  	static EXCommItem* commItems[MAX_COMMITEMS];
  	static void SetupPrivate(
                EXCommItem * comm0,
                EXCommItem * comm1=NULL,
                EXCommItem * comm2=NULL,
                EXCommItem * comm3=NULL,
                EXCommItem * comm4=NULL,
                EXCommItem * comm5=NULL,
                EXCommItem * comm6=NULL,
                EXCommItem * comm7=NULL
                );

	public:

	  static bool DIAGBASE;
		static String *pInfos;
		static int infosCount;

  	static void Setup();
		static void begin();
		static void loop();
		static void print();

		static int getAllInfo(byte maxSize);

		static void broadcast(byte *message);

		static void sendPower(bool iSOn);
	  static void sendThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection);
  	static void sendFunction(uint16_t cab, int16_t functionNumber, bool on);
  	static void sendEmergency();
};

/** This is a class to handle external communications.
An instance of this class receive message from external world and call DCCEX API functions.
*/
class EXCommItem {
	public:
		String name;
		// If true, this item will be called in Labox main mode.
		bool MainTrackEnabled;
		// If true, this item will be called in Labox prog mode.
		bool ProgTrackEnabled;
		// If true, this item's loop method must be called at each loop.
		bool AlwaysLoop;

		/** Create a new instance
		@param inName	item new name.
		*/
		EXCommItem(const String& inName) { 
			this->name = inName; 
			this->MainTrackEnabled = true;
			this->ProgTrackEnabled = true;
			this->AlwaysLoop = false;
		}

		/** Sets the item name
		@param inName	Item new name.
		*/
		void setName(const String& inName) { this->name =  inName; }

		/** Gets the Item name.
		@return	Item name.
		*/
		const String& getName() const { return this->name; }

		/** Start the usage of this EXCommItem.
		If the begin() is not called at least one time, the EXCommItem is not started and will not work !
		@return True if the begin() has been executed without problem, otherwise false.
		*/
		virtual bool begin() = 0;
		/** Ends the usage of the EXCommItem. The EXCommItem is now ready to restart...
		*/
		virtual void end() {}
		/** Function to call in the main execution loop to receive bytes.
		@return True if the loop() has been executed without problem, otherwise false.
		*/
		virtual bool loop() = 0;

		// Send functions
	  virtual void sendPower(bool iSOn) {}
	  virtual void sendThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection) {}
  	virtual void sendFunction(int cab, int16_t functionNumber, bool on) {}
  	virtual void sendEmergency() {}

  	virtual void broadcastLoco(int16_t slot) {}
  	virtual void broadcastSensor(int16_t id, bool value) {}
  	virtual void broadcastTurnout(int16_t id, bool isClosed) {}
  	virtual void broadcastClockTime(int16_t time, int8_t rate) {}
  	virtual void broadcastPower() {}

		virtual void getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) {}

	#ifdef DCCPP_DEBUG_MODE
		/** Print the status of the Throttle.
		@remark Only available if DCCPP_DEBUG_MODE is defined.
		*/
		virtual void print();
	#endif
};
	
#endif
#endif