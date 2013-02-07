#ifndef Ndef_h
#define Ndef_h

#include <inttypes.h>

#define MAX_NDEF_RECORDS 10

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
        uint8_t getTnf();
        uint8_t getTypeLength();
        uint8_t getPayloadLength();
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
        uint8_t _tnf;
        int _typeLength;
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
    uint8_t * encode(); // TODO is getBytes better?
    void add(NdefRecord record); // addRecord?
    NdefRecord get(int index); // TODO overload [] or use an iterator
    void print();
private:
    //NdefRecord * _records;
    NdefRecord _records[MAX_NDEF_RECORDS];
    int _recordCount;
};

#endif