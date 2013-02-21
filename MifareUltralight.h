#ifndef MifareUltralight_h
#define MifareUltralight_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

//void write(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength);
NdefMessage readMifareUltralight(Adafruit_NFCShield_I2C& nfc);

//int getBufferSize(int messageLength);
//int getNdefLength(byte *data);
class MifareUltralight
{
	public:
		MifareUltralight(Adafruit_NFCShield_I2C& nfcShield);
		~MifareUltralight();
		NdefMessage read(); // TODO return NfcTag
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