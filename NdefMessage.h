#ifndef NdefMessage_h
#define NdefMessage_h

#include <Ndef.h>
#include <NdefRecord.h>

#define MAX_NDEF_RECORDS 4

class NdefMessage
{
    public:
        NdefMessage(void);
        NdefMessage(const uint8_t * data, const int numBytes);  
        NdefMessage(const NdefMessage& rhs);  
        ~NdefMessage();     
        NdefMessage& operator=(const NdefMessage& rhs);
        uint8_t getRecordCount();
        int getEncodedSize(); // need so we can pass array to encode
        void encode(uint8_t* data); // TODO is getBytes better?

        void add(NdefRecord& record); // addRecord?
        void addMimeMediaRecord(String mimeType, String payload);
        void addMimeMediaRecord(String mimeType, uint8_t* payload, int payloadLength);
        void addTextRecord(String text);
        void addTextRecord(String text, String encoding);
        void addUriRecord(String uri);
        void addEmptyRecord();
        NdefRecord getRecord(int index); // TODO overload [] 
        void print();
    private:
        NdefRecord _records[MAX_NDEF_RECORDS];
        uint8_t _recordCount;
};

#endif