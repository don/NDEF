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

NdefRecord::NdefRecord(const NdefRecord& rhs)
{
    _tnf = rhs._tnf;
    _typeLength = rhs._typeLength;
    _payloadLength = rhs._payloadLength;
    _idLength = rhs._idLength;

    _type = (uint8_t*)malloc(_typeLength);
    memcpy(_type, rhs._type, _typeLength);

    _payload = (uint8_t*)malloc(_payloadLength);
    memcpy(_payload, rhs._payload, _payloadLength);

    _id = (uint8_t*)malloc(_idLength);
    memcpy(_id, rhs._id, _idLength);    
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

NdefRecord& NdefRecord::operator=(const NdefRecord& rhs)
{
    if (this != &rhs)
    {
        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        // TODO need to free _type, _payload, and _id if they exist

        _type = (uint8_t*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);

        _payload = (uint8_t*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);

        _id = (uint8_t*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);    
    }
    return *this;
}

// size of records in bytes
int NdefRecord::getEncodedSize()
{
    int size = 2; // tnf + typeLength
    if (_payloadLength > 0xFF) 
    {
        size += 4;
    } 
    else 
    {
        size += 1;
    }
    
    if (_idLength)
    {
        size += 1;
    }

    size += (_typeLength + _payloadLength + _idLength);

    return size;
}

void NdefRecord::encode(uint8_t* data, bool firstRecord, bool lastRecord)
{
    // assert data > getEncodedSize()

    uint8_t* data_ptr = &data[0];

    *data_ptr = getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = _typeLength;
    data_ptr += 1;

    *data_ptr = _payloadLength; // TODO handle sr == false
    data_ptr += 1;
    if (_idLength)
    {
        *data_ptr = _idLength;
        data_ptr += 1;
    }

    //Serial.println(2);
    memcpy(data_ptr, _type, _typeLength);
    data_ptr += _typeLength;

    memcpy(data_ptr, _payload, _payloadLength);
    data_ptr += _payloadLength;

    if (_idLength)
    {
        memcpy(data_ptr, _id, _idLength);
        data_ptr += _idLength;
    }
}

uint8_t NdefRecord::getTnfByte(bool firstRecord, bool lastRecord)
{
    int value = _tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (_typeLength <= 0xFF) { // TODO test 0xFF on tag
        value = value | 0x10;
    }

    if (_idLength) {
        value = value | 0x8;
    }

    return value;
}

uint8_t NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(uint8_t tnf)
{
    _tnf = tnf;
}

uint8_t NdefRecord::getTypeLength()
{
    return _typeLength;
}

int NdefRecord::getPayloadLength()
{
    return _payloadLength;
}

uint8_t NdefRecord::getIdLength()
{
    return _idLength;
}

// TODO don't return an array we created
// NdefRecord::getType(uint8_t * type) and copy into their array
// OR return an object that has the type and the array
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
    Serial.print("    Record is ");Serial.print(getEncodedSize());Serial.println(" bytes");

}

NdefMessage::NdefMessage(void)
{
    _recordCount = 0;       
}

NdefMessage::NdefMessage(byte * data, const int numBytes)
{
    Serial.print("Decoding ");Serial.print(numBytes);Serial.println(" bytes");    
    PrintHex(data, numBytes);
    // _records = (NdefRecord*)malloc(sizeof(NdefRecord *) * MAX_NDEF_RECORDS);
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

        add(record);
        
        if (me) break; // last message
    }
        
}

NdefMessage::~NdefMessage()
{
    //free(_records);
}

int NdefMessage::recordCount()
{
    return _recordCount;
}

int NdefMessage::getEncodedSize()
{
    int size = 0;
    int i;
    for (i = 0; i < _recordCount; i++) 
    {
        size += _records[i].getEncodedSize();
    }
    return size;
}

void NdefMessage::encode(uint8_t* data)
{
    // assert sizeof(data) >= getEncodedSize()
    uint8_t* data_ptr = &data[0];

    int i;
    for (i = 0; i < _recordCount; i++) 
    {    
        _records[i].encode(data_ptr, i == 0, (i + 1) == _recordCount);
        // TODO can NdefRecord.encode return the record size?        
        data_ptr += _records[i].getEncodedSize(); 
    }

    // TODO don't print here
    Serial.println("\nEncoded");
    PrintHex(data, getEncodedSize());
}

void NdefMessage::add(NdefRecord record)
{

    if (_recordCount < MAX_NDEF_RECORDS)
    {
        _records[_recordCount] = record;
        _recordCount++;                
    }
    else
    {
        // TODO consider returning status
        Serial.println("WARNING: Too many records. Increase MAX_NDEF_RECORDS.");
    }
}

// TODO would NdefRecord::mimeMediaRecord be better?
void NdefMessage::addMimeMediaRecord(String mimeType, String payload)
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_MIME_MEDIA);

    byte type[mimeType.length() + 1];
    mimeType.getBytes(type, sizeof(type));
    r->setType(type, mimeType.length());

    byte payloadBytes[payload.length() + 1];
    payload.getBytes(payloadBytes, sizeof(payloadBytes));
    r->setPayload(payloadBytes, payload.length());

    add(*r);

    /*
    TODO call other method
    byte payloadBytes[payload.length() + 1];
    payload.getBytes(payloadBytes, sizeof(payloadBytes));
    
    addMimeMediaRecord(mimeType, payloadBytes, payload.length);
    */
}

void NdefMessage::addMimeMediaRecord(String mimeType, uint8_t* payload, int payloadLength)
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_MIME_MEDIA);

    byte type[mimeType.length() + 1];
    mimeType.getBytes(type, sizeof(type));
    r->setType(type, mimeType.length());

    r->setPayload(payload, payloadLength);

    add(*r);
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
    Serial.print("\nNDEF Message ");Serial.print(_recordCount);Serial.print(" record");
    _recordCount == 1 ? Serial.print(", ") : Serial.print("s, ");
    Serial.print(getEncodedSize());Serial.println(" bytes");

    int i;
    for (i = 0; i < _recordCount; i++) 
    {
         _records[i].print();
    }
}
