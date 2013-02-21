// TODO pass Adafruit_NFCShield_I2C& nfc in the constructor or read/write?
// TODO how to handle different arguments? MifareClassic needs uint8_t * uid, int uidLength)
class NfcDriver
{
public:
	virtual NdefMessage& read(void) = 0;
	virtual boolean write(NdefMessage& message) = 0;
}