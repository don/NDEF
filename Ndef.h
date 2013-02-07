#ifndef Ndef_h
#define Ndef_h

#include <inttypes.h>
#include "Arduino.h"

#define MAX_NDEF_RECORDS 10

#define TNF_EMPTY 0x0
#define TNF_WELL_KNOWN 0x01
#define TNF_MIME_MEDIA 0x02
#define TNF_ABSOLUTE_URI 0x03
#define TNF_EXTERNAL_TYPE 0x04
#define TNF_UNKNOWN 0x05
#define TNF_UNCHANGED 0x06
#define TNF_RESERVED 0x07

// TODO see what is useful here
// http://w3c.github.com/nfc/proposals/intel/nfc.html
// which is based on 
// http://www.w3.org/wiki/Near_field_communications_%28NFC%29

class NdefRecord
{
    public:
        NdefRecord();
        NdefRecord(const NdefRecord& rhs);
        ~NdefRecord(); 
        NdefRecord& operator=(const NdefRecord& rhs);       
        int getEncodedSize();
        void encode(uint8_t* data, bool firstRecord, bool lastRecord);
        uint8_t getTnf();
        uint8_t getTypeLength();
        int getPayloadLength();
        uint8_t getIdLength();
        uint8_t * getType();  // TODO don't return array
        uint8_t * getPayload();
        uint8_t * getId(); 
        void setTnf(uint8_t tnf);
        void setType(uint8_t * type, const int numBytes);
        void setPayload(uint8_t * payload, const int numBytes);
        void setId(uint8_t * id, const int numBytes);
        void print();
    private:
        uint8_t getTnfByte(bool firstRecord, bool lastRecord);
        uint8_t _tnf;
        int _typeLength; // uint8_t
        int _payloadLength;        
        int _idLength;    
        uint8_t * _type;        
        uint8_t * _payload;        
        uint8_t * _id;
};

class NdefMessage
{
public:
    NdefMessage(void);
    NdefMessage(uint8_t * data, const int numBytes);  
    ~NdefMessage();     
    int recordCount(); // TODO getRecordCount() or read only property instead of a function?
    int getEncodedSize(); // need so we can pass array to encode
    void encode(uint8_t* data); // TODO is getBytes better?

    void add(NdefRecord record); // addRecord?
    void addMimeMediaRecord(String mimeType, String payload);
    void addMimeMediaRecord(String mimeType, uint8_t* payload, int payloadLength);
    NdefRecord get(int index); // TODO overload [] or use an iterator
    void print();
private:
    //NdefRecord * _records;
    NdefRecord _records[MAX_NDEF_RECORDS];
    int _recordCount;
};

#endif