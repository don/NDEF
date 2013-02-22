#include <NfcAdapter.h>
// Drivers
#include <MifareClassic.h>
#include <MifareUltralight.h>

#include <NfcTag.h>

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
    Serial.print(F("Didn't find PN53x board"));
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print(F("Found chip PN5")); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print(F("Firmware ver. ")); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  shield->SAMConfig();
}

boolean NfcAdapter::tagPresent()
{
	uint8_t success;
  uidLength = 0; 

	success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  // if (success)
  // {
  //   Serial.println("Found an ISO14443A card");
  //   Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
  //   Serial.print("  UID Value: ");
  //   shield->PrintHex(uid, uidLength);
  //   Serial.println("");
  // }

  return success;
}

// TODO return a NfcTag
NdefMessage NfcAdapter::read() 
{

  NdefMessage ndefMessage;

	if (uidLength == 4)
  {
    Serial.println(F("Mifare Classic card (4 byte UID)"));

    // NfcTag tag = NfcTag(uid, uidLength, "Mifare Classic", ndefMessage);

    ndefMessage = readMifareClassic(*shield, uid, uidLength);
  }    
  else
  {
    // TODO need a better way to determine which driver to use
    // Since I have Mifare Classic cards with 7 byte UIDs
    Serial.println(F("Mifare Ultralight card (7 byte UID)"));

    MifareUltralight ultralight = MifareUltralight(*shield);
    ndefMessage = ultralight.read();
  }
  return ndefMessage;
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
    Serial.println(F("Unsupported Tag"));    
    success = false;
  }
  return success;
}