#include <NfcTag.h>

// NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tech)
// {
// 	_uid = uid;
// 	_uidLength = uidLength;
// 	_technology = tech;
// }

NfcTag::NfcTag(uint8_t* uid, uint8_t uidLength, String tech, NdefMessage& ndefMessage)
{
	_uid = uid;
	_uidLength = uidLength;
	_technology = tech;
	_ndefMessage = &ndefMessage;	
}

// TODO Dave: should this class be deleting _ndefMessage?
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
	int i;
	for (i = 0; i < _uidLength; i++)
	{
		uidString += String(_uid[i], HEX);
	}
}

String NfcTag::getTechnology()
{
	return _technology;
}

NdefMessage& NfcTag::getNdefMessage()
{
	return *_ndefMessage;
}
