/*
   LaBox Project
   XpressNet part

   @Author : lebelge2
   @Organization : Locoduino.org
*/
//=========================================== XpressNetESP V.4  Half-Duplex ou Full-Duplex =====================================================
// Dernière modif: 10-11-25

#ifndef __XPressNet_h
#define __XPressNet_h

#include "DCC.h"
#include "EXComm.h"

#ifdef ENABLE_XPRESSNET

//                             //------ Définisser un mode de transmission --------
//#define FULL_DUPLEX            //----------------Mode Full-Duplex -----------------
#define HALF_DUPLEX          //----------------Mode Half-Duplex -----------------

#if not defined(FULL_DUPLEX) && not defined(HALF_DUPLEX)
#error Définissez un mode de transmission: Half_Duplex ou Full_Duplex.
#endif
#if defined(FULL_DUPLEX) && defined(HALF_DUPLEX)
#error Définissez un seul mode de transmission: Half_Duplex ou Full_Duplex.
#endif

#define XNetVersion 0x30       // System Bus Version V.3.0
#define XNetID 0x10            // Centrale: 0x00 = LZ100; 0x01 = LH200; 0x10 = ROCO MultiMaus; 0x02 = other; 

#define csNormal 0x00          // Normal Operation Resumed ist eingeschaltet
#define csEmergencyStop 0x01   // Der Nothalt ist eingeschaltet
#define csTrackVoltageOff 0x02 // Die Gleisspannung ist abgeschaltet
#define csShortCircuit 0x04    // Kurzschluss
#define csServiceMode 0x08     // Der Programmiermodus ist aktiv - Service Mode

#define Loco14 0x00            // 14 speed step
#define Loco27 0x01            // 27 speed step
#define Loco28 0x02            // 28 speed step
#define Loco128 0x04           // 128 speed step

#define GENERAL_BROADCAST 0x60  //0x160

const bool DIAG_XPNET = false;
//const bool DIAG_XPNET = true;

class XPressNet: public EXCommItem {
  public:
    XPressNet(int inRxPin, int inTxPin, int inDirPin);
    bool begin() override;
    bool loop() override;
		void end() override;
		void getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) override;
    static void Update();
    static void Decodage();
    static void Tx9(int dTx);
    static void Rx9();
    static void F0a28();
    static void Test();
    static void Marqueur1(int a);
    static void Marqueur2(int b);
    static void XNetsend(unsigned char *dataString, byte byteCount);
    static void getXOR(unsigned char *data, byte length);
    static void getfonction(uint8_t adr);
    static void Fonction(int a, int b);
    static void notifyXNetPower(uint8_t State);
    static void setPower(byte Power);
    static void notifyXNetgiveLocoMM(uint8_t a, word b);
    static void SetLocoInfoMM(uint8_t UserOps, uint8_t Steps, uint8_t Speed, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3);
    static void unknown(void);
    static void Rx();
};

enum StateCV {
  Ready,            // 0
  Reading,          // 1
  Reading_OK,       // 2
  Reading_FAIL,     // 3
  Writing,          // 4
  Written_OK,       // 5
  Written_FAIL      // 6
};
static StateCV  stateCV;
#endif
#endif
