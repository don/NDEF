#include "MifareClassic.h"

#define BLOCK_SIZE 16

MifareClassic::MifareClassic()
{
}

MifareClassic::~MifareClassic()
{
}

NfcTag MifareClassic::read(Adafruit_NFCShield_I2C& nfc, uint8_t * uid, int uidLength)
{
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
    int currentBlock = 4;
    int messageLength = 0;
    byte data[BLOCK_SIZE];

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
    int bufferSize = getBufferSize(messageLength);
    uint8_t buffer[bufferSize];

    Serial.print("Message Length ");Serial.println(messageLength);
    Serial.print("Buffer Size ");Serial.println(bufferSize);

    while (index < bufferSize)
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
        Serial.print("Block ");Serial.print(currentBlock);Serial.print(" ");
        nfc.PrintHexChar(&buffer[index], BLOCK_SIZE);
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

    // NdefMessage ndefMessage = NdefMessage(&buffer[4], messageLength);
    // NfcTag tag = NfcTag(uid, uidLength, "Mifare Classic", ndefMessage);

    NdefMessage ndefMessage = NdefMessage(&buffer[4], messageLength);
    NfcTag tag = NfcTag(uid, uidLength, "Mifare Classic", ndefMessage);
    
    // WORKS!! NfcTag tag = NfcTag(uid, uidLength, "Mifare Classic", &buffer[4], messageLength);
    return tag;
}

int MifareClassic::getBufferSize(int messageLength)
{ 

  // TLV header is 4 bytes. TLV terminator is 1 byte.
  int bufferSize = messageLength + 5;

  // bufferSize needs to be a multiple of BLOCK_SIZE
  if (bufferSize % BLOCK_SIZE != 0)
  {
    bufferSize = ((bufferSize / BLOCK_SIZE) + 1) * BLOCK_SIZE;    
  }

  return bufferSize;
}

// get the ndef data length from the mifare TLV
// assuming the type and the length are in the first 4 bytes
// { 0x0, 0x0, 0x3, LENGTH }
// { 0x3, 0xFF, LENGTH, LENGTH }
int MifareClassic::getNdefLength(byte *data)
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

boolean MifareClassic::write(Adafruit_NFCShield_I2C& nfc, NdefMessage& m, uint8_t * uid, int uidLength)
{

    uint8_t encoded[m.getEncodedSize()];
    m.encode(encoded);
        
    uint8_t buffer[getBufferSize(sizeof(encoded))];
    memset(buffer, 0, sizeof(buffer));

    Serial.print("sizeof(encoded) ");Serial.println(sizeof(encoded));
    Serial.print("sizeof(buffer) ");Serial.println(sizeof(buffer));

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
          return false;
        }
      }

      int write_success = nfc.mifareclassic_WriteDataBlock (currentBlock, &buffer[index]);
      if (write_success) 
      {
        Serial.print(F("Wrote block "));Serial.print(currentBlock);Serial.print(" - ");
        nfc.PrintHexChar(&buffer[index], BLOCK_SIZE);
      } 
      else 
      {
        Serial.print(F("Write failed "));Serial.println(currentBlock);
        return false;
      }
      index += BLOCK_SIZE;                   
      currentBlock++;

      if (nfc.mifareclassic_IsTrailerBlock(currentBlock))
      {
        // can't write to trailer block
        Serial.print(F("Skipping block "));Serial.println(currentBlock);
        currentBlock++;    
      }

    }

    return true;
  }