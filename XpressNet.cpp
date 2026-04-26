/*
   LaBox Project
   XpressNet part

   @Author : lebelge2@yahoo.fr
   @Organization : Locoduino.org
*/
//=========================================== XPressNet Half-Duplex ou Full-Duplex =====================================================
//                             //------ Définisser un mode de transmission dans XpressNet.h --------

#include <Arduino.h>
#include <driver/uart.h>
#include "DCC.h"

#ifdef ENABLE_XPRESSNET
#include "XpressNet.h"
#include "TrackManager.h"
#include "DCCWaveform.h"
#include "DCCEXParser.h"
#include "hmi.h"
#include "EXComm.h"
#include "EXCommItems.h"
#include "LaboxModes.h"

const String XpressNetVersion = "V.4.0";
// Dernière modif: 29-03-26
// Correction d'un Bug, icône aiguillage
//                   

int OffsetAdress = 1;         // Offset numéro de décodeur de fonctions
int OffsetPort = 0;           // Offset numéro de sortie (Décodeur de fonctions)

// Adresses d'appel périphériques:                p + 0x40 + 000x xxxx
uint8_t  UserOps[35] = {68, 65, 66, 195, 68, 197, 198, 71, 75, 72, 201, 202, 75, 204, 77, 78, 210, 207, 80, 209, 210, 83, 212, 85, 89, 86, 215, 216, 89, 90, 219, 92, 221, 222, 95};
// Adresses de réponses aux périphériques:        p + 0x60 + 000x xxxx
uint8_t  DirectedOps[35] = {228, 225, 226, 99, 228, 101, 102, 231, 235, 232, 105, 106, 235, 108, 237, 238, 114, 111, 240, 113, 114, 143, 116, 245, 249, 246, 119, 120, 249, 250, 123, 252, 125, 126, 255};

uint16_t BufXpress[9];                  // Buffer reception datas XpressNet
uint8_t Railpower;
unsigned long currentxpTime;
unsigned long previousxpTime;
int Bi;                                // Buffer Index
int Ti;
uint8_t xpSpeed;
bool dir;
bool Bit9;
int Xor;                               // Xor
int xpAdr;
uint8_t Gr1, Gr2, Gr3, Gr4, Gr5;       // Groupes fonctions
uint8_t fonction;
unsigned long xpRegFonc;
int rxPin, txPin, dirPin;
int Logic = 12;
unsigned int xpTime;
volatile unsigned int xpData[40];
volatile bool xpValBit[40];
int NbrBit;
int NbrBitFinTrame;
int PosBit;
int valTemp;
bool OpCode;
uint8_t xpCvValue = 0;
uint8_t xpCvAddress = 0;
uint16_t nbrBitHaut;
uint16_t nbrBitBas;
int posBit;
int nbr;
int timeExceeded;
uint8_t memoTrnt;
//--------------------------------------------------------------------------------
XPressNet::XPressNet(int inRxPin, int inTxPin, int inDirPin) : EXCommItem("XPressNet") {
  rxPin = inRxPin;          // 12
  txPin = inTxPin;          // 13
  dirPin = inDirPin;        // 15
#ifdef HALF_DUPLEX
  Logic = inRxPin;
#endif
  this->MainTrackEnabled = true;
  this->ProgTrackEnabled = true;
  this->AlwaysLoop = true;
}
//#endif
//------------------------------------------------------------------ -----------
#ifdef HALF_DUPLEX
void XPressNet::Rx() {
  BufXpress[Bi] = 0;
  posBit = 0;
  while (posBit < 11) {
    timeExceeded = 0;
    nbr = 0;
    while (digitalRead(txPin) == LOW) {        // Niveau bas
      if (nbr == 0) {
        bitClear(BufXpress[Bi], posBit++);
        Marqueur2(1);
        if (posBit > 11) break;
      }
      // nbr++;
      if (nbr++ > 55) nbr = 0;  //51
      if (timeExceeded++ > 800) return;
    }
    Marqueur1(1);
    timeExceeded = 0;
    nbr = 0;
    while (digitalRead(txPin) == HIGH) {         // Niveau haut
      if (nbr == 0) {
        bitSet(BufXpress[Bi], posBit++);
        Marqueur2(1);
        if (posBit > 11) break;
      }
      //   nbr++;
      if (nbr++ > 55) nbr = 0;
      if (timeExceeded++ > 800) return;
    }
    Marqueur1(1);
  }  // Fin de while
  BufXpress[Bi] = (BufXpress[Bi] >> 1) & 0xFF;       // Enleve Start et Stop bit
  Marqueur1(1);
}
#endif
//------------------------------------------------------------------------------------------
bool XPressNet::begin() {
#ifdef HALF_DUPLEX
  pinMode(Logic, OUTPUT);
  pinMode(14, OUTPUT);
  DIAG(F("[XPRESSNET]  RxTx:%d  Dir:%d"), txPin, dirPin);
#endif
#ifdef FULL_DUPLEX
  Serial2.begin(62500, SERIAL_8N1, rxPin, txPin);               // Connecté au port série2
  DIAG(F("[XPRESSNET] Serial2 Txd:%d   Rxd:%d   Dir:%d"), txPin, rxPin, dirPin);
#endif
  pinMode(dirPin, OUTPUT);
  delay(3);
  // XPressNet::setPower(csEmergencyStop);
  XPressNet::setPower(csTrackVoltageOff);
  delay(3);
  XPressNet::setPower(csTrackVoltageOff);
  return true;
}
//---------------------------------------------------------------------------------
bool XPressNet::loop() {
  currentxpTime = micros();
  if (currentxpTime < (previousxpTime + 3000))        // 3ms minimum
    return false;
  previousxpTime = currentxpTime;
  XPressNet::Update();
  return true;
}
void XPressNet::end() {
#ifdef FULL_DUPLEX
	Serial2.end();
#endif
}

