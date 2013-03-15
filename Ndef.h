#ifndef Ndef_h
#define Ndef_h

#include <Arduino.h>

#define NULL (void *)0

void PrintHex(const byte * data, const uint32_t numBytes);
void PrintHexChar(const byte * data, const uint32_t numBytes);
void DumpHex(const byte * data, const uint32_t numBytes, const uint8_t blockSize);




// RTD_TEXT: [0x54], // "T"
// RTD_URI: [0x55], // "U" 
// RTD_SMART_POSTER: [0x53, 0x70], // "Sp"
// RTD_ALTERNATIVE_CARRIER: [0x61, 0x63], // "ac"
// RTD_HANDOVER_CARRIER: [0x48, 0x63], // "Hc"
// RTD_HANDOVER_REQUEST: [0x48, 0x72], // "Hr"
// RTD_HANDOVER_SELECT: [0x48, 0x73], // "Hs"

/*
// Maybe for something like NdefField NdefRecord::getType() 
// Less useful for Payload
// Really need subclasses of NdefRecord with additonal constructors and getters
class NdefField
{
public:
    NdefField();
    ~NdefField();
    NdefField& operator=(const NdefField& rhs);
    int length();
    int getLength();
    void getBytes(uint8_t* data);
    String getValueAsString();
    void setValue(String value);
    void setValue(uint8_t* data, int dataLength);
};
*/

#endif