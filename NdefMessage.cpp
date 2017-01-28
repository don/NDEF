#include <NdefMessage.h>

NdefMessage::NdefMessage(void)
{
    _recordCount = 0;
}

NdefMessage::NdefMessage(const byte * data, const int numBytes)
{
    #ifdef NDEF_DEBUG
    Serial.print(F("Decoding "));Serial.print(numBytes);Serial.println(F(" bytes"));
    PrintHexChar(data, numBytes);
    //DumpHex(data, numBytes, 16);
    #endif

    _recordCount = 0;

    int index = 0;

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
        int typeLength = data[index];

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

        addRecord(record);

        if (me) break; // last message
    }

}

NdefMessage::NdefMessage(const NdefMessage& rhs)
{

    _recordCount = rhs._recordCount;
    for (unsigned int i = 0; i < _recordCount; i++)
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
        for (unsigned int i = 0; i < _recordCount; i++)
        {
            // TODO Dave: is this the right way to delete existing records?
            _records[i] = NdefRecord();
        }

        _recordCount = rhs._recordCount;
        for (unsigned int i = 0; i < _recordCount; i++)
        {
            _records[i] = rhs._records[i];
        }
    }
    return *this;
}

unsigned int NdefMessage::getRecordCount()
{
    return _recordCount;
}

int NdefMessage::getEncodedSize()
{
    int size = 0;
    for (unsigned int i = 0; i < _recordCount; i++)
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

    for (unsigned int i = 0; i < _recordCount; i++)
    {
        _records[i].encode(data_ptr, i == 0, (i + 1) == _recordCount);
        // TODO can NdefRecord.encode return the record size?
        data_ptr += _records[i].getEncodedSize();
    }

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

void NdefMessage::addMimeMediaRecord(char *mimeType, char* payload)
{
	addMimeMediaRecord(mimeType, reinterpret_cast<byte*>(payload), strlen(payload));
}

void NdefMessage::addMimeMediaRecord(char *mimeType, byte* payload, int payloadLength)
{
    NdefRecord  r;
    r.setTnf(TNF_MIME_MEDIA);

    r.setType(reinterpret_cast<byte*>(mimeType), strlen(mimeType));
    r.setPayload(payload, payloadLength);
    addRecord(r);

}

void NdefMessage::addUnknownRecord(byte *payload, int payloadLength)
{
    NdefRecord  r;
    r.setTnf(TNF_UNKNOWN);

    r.setType(payload, 0);
    r.setPayload(payload, payloadLength);
    addRecord(r);

}


void NdefMessage::addTextRecord(char *text)
{
    addTextRecord(text, "");
}

void NdefMessage::addTextRecord(char *text, char *encoding)
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
    r.setPayload(prefix, prefixSize, reinterpret_cast<byte*>(text), strlen(text));

    addRecord(r);
}

void NdefMessage::addUriRecord(char *uri)
{
    NdefRecord  r;
    r.setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_URI[1] = { 0x55 }; // TODO this should be a constant or preprocessor
    r.setType(RTD_URI, sizeof(RTD_URI));

    // encoding prefix
	const uint8_t prefixSize = 1;
    byte prefix[prefixSize] = {0};

    // set payload
    r.setPayload(prefix, prefixSize, reinterpret_cast<byte*>(uri), strlen(uri));

    addRecord(r);
}

void NdefMessage::addAndroidApplicationRecord(char *packageName)
{
    NdefRecord  r;
    r.setTnf(TNF_EXTERNAL_TYPE);

    char *RTD_AAR = "android.com:pkg"; // TODO this should be a constant or preprocessor
    r.setType((uint8_t *)RTD_AAR, strlen(RTD_AAR));

    r.setPayload((uint8_t *)packageName, strlen(packageName));

    addRecord(r);
}

void NdefMessage::addEmptyRecord()
{
    NdefRecord  r;
    r.setTnf(TNF_EMPTY);
    addRecord(r);
}

NdefRecord NdefMessage::getRecord(int index)
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

NdefRecord NdefMessage::operator[](int index)
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
