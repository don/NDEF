#ifndef NfcAdapter_h
#define NfcAdapter_h

#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

/*
  • nfc.tagAvailable() -- returns tag UID when a tag is present
  • nfc.mimeTypeAvailable(char[] mimeType) --
  • nfc.ndefAvailable() -- returns the tag abstraction (presumably an NDEF or NDEFRecord) when an NDEF formatted tag is found
  • nfc.addNdefFormatableAvailable() -- returns tag UID when a tag is present
  • nfc.write()  -- based on Stream.write()
  * nfc.read() -- based on Stream.read()
  • nfc.share()
  • nfc.unshare()
  • nfc.erase()
*/

  class NfcAdapter {
  public:
    NfcAdapter(void);
    ~NfcAdapter(void); 
    void begin(void);
    boolean tagPresent(); // tagAvailable
    NdefMessage read();
  	boolean write(NdefMessage& ndefMessage);
  private:
    Adafruit_NFCShield_I2C* shield;
    uint8_t uid[7];    // Buffer to store the returned UID
    uint8_t uidLength; // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  };

#endif