#ifndef MifareClassic_h
#define MifareClassic_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

void writeMifareClassic(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength);
NdefMessage readMifareClassic(Adafruit_NFCShield_I2C& nfc, uint8_t * uid, int uidLength);

int getBufferSize(int messageLength);
int getNdefLength(byte *data);

#endif