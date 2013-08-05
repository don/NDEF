#if 0
#include <SPI.h>
#include <PN532SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
#else

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
#endif

void setup(void) {
  Serial.begin(115200);
  Serial.println("NDEF Reader");
  nfc.begin();
}

void loop(void) {
  Serial.println("\nScan a NFC tag\n");
  if (nfc.tagPresent()) 
  {
    NfcTag tag = nfc.read();
    tag.print();

    if (tag.hasNdefMessage()) // every tag won't have a message        
    {
      NdefMessage message = tag.getNdefMessage();
      // message.print();  // message has a print method (this is more like debug)

      // cycle through the records, printing some info from each
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++) 
      {
        NdefRecord record = message.getRecord(i);  
        // NdefRecord record = message[i]; // alternate syntax

        Serial.println(record.getTnf()); 
        Serial.println(record.getType()); // will be "" for TNF_EMPTY

        // we can't generically get the payload, since the tnf and type determine how the payload is decoded
        int payloadLength = record.getPayloadLength();
        uint8_t payload[payloadLength];
        record.getPayload(payload);
        PrintHexChar(payload, payloadLength);

        // id is probably blank and will return ""
        String uid = record.getId();
        if (uid != "") {
          Serial.println(uid);
        }                
      }        
    }    
  }
  delay(3000);
}

