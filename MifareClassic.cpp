#include "MifareClassic.h"
#include "Ndef.h"
#include <Adafruit_NFCShield_I2C.h>

#define BLOCK_SIZE 16

NdefMessage& readMifareClassic(Adafruit_NFCShield_I2C& nfc, uint8_t * uid, int uidLength)
{
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
    int currentBlock = 4;
    int messageLength = 0;
    byte data[16];

    // read first block to get message length
    int success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);
    if (success)
    {
      success = nfc.mifareclassic_ReadDataBlock(currentBlock, data);
      if (success)
      {
        messageLength = getNdefLength(data);        
      }
      else
      {
        Serial.print("Error. Failed read block ");Serial.println(currentBlock); 
      }
    }
    else
    {
      Serial.print("Error. Block Authentication failed for ");Serial.println(currentBlock);
    }

    // this should be nested in the message length loop
    int index = 0;
    int buffer_size = getBufferSize(messageLength);
    uint8_t buffer[buffer_size];

    Serial.print("Message Length ");Serial.println(messageLength);
    Serial.print("Buffer Size ");Serial.println(buffer_size);

    while (index < buffer_size)
    {
      // authenticate on every sector
      if (nfc.mifareclassic_IsFirstBlock(currentBlock))
      {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);        
        if (!success)
        {
          Serial.print("Error. Block Authentication failed for ");Serial.println(currentBlock);
        }
      }

      // read the data
      success = nfc.mifareclassic_ReadDataBlock(currentBlock, &buffer[index]);
      if (success) 
      {
        Serial.print("Read block ");Serial.println(currentBlock);
        // Serial.print("Block ");Serial.print(currentBlock);Serial.print(" ");
        // nfc.PrintHexChar(&buffer[index], BLOCK_SIZE);
      } 
      else 
      {
        Serial.print("Read failed ");Serial.println(currentBlock);
      }

      index += BLOCK_SIZE;                   
      currentBlock++;

      // skip the trailer block
      if (nfc.mifareclassic_IsTrailerBlock(currentBlock))
      {
        Serial.print("Skipping block ");Serial.println(currentBlock);
        currentBlock++;    
      }

    }

    // print out for debugging (this would be easier block by block)
    Serial.println("\nRaw NDEF data");
    uint8_t* buffer_ptr = &buffer[0];    
    int j = 0;
    for (j = 0; j < (buffer_size / BLOCK_SIZE); j++)    
    {
        nfc.PrintHexChar(buffer_ptr, BLOCK_SIZE);
        buffer_ptr += BLOCK_SIZE;
    }
    Serial.println();

    nfc.PrintHex(&buffer[4], messageLength);

    NdefMessage* ndefMessage = new NdefMessage(&buffer[4], messageLength);
    return *ndefMessage;
}

int getBufferSize(int messageLength)
{ 

  // TLV header is 4 bytes. TLV terminator is 1 byte.
  int buffer_size = messageLength + 5;

  // buffer_size needs to be a multiple of BLOCK_SIZE
  if (buffer_size % BLOCK_SIZE != 0)
  {
      buffer_size = ((messageLength / BLOCK_SIZE) + 1) * BLOCK_SIZE;    
  }

  return buffer_size;
}

// get the ndef data length from the mifare TLV
// assuming the type and the length are in the first 4 bytes
// { 0x0, 0x0, 0x3, LENGTH }
// { 0x3, 0xFF, LENGTH, LENGTH }
int getNdefLength(byte *data)
{
    int ndefLength;
    if (data[0] == 0x03 && data[1] == 0xFF) {
        // would rather find spec, but using 
        // http://www.developer.nokia.com/Community/Discussion/showthread.php?131601-Problem-decoding-NDEF-message-on-Mifare-Card&p=407235&viewfull=1#post407235
        ndefLength = ((0xFF & data[2]) << 8) | (0xFF & data[3]);
    } else if (data[2] == 0x03) { // 1 byte
        ndefLength = data[3];
    } else {
        Serial.println("ERROR: Do not know how to decode length.");
        ndefLength = -1;
    }
    
    return ndefLength;
}

// TODO return success / failure
void writeMifareClassic(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength)
{

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
      if (write_success) 
      {
        Serial.print("wrote block ");Serial.println(currentBlock);
      } 
      else 
      {
        Serial.print("write failed ");Serial.println(currentBlock);
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