//-------------------------------------------------------------------
void XPressNet::Update() {
  Ti++;                                              // Adresse périphérique suivante
  if (Ti > 34)  Ti = 0;                              // Plage d'adresse
#ifdef HALF_DUPLEX                                   // MODE HALF_DUPLEX
  OpCode = true;                                     // 8 bit sans Stop
  pinMode(txPin, OUTPUT);                            // Pin RxTx en sortie
  uint8_t callByte[1] = {UserOps[Ti]};               // Envoi adresse aux périphériques (A = adresse, P = Parité.   P10A AAAA)
  XPressNet::XNetsend(callByte, 1);
  pinMode(txPin, INPUT_PULLUP);                      // Pin Tx en entrée
  OpCode = false;
  for (int n = 0; n < 800; n++) {                     // Le périphérique a une fenêtre de 110 µs pour répondre. (200µ)
    if (digitalRead(txPin) == LOW) goto A;           // Attend réponse du périphérique (Start Bit, niveau bas pin rx)
  }
  Marqueur1(1);                                        // Pour analyseur logique
  return ;                                            // Temps dépassé, retour. (Périférique suivant)
A:
  //Marqueur1(1);
  Bi = 0;
  Xor = 0;
  cli();
  XPressNet::Rx();
  int nbrByte =  BufXpress[Bi] & 0x0F;
  //Serial.println(nbrByte);
  Xor ^= BufXpress[Bi++];
  for (int n = 0; n < nbrByte + 1; n++) {
    for (int n = 0; n < 800; n++) {
      if (digitalRead(txPin) == LOW) goto B;           // Attend start bit
    }
    //Marqueur1(1);                                        // Pour analyseur logique
    return ;                                            // Temps dépassé, retour. (Périférique suivant)
B:
    XPressNet::Rx();
    Xor ^= BufXpress[Bi++];                             // Xor des datas
  }
  sei();

#endif //....................................
#ifdef FULL_DUPLEX                                        // MODE  FULL_DUPLEX
  digitalWrite(dirPin, HIGH);                           // Dir Haut
  Serial2.write(UserOps[Ti]);                           // Envoi adresse au périphérique (A = adresse, P = Parité.   P10A AAAA)
  delayMicroseconds(150);                               // Attendre envoi complet.
  digitalWrite(dirPin, LOW);                            // Dir Bas
  for (int n = 0; n < 80; n++) {                        // Le périphérique a une fenêtre de 110 µs pour répondre.
    if (digitalRead(rxPin) == LOW) goto A;              // Attend réponse du périphérique (Start Bit, niveau bas pin rx)
    delayMicroseconds(1);
  }
  // Marqueur1(1);                                           // Pour analyseur logique
  return ;                                               // Temps dépassé, retour. (Périférique suivant)
A:                                                       // Réception en cours...
  for (int n = 0; n < 100; n++) {                        // 100 ms
    if (Serial2.available()) goto B;                     // Attend datas disponibles
    delay(1);
  }
  // Marqueur1(1);                                           // Pour analyseur logique
  return ;                                                // Temps dépassé, retour. (Périférique suivant)
B:                                                        // Datas disponibles
  //Marqueur1(1);
  Bi = 0;                                                 // Buffer Index 0
  Xor = 0;
  while (Serial2.available() > 0) {                       // Lire tous les Datas
    BufXpress[Bi] = Serial2.read();                       // Lecture et mémorisation des datas reçu
    delayMicroseconds(200);
    Xor ^= BufXpress[Bi++];                               // Xor des datas
  }
#endif //...................................
  if (Xor != 0) {                                         // Si pas d'erreur, Xor = 0
    // DIAG(F("[XPRESSNET] Wrong checksum"));                // Mauvais checksum, retour
    //  Serial.println("Mauvais checksum");                   // Mauvais checksum, retour
    uint8_t ERR[] = {DirectedOps[Ti], 0x61, 0x80, 0xE1 }; // Evoi code d'erreur
    XNetsend(ERR, 4);
    return ;
  }
  for (int n = 0; n < 32; n += 8) {                       // Pour adressage prioritaire,
    UserOps[n] = UserOps[Ti];                             // Mémorise adresse d'appel     4x plus rapide.
    DirectedOps[n] = DirectedOps[Ti];                     // Mémorise adresse de réponse
  }
  Decodage();                                             // Décodage de la trame reçue
}

