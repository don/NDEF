#ifndef MifareUltralight_h
#define MifareUltralight_h

#include <Adafruit_NFCShield_I2C.h>
#include <NfcTag.h>
#include <Ndef.h>

class MifareUltralight
{
	public:
		MifareUltralight(Adafruit_NFCShield_I2C& nfcShield);
		~MifareUltralight();
		NfcTag read(byte *uid, unsigned int uidLength);
	private:
		Adafruit_NFCShield_I2C* nfc;
		unsigned int tagCapacity;
		unsigned int messageLength;
		unsigned int bufferSize;
		unsigned int ndefStartIndex;
		boolean isUnformatted();
		void readCapabilityContainer();
		void findNdefMessage();
		void calculateBufferSize();
};

#endif