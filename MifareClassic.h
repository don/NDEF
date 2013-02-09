#ifndef MifareClassic_h
#define MifareClassic_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

void write(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength);

#endif