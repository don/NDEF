#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <NfcAdapter.h>

NfcAdapter nfc = NfcAdapter();

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
        if (tag.hasNdefMessage()) // every tag won't have a message        
        {
            NdefMessage message = tag.getNdefMessage();
            // message.print();  // message has a print method (this is more like debug)
            
            // cycle through the records, printing some info from each
            int recordCount = message.getRecordCount();
            for (int i = 0; i < recordCount; i++) 
            {
                //NdefRecord record = message.getRecord(i);  
                 NdefRecord record = message[i]; // alternate syntax
                Serial.print("TNF: ");
                Serial.println(record.getTnf()); 
                Serial.print("Type: ");
                Serial.println(record.getType()); // will be "" for TNF_EMPTY
                
                // we can't generically get the payload, since the tnf and type determine how the payload is decoded
                int payloadLength = record.getPayloadLength();
                Serial.print("Payload Length: ");
                Serial.println(payloadLength);
                byte payload[payloadLength];
                record.getPayload(payload);
             
               // this block would make a nice helper function, to return the payload as a String:
                String result = "";
                for (int c=0; c< payloadLength; c++) {
                 result += (char)payload[c]; 
                }
             
                Serial.print("Payload: ");
                Serial.println(result);
               
                // id is probably blank and will return ""
                Serial.print("ID: ");
                Serial.println(record.getId());                
            }        
        }    
    }
    delay(3000);
}
