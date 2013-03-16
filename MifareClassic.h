#ifndef MifareClassic_h
#define MifareClassic_h

#include <Due.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>
#include <NfcTag.h>

class MifareClassic
{
	public:
		MifareClassic(Adafruit_NFCShield_I2C& nfcShield);
		~MifareClassic();
		NfcTag read(uint8_t * uid, int uidLength); 
		boolean write(NdefMessage& ndefMessage, uint8_t * uid, int uidLength); 
	private:
		Adafruit_NFCShield_I2C* _nfcShield;
		int getBufferSize(int messageLength);
		int getNdefLength(byte *data);
};

#endif