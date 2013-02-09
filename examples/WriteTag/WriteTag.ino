//
// Write "Hello, world!" in a plain text NDEF message
// Tag must be a Mifare Classic with a 4 byte ID
// Tested with Samsung TecTile
//
#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Ndef.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
	  
      NdefMessage* m = new NdefMessage();
      m->addTextRecord("Hello, World");
      m->addUriRecord("http://arduino.cc");
      m->addEmptyRecord();
      m->addTextRecord("Goodbye, World");
      
      m->print();

      write(*m, uid, uidLength);

      Serial.println("\nTry and read the tag with your phone.");
              
      delay(5000);      
    }
    else
    {
      Serial.println("This sketch only supports Mifare Classic tags with 4 byte UIDs");
      delay(5000);
    }
    
  }
}

// TODO this belongs in MifareClassic Library
// TODO return success / failure
void write(NdefMessage& m, uint8_t * uid, int uidLength)
{
    int BLOCK_SIZE = 16;

    uint8_t encoded[m.getEncodedSize()];
    m.encode(encoded);
        
    // TLV wrapper 
    // assuming short TLV  3 bytes type +  byte length of data + NDEF_DATA + 1 byte terminator
    int buffer_size = sizeof(encoded) + 5;
    Serial.print("buffer_size ");Serial.println(buffer_size);

    // buffer_size needs to be a multiple of BLOCK_SIZE
    if (buffer_size % BLOCK_SIZE != 0)
    {
      buffer_size = ((buffer_size / BLOCK_SIZE) + 1) * BLOCK_SIZE;
      Serial.print("Adjusted buffer_size to ");Serial.println(buffer_size);
    }

    uint8_t buffer[sizeof(encoded) + 5];
    buffer[0] = 0x0;        
    buffer[1] = 0x0;        
    buffer[2] = 0x3;        
    buffer[3] = sizeof(encoded);
    memcpy(&buffer[4], encoded, sizeof(encoded));
    buffer[4+sizeof(encoded)] = 0xFE; // terminator

    // Write to tag
    int index = 0;
    int currentBlock = 4;
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 }; // this is Sector 1 - 15 key

    while (index < sizeof(buffer))
    {  

      if (nfc.mifareclassic_IsFirstBlock(currentBlock))
      {
        int success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);        
        if (!success)
        {
          Serial.print("Error. Block Authentication failed for ");Serial.println(currentBlock);
          while (1); // halt
        }
      }

      int write_success = nfc.mifareclassic_WriteDataBlock (currentBlock, &buffer[index]);
      if (write_success) {
          Serial.print("wrote block ");Serial.println(currentBlock);
      } else {
          Serial.println("write failed");
          while (1); // halt
      }
      index += BLOCK_SIZE;                   
      currentBlock++;

      if (nfc.mifareclassic_IsTrailerBlock(currentBlock))
      {
        // can't write to trailer block
        Serial.print("skipping block ");Serial.println(currentBlock);
        currentBlock++;    
      }

    }
        
}

