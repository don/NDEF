#ifndef NfcTag_h
#define NfcTag_h

#include <inttypes.h>
#include <Arduino.h>
#include <Ndef.h>

class NfcTag
{
	public:
		//NfcTag(uint8_t* uid, uint8_t uidLength, String tech);
		NfcTag(uint8_t* uid, uint8_t uidLength, String tech, NdefMessage& ndefMessage);
		~NfcTag(void);
		uint8_t getUidLength();
		void getUid(uint8_t* uid, uint8_t uidLength);
		String getUidString();
		String getTechnology();
		NdefMessage& getNdefMessage();
	private:
		uint8_t* _uid;
		uint8_t _uidLength;
		String _technology; // Mifare Classic, Mifare Ultralight, etc
		NdefMessage* _ndefMessage;
};

#endif