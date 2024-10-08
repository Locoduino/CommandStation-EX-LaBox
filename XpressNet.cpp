/*
   LaBox Project
   XpressNet part

   @Author : lebelge2
   @Organization : Locoduino.org
*/
//================================================ XPRESSNET MASTER V.1. ===================================================
// Dernière modif: 01-09-24

#include <Arduino.h>
#include <driver/uart.h>
#include "DCC.h"

#ifdef ENABLE_XPRESSNET

#include "TrackManager.h"
#include "DCCWaveform.h"
#include "DCCEXParser.h"
#include "hmi.h"
#include "EXComm.h"
#include "EXCommItems.h"

// Adresses d'appel périphériques:                p + 0x40 + 000x xxxx   (8 bits)
char Adress40[35] = {0, 65, 66, 195, 68, 197, 198, 71, 0, 72, 201, 202, 75, 204, 77, 78, 0, 207, 80, 209, 210, 83, 212, 85, 0, 86, 215, 216, 89, 90, 219, 92, 221, 222, 95};
// Adresses de réponses aux périphériques:    1 + p + 0x60 + 000x xxxx   (9 bits)
int  Adress60[35] = {0, 481, 482, 355, 484, 357, 358, 487, 0, 488, 361, 362, 491, 364, 493, 495, 0, 367, 496, 369, 370, 499, 372, 501, 0, 502, 375, 376, 505, 506, 379, 508, 381, 382, 511};

int BufXpress[16];                               // Buffer reception datas XpressNet
char StatusOn[4] = {0x62, 0x22, 0x00, 0x40};
char V30[5] = {0x63, 0x21, 0x30, 0x00, 0x72};
char NoOp[3] = {0x61, 0x82, 0xE1};
char NormalOp[3] = {0x61, 0x01, 0x60};
char EnService[3] = {0x61, 0x02, 0x63};
char EmergStop[3] = {0x81, 0x00, 0x81};
char ErreurTransf[3] = {0x61, 0x80, 0xE1};
bool RecData;
unsigned long currentTime = 0;
unsigned long previousTime = 0;
int BufLen;
int Bi;                                // Buffer Index
int Ti;
byte Ac;                               // Tableau Index
int n;                                 // for n
bool dir;
int minAdress;
int maxAdress;
int nData ;                            // Nombre de data
int Xor;                               // Xor
byte b[5];
byte nB;
int16_t XpressCvValue = -1;
int16_t XpressCvAddress = 0;
int MemoAdrAppel;
int MemoAdrRep;
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

int rxPin, txPin, dirPin;

XPressNet::XPressNet(int inRxPin, int inTxPin, int inDirPin) : EXCommItem("XPressNet")
{
	rxPin = inRxPin;
	txPin = inTxPin;
	dirPin = inDirPin;
}

bool XPressNet::begin() {
  uart_config_t uart_config = {              //configure UART_NUM_2 for RS485
    .baud_rate = 62500,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_2,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
  };
  Serial2.begin(62500);
  uart_driver_delete(UART_NUM_2);
  uart_param_config(UART_NUM_2, &uart_config);
  uart_set_pin(UART_NUM_2, txPin, rxPin, dirPin, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_2, 127 * 2, 0, 0, NULL, 0);
  uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);

  minAdress = 0;                                //  Plage d'adressage des périphériques,
  maxAdress = 34;                               //  31 adresses + 4 adresses prioritaires

 	DIAG(F("[XPRESSNET] Serial2 Txd:%d   Rxd:%d   Dir:%d"), txPin, rxPin, dirPin);

	return true;
}

void XPressNet::getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) 
{
	char mess[maxSize*2];

	sprintf(mess, "[XPNET] Serial2");
	*pMess1 = mess;

	sprintf(mess, "[XPNET] Tx:%d Rx:%d", txPin, rxPin);
	*pMess2 = mess;

	sprintf(mess, "[XPNET] dir:%d", dirPin);
	*pMess3 = mess;
}

bool XPressNet::loop() {
  if (Serial2.available() ) {
    Bi = 0;                                     // Buffer Index 0
    Xor = 0;
    while (Serial2.available() > 0) {
      BufXpress[Bi] = Serial2.read();           // Mémorisation des datas reçu
      Xor ^= BufXpress[Bi++];                   // Xor des datas
    }
    if (Xor != 0) {                             // Si pas d'erreur, Xor = 0
		 	DIAG(F("[XPRESSNET] Wrong checksum"));    // Mauvais checksum, retour
			return false;
    }
    else {
      MemoAdrAppel = Adress40[Ti];             // Mémorise adresse d'appel
      MemoAdrRep = Adress60[Ti];               // Mémorise adresse de réponse
      for (n = 0; n < 32; n += 8) {            // Pour adressage prioritaire,
        Adress40[n] = MemoAdrAppel;            // 4x plus rapide.
        Adress60[n] = MemoAdrRep;
      }
      Decodage();                              // Décodage de la trame reçue
    }		
  }

  currentTime = millis();
  if ((currentTime - previousTime) > 10) {       // 5 ms minimum
    previousTime = currentTime;
    Ti++;
    if (Ti > maxAdress)  Ti = minAdress;
    Serial2.write(Adress40[Ti]);                // Envoi adresse aux périphériques (A = adresse, P = Parité.   P10A AAAA
  }

	return true;
}

