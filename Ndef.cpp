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

// Borrowed from Adafruit_NFCShield_I2C
void PrintHexChar(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print("0");
    Serial.print(data[szPos], HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(" ");
    }
  }
  Serial.print("  ");
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    if (data[szPos] <= 0x1F)
      Serial.print(".");
    else
      Serial.print((char)data[szPos]);
  }
  Serial.println("");
}

// Note if buffer % blockSize != 0, last block will not be written
void DumpHex(const byte * data, const uint32_t numBytes, const uint8_t blockSize)
{
    int i;
    for (i = 0; i < (numBytes / blockSize); i++)    
    {
        PrintHexChar(data, blockSize);
        data += blockSize;
    }
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

void NdefRecord::getType(uint8_t* type)
{
    // assert sizeof(type) >= sizeof(_type)
    memcpy(type, _type, sizeof(_type));
}

void NdefRecord::setType(uint8_t * type, const int numBytes)
{
    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

void NdefRecord::getPayload(uint8_t * payload)
{
    memcpy(payload, _payload, sizeof(_payload));
}

void NdefRecord::setPayload(uint8_t * payload, const int numBytes)
{
    _payload = (uint8_t*)malloc(numBytes);    
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

void NdefRecord::getId(uint8_t * id)
{
    memcpy(id, _id, sizeof(_id));
}

void NdefRecord::setId(uint8_t * id, const int numBytes)
{
 
    _id = (uint8_t*)malloc(numBytes);    
    memcpy(_id, id, numBytes);
    _idLength = numBytes;
}

/*
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
*/

void NdefRecord::print()
{
    Serial.println("  NDEF Record");    
    Serial.print("    TNF 0x");Serial.print(_tnf, HEX);Serial.print(" ");
    switch (_tnf) {
    case TNF_EMPTY:
        Serial.println("Empty");
        break; 
    case TNF_WELL_KNOWN:
        Serial.println("Well Known");
        break;     
    case TNF_MIME_MEDIA:
        Serial.println("Mime Media");
        break;     
    case TNF_ABSOLUTE_URI:
        Serial.println("Absolute URI");
        break;     
    case TNF_EXTERNAL_TYPE:
        Serial.println("External");
        break;     
    case TNF_UNKNOWN:
        Serial.println("Unknown");
        break;     
    case TNF_UNCHANGED:
        Serial.println("Unchanged");
        break;     
    case TNF_RESERVED:
        Serial.println("Reserved");
        break;
    default:
        Serial.println();     
    }
    Serial.print("    Type Length 0x");Serial.print(_typeLength, HEX);Serial.print(" ");Serial.println(_typeLength);
    Serial.print("    Payload Length 0x");Serial.print(_payloadLength, HEX);;Serial.print(" ");Serial.println(_payloadLength);    
    if (_idLength)
    {
        Serial.print("    Id Length 0x");Serial.println(_idLength, HEX);  
    }
    Serial.print("    Type ");PrintHexChar(_type, _typeLength);  
    // TODO chunk large payloads so this is readable
    Serial.print("    Payload ");PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print("    Id ");PrintHexChar(_id, _idLength);  
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
    //PrintHex(data, numBytes);
    DumpHex(data, numBytes, 16);

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

    Serial.println("\nEncoded");
    PrintHex(data, getEncodedSize());
}

// TODO Dave: should the be NdefRecord& to avoid a copy when calling?
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

void NdefMessage::addMimeMediaRecord(String mimeType, String payload)
{

    byte payloadBytes[payload.length() + 1];
    payload.getBytes(payloadBytes, sizeof(payloadBytes));
    
    addMimeMediaRecord(mimeType, payloadBytes, payload.length());
}

void NdefMessage::addMimeMediaRecord(String mimeType, uint8_t* payload, int payloadLength)
{
    NdefRecord r = NdefRecord();
    r.setTnf(TNF_MIME_MEDIA);

    byte type[mimeType.length() + 1];
    mimeType.getBytes(type, sizeof(type));
    r.setType(type, mimeType.length());

    r.setPayload(payload, payloadLength);

    add(r);
}

void NdefMessage::addTextRecord(String text)
{
    addTextRecord(text, "en");
}

void NdefMessage::addTextRecord(String text, String encoding)
{
    NdefRecord r = NdefRecord();
    r.setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_TEXT[1] = { 0x54 }; // TODO this should be a constant or preprocessor
    r.setType(RTD_TEXT, sizeof(RTD_TEXT));

    // X is a placeholder for encoding length
    String payloadString = "X" + encoding + text;

    byte payload[payloadString.length() + 1];
    payloadString.getBytes(payload, sizeof(payload));

    // replace X with the real encoding length
    payload[0] = encoding.length();

    r.setPayload(payload, payloadString.length());

    add(r);
}

void NdefMessage::addUriRecord(String uri)
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_URI[1] = { 0x55 }; // TODO this should be a constant or preprocessor
    r->setType(RTD_URI, sizeof(RTD_URI));

    // X is a placeholder for identifier code
    String payloadString = "X" + uri;

    byte payload[payloadString.length() + 1];
    payloadString.getBytes(payload, sizeof(payload));

    // add identifier code 0x0, meaning no prefix substitution
    payload[0] = 0x0;

    r->setPayload(payload, payloadString.length());

    add(*r);
    delete(r);
}

void NdefMessage::addEmptyRecord()
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_EMPTY);
    add(*r);
    delete(r);
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


