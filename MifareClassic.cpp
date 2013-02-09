#include "MifareClassic.h"
#include "Ndef.h"
#include <Adafruit_NFCShield_I2C.h>

// TODO return success / failure
void write(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength)
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