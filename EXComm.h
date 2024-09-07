//-------------------------------------------------------------------
#ifndef __EXComm_hpp__
#define __EXComm_hpp__
//-------------------------------------------------------------------

#include <Arduino.h>
#include "defines.h"

#if defined(ENABLE_EXCOMM)

#define MAX_COMMITEMS		8

/** This is a class to handle external communications.
An instance of this class receive message from external world and call DCCEX API functions.
*/
class EXCommItem {
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
		String name;
		// If true, this item will be called in Labox main mode.
		bool MainTrackEnabled;
		// If true, this item will be called in Labox prog mode.
		bool ProgTrackEnabled;
		// If true, this item's loop method must be called at each loop.
		bool AlwaysLoop;

  	static void Setup();
		static void beginItems();
		static void loop();
		static void printItems();

		static void sendPowerItems(bool iSOn);
	  static void sendThrottleItems(uint16_t cab, uint8_t tSpeed, bool tDirection);
  	static void sendFunctionItems(int cab, int16_t functionNumber, bool on);
  	static void sendEmergencyItems();

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
		virtual bool beginItem() = 0;
		/** Ends the usage of the EXCommItem. The EXCommItem is now ready to restart...
		*/
		virtual void endItem() {}
		/** Function to call in the main execution loop to receive bytes.
		@return True if the loop() has been executed without problem, otherwise false.
		*/
		virtual bool loopItem() = 0;

		// Send functions
	  virtual void sendPower(bool iSOn) {}
	  virtual void sendThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection) {}
  	virtual void sendFunction(int cab, int16_t functionNumber, bool on) {}
  	virtual void sendEmergency() {}

	#ifdef DCCPP_DEBUG_MODE
		/** Print the status of the Throttle.
		@remark Only available if DCCPP_DEBUG_MODE is defined.
		*/
		virtual void printItem();
	#endif
};
	
#endif
#endif