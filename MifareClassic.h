#ifndef MifareClassic_h
#define MifareClassic_h

#include <Due.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>
#include <NfcTag.h>

class MifareClassic
{
    public:
        MifareClassic(Adafruit_NFCShield_I2C& nfcShield);
        ~MifareClassic();
        NfcTag read(byte *uid, unsigned int uidLength);
        boolean write(NdefMessage& ndefMessage, byte *uid, unsigned int uidLength);
    private:
        Adafruit_NFCShield_I2C* _nfcShield;
        int getBufferSize(int messageLength);
        int getNdefStartIndex(byte *data);
        bool decodeTlv(byte *data, int &messageLength, int &messageStartIndex);
};

#endif