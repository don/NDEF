#include <NfcTag.h>

NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tagType, NdefMessage& ndefMessage)
{
	_uid = uid;
	_uidLength = uidLength;
	_tagType = tagType;
	_ndefMessage = &ndefMessage;	
}

NfcTag::~NfcTag()
{
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
	int i;
	for (i = 0; i < _uidLength; i++)
	{
		uidString += String(_uid[i], HEX);
	}
}

String NfcTag::getTagType()
{
	return _tagType;
}

NdefMessage& NfcTag::getNdefMessage()
{
	return *_ndefMessage;
}

void NfcTag::print()
{
	Serial.println("********");
	Serial.print("NFC Tag (");Serial.print(_tagType);Serial.println(")");
	Serial.print("uid ");Serial.println(getUidString());
	// TODO ensure message exists
	_ndefMessage->print();
}
