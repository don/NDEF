#include "NdefRecord.h"

NdefRecord::NdefRecord()
{   
    //Serial.println("NdefRecord Constructor 1");
    _tnf = 0;
    _typeLength = 0;
    _payloadLength = 0;    
    _idLength = 0;
    _type = (uint8_t *)NULL;
    _payload = (uint8_t *)NULL;
    _id = (uint8_t *)NULL;
}

NdefRecord::NdefRecord(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord Constructor 2 (copy)");

    _tnf = rhs._tnf;
    _typeLength = rhs._typeLength;
    _payloadLength = rhs._payloadLength;
    _idLength = rhs._idLength;
    _type = (uint8_t *)NULL;
    _payload = (uint8_t *)NULL;
    _id = (uint8_t *)NULL;

    if (_typeLength)
    {
        _type = (uint8_t*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (uint8_t*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (uint8_t*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);    
    }  

}

// TODO NdefRecord::NdefRecord(tnf, type, payload, id)

NdefRecord::~NdefRecord()
{
    //Serial.println("NdefRecord Destructor");
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
    //Serial.println("NdefRecord ASSIGN");

    if (this != &rhs)
    {
        // free existing
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

        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        if (_typeLength)
        {
            _type = (uint8_t*)malloc(_typeLength);
            memcpy(_type, rhs._type, _typeLength);
        }

        if (_payloadLength)
        {
            _payload = (uint8_t*)malloc(_payloadLength);
            memcpy(_payload, rhs._payload, _payloadLength);
        }

        if (_idLength)
        {
            _id = (uint8_t*)malloc(_idLength);
            memcpy(_id, rhs._id, _idLength);    
        }
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

String NdefRecord::getType()
{
    // TODO is there a simpler way to do this?
    char* type = (char*)malloc(_typeLength);
    memset(type, 0, sizeof(type));    
    memcpy(type, _type, _typeLength);
    String typeString = String(type);
    free(type);
    return typeString;
}

// this assumes the caller created type correctly
void NdefRecord::getType(uint8_t* type)
{
    memcpy(type, _type, _typeLength);
}

void NdefRecord::setType(const uint8_t * type, const uint8_t numBytes)
{
    if(_typeLength)
    {
        free(_type);
    }

    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

// caller is responsible for deleting array
uint8_t* NdefRecord::getPayload()
{
    uint8_t* payload = (uint8_t*)malloc(_payloadLength);
    memcpy(payload, _payload, _payloadLength);
    return payload;
}

// assumes the caller sized payload properly
void NdefRecord::getPayload(uint8_t* payload)
{
    memcpy(payload, _payload, _payloadLength);
}

void NdefRecord::setPayload(const uint8_t * payload, const int numBytes)
{
    if (_payloadLength)
    {
        free(_payload);
    }

    _payload = (uint8_t*)malloc(numBytes);
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

String NdefRecord::getId()
{
    // TODO is there a simpler way to do this?
    char* id = (char*)malloc(_idLength);
    memset(id, 0, sizeof(id));    
    memcpy(id, _id, _idLength);
    String idString = String(id);
    free(id);
    return idString;
}

void NdefRecord::getId(uint8_t * id)
{
    memcpy(id, _id, _idLength);
}

void NdefRecord::setId(const uint8_t * id, const uint8_t numBytes)
{
    if (_idLength)
    {
        free(_id);
    }
 
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
    Serial.println(F("  NDEF Record"));    
    Serial.print(F("    TNF 0x"));Serial.print(_tnf, HEX);Serial.print(" ");
    switch (_tnf) {
    case TNF_EMPTY:
        Serial.println(F("Empty"));
        break; 
    case TNF_WELL_KNOWN:
        Serial.println(F("Well Known"));
        break;     
    case TNF_MIME_MEDIA:
        Serial.println(F("Mime Media"));
        break;     
    case TNF_ABSOLUTE_URI:
        Serial.println(F("Absolute URI"));
        break;     
    case TNF_EXTERNAL_TYPE:
        Serial.println(F("External"));
        break;     
    case TNF_UNKNOWN:
        Serial.println(F("Unknown"));
        break;     
    case TNF_UNCHANGED:
        Serial.println(F("Unchanged"));
        break;     
    case TNF_RESERVED:
        Serial.println(F("Reserved"));
        break;
    default:
        Serial.println();     
    }
    Serial.print(F("    Type Length 0x"));Serial.print(_typeLength, HEX);Serial.print(" ");Serial.println(_typeLength);
    Serial.print(F("    Payload Length 0x"));Serial.print(_payloadLength, HEX);;Serial.print(" ");Serial.println(_payloadLength);    
    if (_idLength)
    {
        Serial.print(F("    Id Length 0x"));Serial.println(_idLength, HEX);  
    }
    Serial.print(F("    Type "));PrintHexChar(_type, _typeLength);  
    // TODO chunk large payloads so this is readable
    Serial.print(F("    Payload "));PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id "));PrintHexChar(_id, _idLength);  
    }
    Serial.print(F("    Record is "));Serial.print(getEncodedSize());Serial.println(" bytes");

}