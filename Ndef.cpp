#include "Arduino.h"
#include "Ndef.h"

//void Adafruit_NFCShield_I2C::PrintHex(const byte * data, const uint32_t numBytes)
void PrintHex(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    Serial.print("0x");
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print("0");
    Serial.print(data[szPos]&0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(" ");
    }
  }
  Serial.println("");
}

NdefRecord::NdefRecord()
{
    _tnf = 0;
    _typeLength = 0;
    _payloadLength = 0;    
    _idLength = 0;
}

// TODO NdefRecord::NdefRecord(tnf, type, payload, id)

NdefRecord::~NdefRecord()
{
    if (_typeLength) 
    {
        free(_type);
    }
    
    if (_payloadLength)
    {
        free(_payload);        
    }
    
    if (_idLength)
    {
        free(_id);        
    }
}

// This is Java style, what is the C++/Arduino way?
uint8_t NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(uint8_t tnf)
{
    _tnf = tnf;
}

uint8_t * NdefRecord::getType()
{
    return _type;
}

void NdefRecord::setType(uint8_t * type, const int numBytes)
{
    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

uint8_t * NdefRecord::getPayload()
{
    return _payload;
}

void NdefRecord::setPayload(uint8_t * payload, const int numBytes)
{
    _payload = (uint8_t*)malloc(numBytes);    
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

uint8_t * NdefRecord::getId()
{
    return _id;
}

void NdefRecord::setId(uint8_t * id, const int numBytes)
{
 
    _id = (uint8_t*)malloc(numBytes);    
    memcpy(_id, id, numBytes);
    _idLength = numBytes;
}

void NdefRecord::print()
{
    Serial.println("  NDEF Record");    
    Serial.print("    TNF 0x");Serial.println(_tnf, HEX);
    Serial.print("    Type Length 0x");Serial.println(_typeLength, HEX);
    Serial.print("    Payload Length 0x");Serial.println(_payloadLength, HEX);    
    if (_idLength)
    {
        Serial.print("    Id Length 0x");Serial.println(_idLength, HEX);  
    }
    Serial.print("    Type ");PrintHex(_type, _typeLength);  
    Serial.print("    Payload ");PrintHex(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print("    Id ");PrintHex(_id, _idLength);  
    }
}

NdefMessage::NdefMessage(void)
{
    Serial.println("NdefMessage constructor"); 
    _recordCount = 0;       
}

NdefMessage::NdefMessage(byte * data, const int numBytes)
{
    _records = (NdefRecord*)malloc(sizeof(NdefRecord) * 10);
    _recordCount = 0;
    
    int index = 0;
    
    while (index <= numBytes) {
        
        // decode tnf - first byte is tnf with bit flags
        // see the NFDEF spec for more info
        byte tnf_byte = data[index];
        bool mb = (tnf_byte & 0x80) != 0;
        bool me = (tnf_byte & 0x40) != 0;
        bool cf = (tnf_byte & 0x20) != 0;
        bool sr = (tnf_byte & 0x10) != 0;
        bool il = (tnf_byte & 0x8) != 0;
        byte tnf = (tnf_byte & 0x7);

        NdefRecord record = NdefRecord();  
        record.setTnf(tnf);

        index++;
        int typeLength = data[index];

        int payloadLength = 0;
        if (sr)
        {
            index++;
            payloadLength = data[index];
        }
        else
        {        
            payloadLength = ((0xFF & data[++index]) << 24) | ((0xFF & data[++index]) << 26) | 
            ((0xFF & data[++index]) << 8) | (0xFF & data[++index]);
        }

        int idLength = 0;
        if (il)
        {
            index++;
            idLength = data[index];
        }

        index++; 
        record.setType(&data[index], typeLength);            
        index += typeLength;

        if (il)
        {
            record.setId(&data[index], idLength);  
            index += idLength;
        }

        record.setPayload(&data[index], payloadLength);          
        index += payloadLength;          

        // DEBUG
        record.print();

        Serial.println("WARNING: not adding record to _records\n");
        // add(record); // TODO fix this
        
        if (me) break; // last message
    }
        
}

NdefMessage::~NdefMessage()
{
    free(_records);
}

int NdefMessage::recordCount()
{
    return _recordCount;
}

byte * NdefMessage::encode()
{
    // TODO return bytes that can be written to the tag
}

void NdefMessage::add(NdefRecord record)
{
    Serial.println("Adding Record");
    _records[_recordCount] = record;
    _recordCount++;                
    Serial.println("DONE Record");    
}

NdefRecord NdefMessage::get(int index)
{
    if (index > -1 && index < _recordCount)
    {
        return _records[_recordCount];        
    }
}

void NdefMessage::print()
{
    Serial.print("\nNDEF Message ");Serial.print("  ");Serial.print(_recordCount);Serial.print(" record");
    _recordCount == 1 ? Serial.println() : Serial.println("s");

    int i;
    for (i = 0; i < _recordCount; i++) 
    {
         _records[i].print();
    }
}