void XpressCvValueCallback(int16_t inValue) {
  if (inValue >= 0)
    stateCV = StateCV::Reading_OK;
  else
    stateCV = StateCV::Reading_FAIL;
  XpressCvValue = inValue;
}
void XpressCvWriteValueCallback(int16_t inValue) {
  if (inValue >= 0)
    stateCV = StateCV::Written_OK;
  else
    stateCV = StateCV::Written_FAIL;
}

void XPressNet::Decodage() {
  int Adr;
  if (DIAG_XPNET)
	{
    for (n = 0; n < Bi + 1; n++) {                     // DEBUG
      if (BufXpress[n] < 10) Serial.print("0");
      Serial.print(BufXpress[n], HEX);
      Serial.print(" ");
    }
    Serial.println();
	}

  switch (BufXpress[0]) {
    case 0xE4:
      switch (BufXpress[1]) {
        case 0x10:                                // Contrôle vitesse loco. 14 crans
        case 0x11:                                // Contrôle vitesse loco. 27 crans
        case 0x12:                                // Contrôle vitesse loco. 28 crans
        case 0x13:                                // Contrôle vitesse loco. 128 crans
          dir = BufXpress[4] & 0x80;
          Adr = word( BufXpress[2] & 0x1F,  BufXpress[3]);
          DCC::setThrottle(Adr, BufXpress[4], dir);
          break;
        case 0x20:                               // Contrôle F0 à F4 loco
        case 0x21:                               // Contrôle F5 à F8 loco
        case 0x22:                               // Contrôle F9 à F12 loco
        case 0x23:                               // Contrôle F13 à F20 loco
        case 0x28:                               // Contrôle F21 à F28 loco
          nB = 0;
          if (BufXpress[3] > 99) {               // convert train number into a two-byte address
            BufXpress[2] = highByte(BufXpress[3]) | 0xC0;
            b[nB++] = BufXpress[2];                                             // Adresse high
          }
          b[nB++] =  lowByte(BufXpress[3]);                                     // Adresse low
          if (BufXpress[1] == 0x20)   b[nB++] = BufXpress[4] | 0x80;            // F0 à F4
          else if (BufXpress[1] == 0x21)   b[nB++] = BufXpress[4] | 0b10100000; // F5 à F8
          else if (BufXpress[1] == 0x22)   b[nB++] = BufXpress[4] | 0b10110000; // F9 à F12
          else if (BufXpress[1] == 0x23) {                                      // F13 à F20
            b[nB++] =  0b11011110;
            b[nB++] = BufXpress[4];
          }
          else if  (BufXpress[1] == 0x28) {                                     // F21 à F28
            b[nB++] =  0b11011111;
            b[nB++] = BufXpress[4];
          }
          else {}
          DCCWaveform::mainTrack.schedulePacket(b, nB, 4);
          break;
      }
      break;
    case 0xE3:
      break;
    case 0x91:                                          // Requête de demande d’arrêt de d'urgence d’une locomotive adresse courte
      break;
    case 0x92:                                          // Requête de demande d’arrêt de d'urgence d’une locomotive adresse longue
      break;
    case 0x80:                                          // Arrêt urgence toutes loco.
      DCC::setThrottle(0, 1, 1);
      break;
    case 0x52:                                          // Requête de commande à un décodeur d’accessoire.
      b[0] = BufXpress[1] + 128;
      b[1] = BufXpress[2] | 0xF0;
      DCCWaveform::mainTrack.schedulePacket(b, 2, 3);
      break;
    case 0x42:                                           // Requête de demande d’information à un décodeur d’accessoire.
      break;
    case 0x23:                                           // Requête d'écriture CV en Mode Direct (CV mode (CV=1..256))
      if (BufXpress[1] == 0x16) {
        XpressCvAddress = BufXpress[2];
        XpressCvValue = BufXpress[3];
		  	if (DIAG_XPNET)
        	DIAG(F("[XPRESSNET] Write CV %d  value:%d."), XpressCvAddress, XpressCvValue);               // DEBUG
        void (*ptr)(int16_t) = &XpressCvWriteValueCallback;
        DCC::writeCVByte(XpressCvAddress, XpressCvValue, ptr);
        stateCV = Writing;
      }
      break;
    case 0x22:                                           // Requête de lecture CV
      if (BufXpress[1] == 0x14) {                        // Mode paginé

      }
      if (BufXpress[1] == 0x15) {                        // Mode direct (CV mode)
        // Serial.println("Demande Cv");                 // DEBUG
        XpressCvAddress = BufXpress[2];
        XpressCvValue = -1;
		  	if (DIAG_XPNET)
        	DIAG(F("[XPRESSNET] Read CV %d."), XpressCvAddress);               // DEBUG
        void (*ptr)(int16_t) = &XpressCvValueCallback;
        DCC::readCV(XpressCvAddress, ptr);
        stateCV = Reading;
      }
      break;
    case 0x21:
      switch (BufXpress[1] ) {
        case 0x80:                                         // Requête de commande d’arrêt d’urgence
          TrackManager::setMainPower(POWERMODE::OFF);
          TrackManager::setProgPower(POWERMODE::OFF);
          break;
        case 0x81:                                         // Requête de commande de reprise des opérations
          TrackManager::setMainPower(POWERMODE::ON);
          TrackManager::setProgPower(POWERMODE::ON);
          break;
        case 0x24:                                         // Demande de status LaBox (0x21 0x24 Xor)
          DeconnectPin();
          Tx9(Adress60[Ti]);                               // Adresse 9 Bits
          for (n = 0; n < 4; n++)
            Tx9(StatusOn[n]);                              // Réponse de la LaBox  Status On
          ConnectPin();
          TrackManager::setMainPower(POWERMODE::ON);       // Power Tracks ON
          break;
        case 0x21:                                         // Demade Version XpressNet    (0x21 0x21 Xor)
          DeconnectPin();
          Tx9(Adress60[Ti]);                               // Adresse 9 Bits
          for (n = 0; n < 5; n++)
            Tx9(V30[n]);                                   //  Réponse de la LaBox  V.3.0   ID 00
          ConnectPin();
          break;
        case 0x10:                                         // Lecture du CV Service mode
          if (stateCV == StateCV::Reading_OK) {
            stateCV = StateCV::Ready;
				  	if (DIAG_XPNET)
  	  	    	DIAG(F("[XPRESSNET] Reading answer Cv."), XpressCvAddress, XpressCvValue);               // DEBUG
            DeconnectPin();
            Tx9(Adress60[Ti]);
            Tx9(0x63);
            Tx9(0x15);
            Tx9(XpressCvAddress);
            Tx9(XpressCvValue);
            Tx9(0x63 ^ 0x15 ^ XpressCvAddress ^ XpressCvValue );
            ConnectPin();
          }
          else if (stateCV == StateCV::Reading) {
				  	if (DIAG_XPNET)
  	  	    	DIAG(F("[XPRESSNET] Reading..."));               // DEBUG
            DeconnectPin();
            Tx9(Adress60[Ti]);
            Tx9(0x61);                                      // BUSY
            Tx9(0x1F);
            Tx9(0x7E);
            ConnectPin();
          }
          else {
            
          }
          break;
      }
    default:
      break;
  }
}
void XPressNet::DeconnectPin() {
  cli();
  gpio_matrix_out(dirPin, 0x100, false, false);          // Déconnecte la pin Dir (RTS) du module UART2
  gpio_matrix_out(txPin, 0x100, false, false);           // Déconnecte la pin Tx du module UART2
  digitalWrite(dirPin, HIGH);                            // Dir Haut
  delayMicroseconds(8);                                   // 1/2 bit
}
void XPressNet::ConnectPin() {
  digitalWrite(dirPin, LOW);                             // Dir Bas
  gpio_matrix_out(txPin, U2TXD_OUT_IDX, false, false);   // Reconnecte la pin Tx au module UART2
  gpio_matrix_out(dirPin, U2RTS_OUT_IDX, false, false);  // Reconnecte la pin Dir (RTS) au module UART2 (RTS)
  sei();
}

void XPressNet::Tx9(int dTx) {                // Envoi d'un octet 62500 Bauds  9bits  1stop
  digitalWrite(txPin, LOW);       // Bit Start
  delayMicroseconds(15);           // Largeur 16µs
  for (n = 0; n < 9; n++) {        // 9 bits
    if (bitRead(dTx, n))           // Test état du bit
      digitalWrite(txPin, HIGH);  // Etat haut
    else                           // Si non
      digitalWrite(txPin, LOW);   // Etat bas
    delayMicroseconds(15);         // Largeur 16µs
  }                                // Boucle
  digitalWrite(txPin, HIGH);      // 1 Bit Stop
  delayMicroseconds(14);           // 16µs
}
#endif
