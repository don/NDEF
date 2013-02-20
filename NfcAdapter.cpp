#include <NfcAdapter.h>
// Drivers
#include <MifareClassic.h>
#include <MifareUltralight.h>

NfcAdapter::NfcAdapter(void)
{
	shield = new Adafruit_NFCShield_I2C(IRQ, RESET);
}

NfcAdapter::~NfcAdapter(void)
{
  delete shield;
}

void NfcAdapter::begin()
{
	shield->begin();

  uint32_t versiondata = shield->getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  shield->SAMConfig();
}

boolean NfcAdapter::tagPresent()
{
	uint8_t success;
	// reset uid and length
  //uid = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uidLength = 0;                    // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

	success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

	if (success)
	{
		Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    shield->PrintHex(uid, uidLength);
    Serial.println("");
	}
}

NdefMessage& NfcAdapter::read() 
{
	if (uidLength == 4)
  {
    Serial.println("Mifare Classic card (4 byte UID)");

    return readMifareClassic(*shield, uid, uidLength);
  }    
  else
  {
    // TODO need a better way to determine which driver to use
    // Since I have Mifare Classic cards with 7 byte UIDs
    Serial.println("Mifare Ultralight card (7 byte UID)");

    return readMifareUltralight(*shield);
  }
}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
  boolean success;

  if (uidLength == 4)
  {
    writeMifareClassic(*shield, ndefMessage, uid, uidLength);
    success = true; // TODO get status from write!
  }
  else
  {
    Serial.println("Unsupported Tag");    
    success = false;
  }
  return success;
}