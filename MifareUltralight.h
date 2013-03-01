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
		NfcTag read(uint8_t * uid, int uidLength);
	private:
		Adafruit_NFCShield_I2C* nfc;
		uint8_t tagCapacity;
		uint8_t messageLength;
		uint8_t bufferSize;
		uint8_t ndefStartIndex;
		boolean isUnformatted();
		void readCapabilityContainer();
		void findNdefMessage();
		void calculateBufferSize();
};

#endif