#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <NfcAdapter.h>

NfcAdapter nfc = NfcAdapter();

void setup(void) {
  Serial.begin(9600);
  Serial.println("NDEF Reader");
  nfc.begin();
}

void loop(void) {
  Serial.println("Scan a NFC tag");
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    if (tag.hasNdefMessage()) { // every tag won't have a message        
      NdefMessage message = tag.getNdefMessage();
      // cycle through the records, printing some info from each
      int recordCount = message.getRecordCount();
      for (int m = 0; m < recordCount; m++) 
      {
        NdefRecord record = message.getRecord(m);  
        //NdefRecord record = message[m]; // alternate syntax
        Serial.print("TNF: ");
        Serial.println(record.getTnf()); 
        Serial.print("Type: ");
        Serial.println(record.getType()); // will be "" for TNF_EMPTY

        // you can't generically get the payload, since the tnf 
        // and type determine how the payload is decoded:
        int payloadLength = record.getPayloadLength();
        Serial.print("Payload Length: ");
        Serial.println(payloadLength);
        // once you know the length, make an array for it:
        byte payload[payloadLength];   
        record.getPayload(payload);

        // Convert the byte array to a String:
        String result = "";
        for (int c=0; c< payloadLength; c++) {
          result += (char)payload[c]; 
        }
        // print the payload:
        Serial.print("Payload: ");
        Serial.println(result);

        // id is probably blank and will return ""
        Serial.print("ID: ");
        Serial.println(record.getId());                
      }        
    }    
  }
  delay(3000);    // delay before next read
}


