#ifndef MifareUltralight_h
#define MifareUltralight_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

//void write(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength);
NdefMessage& readMifareUltralight(Adafruit_NFCShield_I2C& nfc);

//int getBufferSize(int messageLength);
//int getNdefLength(byte *data);


#endif