void XPressNet::getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) 
{
	char mess[maxSize*2];

	sprintf(mess, "[XNet] %s %s", XpressNetVersion.c_str(),
	#ifdef HALF_DUPLEX
		"HDuplex"
	#else
		"FDuplex"
	#endif
	);
	*pMess1 = mess;	// Creates a copy of the string...

	sprintf(mess, "[XNet] RX %d/TX %d", rxPin, txPin);
	*pMess2 = mess;	// Creates a copy of the string...

	sprintf(mess, "[XNet] Dir %d", dirPin);
	*pMess3 = mess; // Creates a copy of the string
}

//-------------------------------------------------------------------
void xpCvValueCallback(int16_t inValue) {
  if (inValue >= 0)
    stateCV = StateCV::Reading_OK;
  else
    stateCV = StateCV::Reading_FAIL;
  xpCvValue = inValue;
}
//-----------------------------------------------------------------------
void XpressCvWriteValueCallback(int16_t inValue) {
  if (inValue >= 0)
    stateCV = StateCV::Written_OK;
  else
    stateCV = StateCV::Written_FAIL;
}
//------------------------------------------------------------
void XPressNet::Decodage() {
  if (DIAG_XPNET) {
    Serial.print("=> ");
    for (int n = 0; n < Bi; n++) {                     // DEBUG
      if (BufXpress[n] < 10) Serial.print("0");
      Serial.print(BufXpress[n], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  Bit9 = false;
  switch (BufXpress[0]) {
    case 0xE6:  {                                        //   POM CV write MultiMaus
        switch (BufXpress[1]) {
          case 0x30:
            uint16_t Adr = ((BufXpress[2] & 0x3F) << 8) + BufXpress[3];
            uint16_t CVAdr = (((BufXpress[4] & B11) << 8) + BufXpress[5]);
            if ((BufXpress[4] & 0xFC) == 0xEC)  {                              //set byte
              /*   Serial.print("POM 0xEC ");
                 Serial.print(Adr);
                 Serial.print(" ");
                 Serial.print(CVAdr);
                 Serial.print(" ");
                 Serial.println(BufXpress[6]); */
              DCC::writeCVByteMain(Adr, CVAdr, BufXpress[6]);
            }
            if ((BufXpress[4] & 0xFC) == 0xE8)  {                             //set byte
              byte bitNum = BufXpress[6] & B00000011 << 8;
              bool value = BufXpress[6] & B00000100;
              DCC::writeCVBitMain(Adr, CVAdr, bitNum, value);
            }
            break;
        }
      } break;
    case 0xE4:
      xpAdr = word(BufXpress[2] & 0x3F, BufXpress[3]);
      dir = BufXpress[4] & 0x80;
      xpSpeed = BufXpress[4];
      fonction = BufXpress[4];
      switch (BufXpress[1]) {
        case 0x10:                                         // Contrôle vitesse loco. 14 crans
        case 0x11:                                         // Contrôle vitesse loco. 27 crans
        case 0x12:                                         // Contrôle vitesse loco. 28 crans
        case 0x13:                                         // Contrôle vitesse loco. 128 crans
          DCC::setThrottle(xpAdr, xpSpeed, dir);
          break;
        case 0x20:                                // Contrôle F0 à F4 loco
          fonction = (fonction & 0b00010000) >> 4 | (fonction & 0b00001111) << 1 ;      // F0 F4 F3 F2 F1   moveBit4to0  F4 F3 F2 F1 F0
          XPressNet::Fonction(5, 0); break;             // Contrôle F0 à F4 loco
        case 0x21: XPressNet::Fonction(4, 5); break;    // Contrôle F5 à F8 loco
        case 0x22: XPressNet::Fonction(4, 9); break;    // Contrôle F9 à F12 loco
        case 0x23: XPressNet::Fonction(8, 13); break;   // Contrôle F13 à F20 loco
        case 0x28: XPressNet::Fonction(5, 21); break;   // Contrôle F21 à F28 loco
      }
      break;
    case 0xE3:
      switch (BufXpress[1]) {
        case 0x00: {
            xpAdr = word(BufXpress[2] & 0x3F, BufXpress[3]);
            xpSpeed = DCC::getThrottleSpeed(xpAdr);
            XPressNet::getfonction(xpAdr);
            uint8_t LocoInfo[] = {DirectedOps[Ti], 0xE4, 0x04, xpSpeed, Gr1, Gr2, 0x00 };   // 0xE4  0x04  (F0 à F9)
            XPressNet:: getXOR(LocoInfo, 7);
            XNetsend (LocoInfo, 7);
          } break;
        case 0x08:                   // 3.6  Requête d'état de fonction F13-F28
        case 0x09:
          break;
        case 0xF0:                   // Multimaus
          XPressNet::notifyXNetgiveLocoMM(DirectedOps[Ti], word(BufXpress[2] & 0x3F, BufXpress[3])); break;
      }
      break;
    case 0x91:                                          // Requête de demande d’arrêt de d'urgence d’une locomotive adresse courte
      DCC::setThrottle(BufXpress[1] , 1, 1); break;
    case 0x92:                                          // Requête de demande d’arrêt de d'urgence d’une locomotive adresse longue
      xpAdr = word( BufXpress[2] & 0x1F,  BufXpress[3]);
      DCC::setThrottle(xpAdr, 1, 1); break;
    case 0x80:                                          // Arrêt urgence toutes loco. coupure électrique
      if (BufXpress[1] == 0x80) {
        Railpower = csEmergencyStop;
        notifyXNetPower(Railpower);
      }
      break;
    case 0x52: {                                         // Requête de commande à un décodeur d’accessoire.
        int Address = BufXpress[1];  // Modif
        int data = BufXpress[2];
        uint8_t port = ((data & 0x06) >> 1) + OffsetPort;          // 000000BB
        bool gate = bitRead(data, 0);                              // 0000000D
        bool OnOff =  bitRead(data, 3);                            // 0000d000
        DCC::setAccessory(Address, port, gate, OnOff);
        uint8_t TrntInfo[] = {DirectedOps[Ti], 0x42, 0x00, 0x00, 0x00 }; // Feedback (0x42,MOD,DATA,...,XOR) Accessory decoder information response
        TrntInfo[2] = Address;
        TrntInfo[3] = 0;
        if ((data & 0x3) == 1)
          TrntInfo[3] = 6;
        else if ((data & 0x3) == 3)
          TrntInfo[3] = 9;
        else
          TrntInfo[3] = 5;
        if (bitRead (data, 2))
          bitSet(TrntInfo[3], 4);
        memoTrnt = TrntInfo[3];
        getXOR(TrntInfo, 5);
        XNetsend(TrntInfo, 5);
      } break;
    case 0x42: {                                          // 0x42 Demande informations sur le décodeur d'accessoires
        int Address = BufXpress[1];   // Modif
        //int data = BufXpress[2];
        uint8_t TrntInfo[] = {DirectedOps[Ti], 0x42, 0x00, 0x00, 0x00 }; // Feedback (0x42,MOD,DATA,...,XOR) Accessory decoder information response
        TrntInfo[2] = Address;
        TrntInfo[3] = memoTrnt;
        getXOR(TrntInfo, 5);
        XNetsend(TrntInfo, 5);
      } break;
    case 0x23:                                           // Requête d'écriture CV en Mode Direct (CV mode (CV=1..256))
      if (BufXpress[1] == 0x12) {                        // Register Mode write request (Register Mode)
        Serial.println("Non supporté");
      }
      if (BufXpress[1] == 0x16) {                        // Direct Mode CV write request (CV mode)
        xpCvAddress = BufXpress[2];
        xpCvValue = BufXpress[3];
        if (DIAG_XPNET)
          DIAG(F("[XPRESSNET] Write CV %d  value:%d."), xpCvAddress, xpCvValue);               // DEBUG
        void (*ptr)(int16_t) = &XpressCvWriteValueCallback;
        DCC::writeCVByte(xpCvAddress, xpCvValue, ptr);
        stateCV = Writing;
      }
      if (BufXpress[1] == 0x17) {                        // Paged Mode write request(Paged mode)
        Serial.println("Non supporté");
      }
      break;
    case 0x22:                                           // Requête de lecture CV
      if (BufXpress[1] == 0x11) {                        //Register Mode read request
        Serial.println("Non supporté 0x22 0x11");
      }
      if (BufXpress[1] == 0x14) {                        //Paged Mode read request
        Serial.println("Non supporté 0x22 0x14");
      }
      if (BufXpress[1] == 0x15) {                        //Direct Mode CV read request
        Serial.println("Demande Cv");                    // DEBUG
        xpCvAddress = BufXpress[2];
        xpCvValue = -1;
        if (DIAG_XPNET)
          DIAG(F("[XPRESSNET] Read CV %d."), xpCvAddress);               // DEBUG
        void (*ptr)(int16_t) = &xpCvValueCallback;
        DCC::readCV(xpCvAddress, ptr);
        stateCV = Reading;
      }
      break;
    case 0x21:
      switch (BufXpress[1] ) {
        case 0x80:                                             // Requête de commande d’arrêt d’urgence
          Railpower = csTrackVoltageOff;
          XPressNet::notifyXNetPower(Railpower);             // 0x60, 0x61, 0x00, 0x61     Broadcast Track power OFF
          break;
        case 0x81:                                              // Requête de commande de reprise des opérations
          Railpower = csNormal;
          XPressNet::notifyXNetPower(Railpower);             // 0x60, 0x61, 0x01, 0x60      Broadcast Normal operation
          break;
        case 0x24: {                                            // Demande de status LaBox (0x21 0x24 0x05)
            uint8_t   sendStatus[] = { DirectedOps[Ti], 0x62, 0x22, Railpower, 0x00 };
            XPressNet::getXOR(sendStatus, 5);
            XNetsend(sendStatus, 5);
            Ti--;
          } break;
        case 0x21: {                                              // Demande Version Centrale XpressNet    (0x21 0x21 0x00)
            uint8_t  sendVersion[] = { DirectedOps[Ti], 0x63, 0x21, XNetVersion, XNetID, 0x00 };   //63 21 36 0 74
            XPressNet::getXOR(sendVersion, 6);
            XNetsend(sendVersion, 6);
            Ti--;
          } break;
        case 0x10:                                         // Lecture du CV Service mode
          if (stateCV == StateCV::Reading_OK) {
            stateCV = StateCV::Ready;
            if (DIAG_XPNET)
              DIAG(F("[XPRESSNET] Reading answer Cv."), xpCvAddress, xpCvValue);               // DEBUG
            uint8_t  sendReading_OK[] = { DirectedOps[Ti], 0x63, 0x14, xpCvAddress, xpCvValue, 0x00 };   //63 14 ou 15 ?
            XPressNet::getXOR(sendReading_OK, 6);
            XNetsend(sendReading_OK, 6);
          }
          else if (stateCV == StateCV::Reading) {
            DIAG(F("[XPRESSNET] Reading..."));               // DEBUG
            uint8_t  sendReading[] = { DirectedOps[Ti], 0x61, 0x1F, 0x00 };   //63 1F
            XPressNet::getXOR(sendReading, 4);
            XNetsend(sendReading, 4);
          }
          else {

          }
          break;
      }
      break;
    default:
      unknown();                                          // Commande non supportée. {0x61, 0x82, 0xE3}
      DIAG(F("[XPRESSNET] Commande non supportée."));
      break;
  }
}
//------------------------------------------------------
void XPressNet::notifyXNetPower(uint8_t State) {
  if (State == csNormal)
    TrackManager::setMainPower(POWERMODE::ON);      // Requête de commande de reprise des opérations
  else if (State == csEmergencyStop)
    TrackManager::setMainPower(POWERMODE::OFF);     // Requête de commande d’arrêt d’urgence
  else if (State == csTrackVoltageOff)
    TrackManager::setMainPower(POWERMODE::OFF);     // Requête de commande court circuit
  //else                                              // ServiceMode 0x08
  XPressNet::setPower(State);                        // Envoi l'état de la centrale aux périphériques
}
//-------------------------------------------------------
void XPressNet::setPower(uint8_t Power) {
  switch (Power) {
    case csNormal: {
        uint8_t NormalPower[] = { GENERAL_BROADCAST, 0x61, 0x01, 0x60 }; XNetsend(NormalPower, 4); break;
      }
    case csEmergencyStop: {
        uint8_t EStopPower[] = { GENERAL_BROADCAST, 0x81, 0x00, 0x81 }; XNetsend(EStopPower, 4); break;
      }
    case csTrackVoltageOff: {
        uint8_t OffPower[] = { GENERAL_BROADCAST, 0x61, 0x00, 0x61 }; XNetsend(OffPower, 4); break;
      }
    case csShortCircuit: {
        uint8_t ShortPower[] = { GENERAL_BROADCAST, 0x61, 0x08, 0x69 }; XNetsend(ShortPower, 4); break;
      }
    case csServiceMode: {
        uint8_t ServicePower[] = { GENERAL_BROADCAST, 0x61, 0x02, 0x63 }; XNetsend(ServicePower, 4); break;
      }
  }

  Railpower = Power;  //Save new Power State
}
//--------------------------------------------------------
void XPressNet::notifyXNetgiveLocoMM(uint8_t UserOps, word Address) {       // Requête d’information d'une locomotive (Multimaus)
  Serial.println("GM");
  uint8_t Speed = DCC::getThrottleSpeed(Address);
  bool dir = DCC::getThrottleDirection(Address);
  if (dir)
    Speed += 0x80;
  XPressNet::getfonction(Address);
  SetLocoInfoMM(UserOps, 4, Speed, 0, Gr1, Gr2, Gr3);
}
//-----------------------------------------------------------------------------
void XPressNet::SetLocoInfoMM(uint8_t UserOps, uint8_t Steps, uint8_t Speed, uint8_t F0, uint8_t F1, uint8_t F2, uint8_t F3) {
  //E7 0x04=128 Steps||0x0C=128+Busy (Dir,Speed) (F0,F4,F3,F2,F1) F5-F12 F13-F20 0x00 0x00 CRC
  byte v = Speed;
  if (Steps == Loco28 || Steps == Loco27) {
    v = (Speed & 0x0F) << 1;                                   //Speed Bit
    v |= (Speed >> 4) & 0x01;                                  //Addition Speed Bit
    v |= 0x80 & Speed;                                         //Dir
  }
  uint8_t LocoInfo[] = {UserOps,  0xE7, Steps, v, 0x20, F1, F2, F3, 0x00, 0x00 };
  //ERROR: Steps change form > 99 to 28steps only!!! why?? (This info comes from DCC:"notifyLokAll(..)"
  LocoInfo[4] |= F0;
  XPressNet::getXOR(LocoInfo, 10);
  XNetsend(LocoInfo, 10); // XNetsend
}
//-----------------------------------------------------
void  XPressNet::Fonction(int a, int b) {                  // 28 fonctions
  for (int i = 0; i < a; i++) {
    if (bitRead(xpRegFonc, i + b) !=  bitRead(fonction, i)) {
      DCC::setFn(xpAdr, i + b, bitRead(fonction, i));
      bitWrite( xpRegFonc, i + b, bitRead(fonction, i));
    }
  }
}
//-------------------------------------------------
void XPressNet::getXOR (uint8_t *data, byte length) {       // calculate the XOR
  byte XOR = 0x00;
  data++;
  for (int i = 0; i < (length - 2); i++) {
    XOR = XOR ^ *data;
    data++;
  }
  *data = XOR;
}
//--------------------------------------------------------------------------------------------
void XPressNet::XNetsend(unsigned char *dataString, byte byteCount) {
#ifdef HALF_DUPLEX
  pinMode(txPin, OUTPUT);             // Pin RxTx en sortie
#endif
#ifdef FULL_DUPLEX
  gpio_matrix_out(txPin, 0x100, false, false);           // Déconnecte la pin Tx du module UART2
#endif
  digitalWrite (dirPin, HIGH);
  Bit9 = false;
  cli();
  for (int i = 0; i < byteCount; i++)
    Tx9(*dataString++);
  digitalWrite (dirPin, LOW);
  sei();
#ifdef HALF_DUPLEX
  pinMode(txPin, INPUT_PULLUP);                 // Pin RxTx en entrée
#endif
#ifdef FULL_DUPLEX
  gpio_matrix_out(txPin, U2TXD_OUT_IDX, false, false);   // Reconnecte la pin Tx au module UART2
#endif
}
//--------------------------------------------------------------------------------------------
void XPressNet::Tx9(int dTx) {    // Envoi d'un octet 62500 Bauds   9 bits  1 stop
  //cli();
  digitalWrite(txPin, LOW);        // Bit Start
  delayMicroseconds(15);             // Largeur 16µs
  for (int m = 0; m < 8; m++) {      // 8 bits
    if (bitRead(dTx, m))             // Test état du bit
      digitalWrite(txPin, HIGH);   // Etat haut
    else                             // Si non
      digitalWrite(txPin, LOW);    // Etat bas
    delayMicroseconds(15);           // Largeur 16µs
  }                                  // bit suivant
  if (Bit9 == false)                 // Bit 9
    digitalWrite(txPin, HIGH);     // Bit 9 Haut
  else
    digitalWrite(txPin, LOW);      // Bit 9 Bas
  Bit9 = true;
  if (OpCode == true)                // 8 bits sans Stop
    return;
  delayMicroseconds(15);             // 16µs
  digitalWrite(txPin, HIGH);       // 1 Bit Stop
  delayMicroseconds(15);             // 16µs
  // sei();
}
//---------------------------------------------
void XPressNet::getfonction(uint8_t xpAdr) {
  uint32_t FonctionMap = DCC::getFunctionMap(xpAdr);
  uint8_t moveBit0to4 = FonctionMap & 0x1F ;     // 5 bits
  Gr1 = (moveBit0to4 & 0b00000001) << 4 | (moveBit0to4 & 0b00011111) >> 1 ;   // F4 F3 F2 F1 F0    moveBit0to4  F0 F4 F3 F2 F1
  //Gr1 = (FonctionMap & 0x1F) ;                 // Groupe 1: 0 0 0 F0 F4 F3 F2 F1              bit0 à bit4
  Gr2 = (FonctionMap & 0xF00) >> 8;              // Groupe 2: 0 0 0 0 F8 F7 F6 F5               bit8 à bit11
  Gr3 = (FonctionMap & 0xF000) >> 12;            // Groupe 3: 0 0 0 0 F12 F11 F10 F9            bit12 à bit15
  Gr4 = (FonctionMap & 0xFF0000) >> 16;          // Groupe 4: F20 F19 F18 F17 F16 F15 F14 F13   bit16 à bit23
  Gr5 = (FonctionMap & 0xFF000000) >> 24;        // Groupe 5: F28 F27 F26 F25 F24 F23 F22 F21   bit24 à bit31
}
//--------------------------------------------------------------
void XPressNet::unknown(void) {
  uint8_t NotIn[] = {DirectedOps[Ti], 0x61, 0x82, 0xE3 };
  XNetsend(NotIn, 4);
}
//----------------------------------------------------------
void XPressNet::Marqueur1(int a) {    // Pour analyseur logique
  digitalWrite(Logic, HIGH);      // Haut __-
  // delayMicroseconds(a);
  digitalWrite(Logic, LOW);       // Bas  -__
}
void XPressNet::Marqueur2(int b) {    // Pour analyseur logique
  digitalWrite(14, HIGH);      // Haut __-
  // delayMicroseconds(a);
  digitalWrite(14, LOW);       // Bas  -__
}
#endif
