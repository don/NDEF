#ifndef Ndef_h
#define Ndef_h

/* NOTE: To use the Ndef library in your code, don't include Ndef.h
   Add the following includes into your sketch

    #include <Wire.h>
    #include <Adafruit_NFCShield_I2C.h>
    #include <NfcAdapter.h>

*/

#include <Arduino.h>

#define NULL (void *)0

void PrintHex(const byte *data, const long numBytes);
void PrintHexChar(const byte *data, const long numBytes);
void DumpHex(const byte *data, const long numBytes, const int blockSize);

#endif