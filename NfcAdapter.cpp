#include <NfcAdapter.h>

NfcAdapter::NfcAdapter(PN532Interface &interface)
{
    shield = new PN532(interface);
}

NfcAdapter::~NfcAdapter(void)
{
    delete shield;
}

void NfcAdapter::begin(boolean verbose)
{
    shield->begin();

    uint32_t versiondata = shield->getFirmwareVersion();
    if (! versiondata) {
        Serial.print(F("Didn't find PN53x board"));
        while (1); // halt
    }
    if(verbose) {
	    Serial.print(F("Found chip PN5")); Serial.println((versiondata>>24) & 0xFF, HEX);
    	Serial.print(F("Firmware ver. ")); Serial.print((versiondata>>16) & 0xFF, DEC);
    	Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
	}
    // configure board to read RFID tags
    shield->SAMConfig();
}

boolean NfcAdapter::tagPresent(unsigned long timeout)
{
    uint8_t success;
    uidLength = 0;

    // TODO is cast of uidLength OK?
    if(timeout == 0)
    	success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, (uint8_t*)&uidLength);
    else
    	success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, (uint8_t*)&uidLength,timeout);
    return success;
}

boolean NfcAdapter::format() {
  boolean success;
  if (uidLength == 4)
  {
    MifareClassic mifareClassic = MifareClassic(*shield);
    success = mifareClassic.formatNDEF(uid,uidLength);
  } else {
    Serial.print(F("Unsupported Tag. Length is "));Serial.println(uidLength);
    success = false;
  }
  return success;
}

boolean NfcAdapter::erase() {
  boolean success;
  if (uidLength == 4)
  {
    MifareClassic mifareClassic = MifareClassic(*shield);
    success = mifareClassic.formatMifare(uid,uidLength);
  } else {
    Serial.print(F("Unsupported Tag. Length is "));Serial.println(uidLength);
    success = false;
  }
  return success;
}

NfcTag NfcAdapter::read()
{

    uint8_t type = guessTagType();

    // TODO need an abstraction of Driver
    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        Serial.print(F("Can not determine tag type"));
        //Serial.print(F("Can not determine tag type for ATQA 0x"));
        //Serial.print(atqa, HEX);Serial.print(" SAK 0x");Serial.println(sak, HEX);
        return NfcTag(uid, uidLength);
    }
    else
    {
        Serial.print(F("No driver for card type "));Serial.println(type);
        // TODO should set type here
        return NfcTag(uid, uidLength);
    }

}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
    boolean success;    
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
#ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Ultralight"));
#endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        success=ultralight.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        Serial.print(F("Can not determine tag type"));
        success=false;
    }
    else
    {
        Serial.print(F("No driver for card type "));Serial.println(type);
        // TODO should set type here
        success=false;
    }

    return success;
}

// TODO this should return a Driver MifareClassic, MifareUltralight, Type 4, Unknown
// Guess Tag Type by looking at the ATQA and SAK values
// Need to follow spec for Card Identification. Maybe AN1303, AN1305 and ???
unsigned int NfcAdapter::guessTagType()
{

    // 4 byte id - Mifare Classic
    //  - ATQA 0x4 && SAK 0x8
    // 7 byte id
    //  - ATQA 0x44 && SAK 0x8 - Mifare Classic
    //  - ATQA 0x44 && SAK 0x0 - Mifare Ultralight NFC Forum Type 2
    //  - ATQA 0x344 && SAK 0x20 - NFC Forum Type 4

    if (uidLength == 4)
    {
        return TAG_TYPE_MIFARE_CLASSIC;
    }
    else
    {
        return TAG_TYPE_2;
    }
}
