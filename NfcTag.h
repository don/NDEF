#ifndef NfcTag_h
#define NfcTag_h

#include <inttypes.h>
#include <Arduino.h>
#include <NdefMessage.h>

class NfcTag
{
    public:
		enum Type { MIFARE_CLASSIC = 0, TYPE_1, TYPE_2, TYPE_3, TYPE_4, UNKNOWN = 99 };
	
        NfcTag();
        NfcTag(byte *uid, unsigned int uidLength);
        NfcTag(byte *uid, unsigned int uidLength, Type tagType);
        NfcTag(byte *uid, unsigned int uidLength, Type tagType, NdefMessage& ndefMessage);
        NfcTag(byte *uid, unsigned int uidLength, Type tagType, const byte *ndefData, const int ndefDataLength);
        ~NfcTag(void);
        NfcTag& operator=(const NfcTag& rhs);
        uint8_t getUidLength();
        void getUid(byte *uid, unsigned int uidLength);
        Type getTagType();
        boolean hasNdefMessage();
        NdefMessage getNdefMessage();
#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        byte *_uid;
        unsigned int _uidLength;
        Type _tagType; // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        NdefMessage* _ndefMessage;
        // TODO capacity
        // TODO isFormatted
};

#endif