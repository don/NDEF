#include <NdefMessage.h>

NdefMessage::NdefMessage(void)
{
    _recordCount = 0;
}

NdefMessage::NdefMessage(const byte * data, const uint16_t numBytes)
{
    #ifdef NDEF_DEBUG
    Serial.print(F("Decoding "));Serial.print(numBytes);Serial.println(F(" bytes"));
    PrintHexChar(data, numBytes);
    //DumpHex(data, numBytes, 16);
    #endif

    _recordCount = 0;

    uint16_t index = 0;

    while (index <= numBytes)
    {

        // decode tnf - first byte is tnf with bit flags
        // see the NFDEF spec for more info
        byte tnf_byte = data[index];
        // bool mb = tnf_byte & 0x80;
        bool me = tnf_byte & 0x40;
        // bool cf = tnf_byte & 0x20;
        bool sr = tnf_byte & 0x10;
        bool il = tnf_byte & 0x8;
        byte tnf = (tnf_byte & 0x7);

        NdefRecord record = NdefRecord();
        record.setTnf(tnf);

        index++;
        uint8_t typeLength = data[index];

        uint32_t payloadLength = 0;
        if (sr)
        {
            index++;
            payloadLength = data[index];
        }
        else
        {
            payloadLength =
                  (static_cast<uint32_t>(data[index])   << 24)
                | (static_cast<uint32_t>(data[index+1]) << 16)
                | (static_cast<uint32_t>(data[index+2]) << 8)
                |  static_cast<uint32_t>(data[index+3]);
            index += 4;
        }

        uint8_t idLength = 0;
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

        addRecord(record);

        if (me) break; // last message
    }

}

NdefMessage::NdefMessage(const NdefMessage& rhs)
{

    _recordCount = rhs._recordCount;
    for (uint8_t i = 0; i < _recordCount; i++)
    {
        _records[i] = rhs._records[i];
    }

}

NdefMessage::~NdefMessage()
{
}

NdefMessage& NdefMessage::operator=(const NdefMessage& rhs)
{

    if (this != &rhs)
    {

        // delete existing records
        for (uint8_t i = 0; i < _recordCount; i++)
        {
            // TODO Dave: is this the right way to delete existing records?
            _records[i] = NdefRecord();
        }

        _recordCount = rhs._recordCount;
        for (uint8_t i = 0; i < _recordCount; i++)
        {
            _records[i] = rhs._records[i];
        }
    }
    return *this;
}

uint8_t NdefMessage::getRecordCount()
{
    return _recordCount;
}

uint16_t NdefMessage::getEncodedSize()
{
    uint16_t size = 0;
    for (uint8_t i = 0; i < _recordCount; i++)
    {
        size += _records[i].getEncodedSize();
    }
    return size;
}

// TODO change this to return uint8_t*
void NdefMessage::encode(uint8_t* data)
{
    // assert sizeof(data) >= getEncodedSize()
    uint8_t* data_ptr = &data[0];
    uint16_t offset = getHeaderSize();

    for (uint8_t i = 0; i < _recordCount; i++)
    {
        _records[i].encode(data_ptr, i == 0, (i + 1) == _recordCount);

        uint16_t encodedSize = _records[i].getEncodedSize();
        _offsets[i] = offset + encodedSize - _records[i].getPayloadLength();

        offset += encodedSize;
        data_ptr += encodedSize;
    }

}


uint16_t NdefMessage::getHeaderSize() {
    return 2 + (getEncodedSize() > 254 ? 2 : 0);
    // TLV is 0x03 + 1 byte length 
    // OR if size > 254, 0x03 + 3 byte length
    // See getHeader()
}

void NdefMessage::getHeader(byte* header)
{
    uint16_t payloadLength = getEncodedSize();
    bool lengthy = payloadLength > 254;
    header[0] = 0x3;
    if (lengthy) {
        header[1] = 0xFF;
        header[2] = payloadLength >> 8;
        header[3] = payloadLength;
    } else {
        header[1] = payloadLength;
    }
}

uint16_t NdefMessage::getPackagedSize()
{
    return getEncodedSize() + getHeaderSize() + 1;
}


void NdefMessage::getPackaged(uint8_t *data)
{
    uint16_t size = getPackagedSize();

    getHeader(data); 
    encode(&data[getHeaderSize()]);
    data[size-1] = 0xFE;                // Termination byte for TLV
}


