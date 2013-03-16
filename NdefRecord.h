#ifndef NdefRecord_h
#define NdefRecord_h

#include <Due.h>
#include <Arduino.h>
#include <Ndef.h>

#define TNF_EMPTY 0x0
#define TNF_WELL_KNOWN 0x01
#define TNF_MIME_MEDIA 0x02
#define TNF_ABSOLUTE_URI 0x03
#define TNF_EXTERNAL_TYPE 0x04
#define TNF_UNKNOWN 0x05
#define TNF_UNCHANGED 0x06
#define TNF_RESERVED 0x07

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

        uint8_t* getType(); 
        uint8_t* getPayload();
        uint8_t* getId();
        
        void getType(uint8_t* type); 
        void getPayload(uint8_t* payload);
        void getId(uint8_t* id); 
        
        void setTnf(uint8_t tnf);
        void setType(const uint8_t * type, const int numBytes);
        void setPayload(const uint8_t * payload, const int numBytes);
        void setId(const uint8_t * id, const int numBytes);
        
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

#endif