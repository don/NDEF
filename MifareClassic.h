#ifndef MifareClassic_h
#define MifareClassic_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>
#include <NfcTag.h>

class MifareClassic
{
	public:
		MifareClassic();
		~MifareClassic();
		NfcTag read(Adafruit_NFCShield_I2C& nfcShield, uint8_t * uid, int uidLength); 
		boolean write(Adafruit_NFCShield_I2C& nfc, NdefMessage& ndefMessage, uint8_t * uid, int uidLength); 
	private:
		int getBufferSize(int messageLength);
		int getNdefLength(byte *data);
};

#endif