boolean NdefMessage::addRecord(NdefRecord& record)
{

    if (_recordCount < MAX_NDEF_RECORDS)
    {
        _records[_recordCount] = record;
        _recordCount++;
        return true;
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        Serial.println(F("WARNING: Too many records. Increase MAX_NDEF_RECORDS."));
#endif
        return false;
    }
}

void NdefMessage::addMimeMediaRecord(const char *mimeType, const char* payload)
{
	addMimeMediaRecord(mimeType, reinterpret_cast<const byte*>(payload), strlen(payload));
}

void NdefMessage::addMimeMediaRecord(const char *mimeType, const byte* payload, uint16_t payloadLength)
{
    NdefRecord  r;
    r.setTnf(TNF_MIME_MEDIA);

    r.setType(reinterpret_cast<const byte*>(mimeType), strlen(mimeType));
    r.setPayload(payload, payloadLength);
    addRecord(r);

}

void NdefMessage::addUnknownRecord(const byte *payload, int payloadLength)
{
    NdefRecord  r;
    r.setTnf(TNF_UNKNOWN);

    r.setType(payload, 0);
    r.setPayload(payload, payloadLength);
    addRecord(r);

}


void NdefMessage::addTextRecord(const char *text)
{
    addTextRecord(text, "");
}

void NdefMessage::addTextRecord(const char *text, const char *encoding)
{
    NdefRecord  r;
    r.setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_TEXT[1] = { 0x54 }; // TODO this should be a constant or preprocessor
    r.setType(RTD_TEXT, sizeof(RTD_TEXT));

    // encoding length
	const uint8_t prefixSize = 5;
	byte prefix[prefixSize];
	byte encodingSize = strlen(encoding);
	prefix[0] = encodingSize;
	for (uint8_t i=0; encoding[i] && (i+1) < prefixSize; ++i) // limit encoding to max 4 bytes
		prefix[i+1] = encoding[i];

	// set payload	
    r.setPayload(prefix, prefixSize, reinterpret_cast<const byte*>(text), strlen(text));

    addRecord(r);
}

void NdefMessage::addUriRecord(const char *uri)
{
    NdefRecord  r;
    r.setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_URI[1] = { 0x55 }; // TODO this should be a constant or preprocessor
    r.setType(RTD_URI, sizeof(RTD_URI));

    // encoding prefix
	const uint8_t prefixSize = 1;
    byte prefix[prefixSize] = {0};

    // set payload
    r.setPayload(prefix, prefixSize, reinterpret_cast<const byte*>(uri), strlen(uri));

    addRecord(r);
}


void NdefMessage::addExternalRecord(const char *type,const char* payload)
{
	addExternalRecord(type, reinterpret_cast<const byte*>(payload), strlen(payload));
}

void NdefMessage::addExternalRecord(const char *type, const byte *payload, uint16_t payloadLength)
{
	NdefRecord  r;
	r.setTnf(TNF_EXTERNAL_TYPE);
	
	r.setType(reinterpret_cast<const byte*>(type), strlen(type));
    r.setPayload(payload, payloadLength);
	addRecord(r);
}


void NdefMessage::addAndroidApplicationRecord(const char *packageName)
{
	addExternalRecord("android.com:pkg", packageName);
}

void NdefMessage::addEmptyRecord()
{
    NdefRecord  r;
    r.setTnf(TNF_EMPTY);
    addRecord(r);
}

NdefRecord NdefMessage::getRecord(uint8_t index)
{
    if (index > -1 && index < static_cast<int>(_recordCount))
    {
        return _records[index];
    }
    else
    {
        return NdefRecord(); // would rather return NULL
    }
}

uint16_t NdefMessage::getOffset(uint8_t index)
{
    if (index < _recordCount) {
        return _offsets[index];
    }
    return 0;
}

NdefRecord NdefMessage::operator[](uint8_t index)
{
    return getRecord(index);
}

#ifdef NDEF_USE_SERIAL
void NdefMessage::print()
{
    Serial.print(F("\nNDEF Message "));Serial.print(_recordCount);Serial.print(F(" record"));
    _recordCount == 1 ? Serial.print(", ") : Serial.print("s, ");
    Serial.print(getEncodedSize());Serial.println(F(" bytes"));

    for (unsigned int i = 0; i < _recordCount; i++)
    {
         _records[i].print();
    }
}
#endif
