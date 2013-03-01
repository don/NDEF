#include <NfcTag.h>

NfcTag::NfcTag()
{
	_uid = 0;
	_uidLength = 0;
	_tagType = "Unknown";
	_ndefMessage = (NdefMessage*)NULL;	
}

NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength)
{
	_uid = uid;
	_uidLength = uidLength;
	_tagType = "Unknown";
	_ndefMessage = (NdefMessage*)NULL;	
}

NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tagType)
{
	_uid = uid;
	_uidLength = uidLength;
	_tagType = tagType;
	_ndefMessage = (NdefMessage*)NULL;	
}

NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tagType, NdefMessage& ndefMessage)
{
	_uid = uid;
	_uidLength = uidLength;
	_tagType = tagType;
	_ndefMessage = new NdefMessage(ndefMessage);	
}

// I don't like this version, but it will use less memory
NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tagType, const uint8_t * ndefData, const int ndefDataLength)
{
	_uid = uid;
	_uidLength = uidLength;
	_tagType = tagType;
	_ndefMessage = new NdefMessage(ndefData, ndefDataLength);	
}

NfcTag::~NfcTag()
{
	delete _ndefMessage;
}

uint8_t NfcTag::getUidLength()
{
	return _uidLength;
}

void NfcTag::getUid(uint8_t* uid, uint8_t uidLength)
{
	memcpy(_uid, uid, uidLength);
}

String NfcTag::getUidString()
{
	String uidString = "";
	for (int i = 0; i < _uidLength; i++)
	{
		if (i > 0) 
		{ 
			uidString += " "; 
		}

		if (_uid[i] < 0xF) 
		{
			uidString += "0"; 
		}

		uidString += String(_uid[i], HEX);
	}
	uidString.toUpperCase();
	return uidString;
}

String NfcTag::getTagType()
{
	return _tagType;
}

NdefMessage NfcTag::getNdefMessage()
{
	return *_ndefMessage;
}

void NfcTag::print()
{
	Serial.print("NFC Tag - ");Serial.print(_tagType);
	Serial.print("UID ");Serial.println(getUidString());
	// TODO ensure message exists
	_ndefMessage->print();
}
