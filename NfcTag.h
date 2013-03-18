#ifndef NfcTag_h
#define NfcTag_h

#include <inttypes.h>
#include <Arduino.h>
#include <NdefMessage.h>

class NfcTag
{
    public:
        NfcTag();
        NfcTag(uint8_t* uid, uint8_t uidLength);
        NfcTag(uint8_t* uid, uint8_t uidLength, String tagType);
        NfcTag(uint8_t* uid, uint8_t uidLength, String tagType, NdefMessage& ndefMessage);
        NfcTag(uint8_t* uid, uint8_t uidLength, String tagType, const uint8_t * ndefData, const int ndefDataLength);  
        ~NfcTag(void);
        NfcTag& operator=(const NfcTag& rhs);
        uint8_t getUidLength();
        void getUid(uint8_t* uid, uint8_t uidLength);
        String getUidString();
        String getTagType();
        boolean hasNdefMessage();
        NdefMessage getNdefMessage();
        void print();
    private:
        uint8_t* _uid;
        uint8_t _uidLength;
        String _tagType; // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        NdefMessage* _ndefMessage;
        // TODO capacity
        // TODO isFormatted
};

#endif