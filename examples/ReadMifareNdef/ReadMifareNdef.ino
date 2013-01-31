//
// Read raw data from Mifare Classic or Ultralight tag and print out NDEF info
//
#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);

// Mifare Classic NDEF key
uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

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

// TODO really want to return an array of objects or structs
void decode(const byte * data, const uint32_t numBytes) {
    
  // data is a stream of ndef records
  
  // TODO array to hold new records
  
  Serial.println("NDEF Message\n");
  
  int index = 0;
  
  while (index <= numBytes) {
    
      // decode tnf - first byte is tnf with bit flags
      // see the NFDEF spec for more info
      byte tnf_byte = data[index];
      bool mb = (tnf_byte & 0x80) != 0;
      bool me = (tnf_byte & 0x40) != 0;
      bool cf = (tnf_byte & 0x20) != 0;
      bool sr = (tnf_byte & 0x10) != 0;
      bool il = (tnf_byte & 0x8) != 0;
      byte tnf = (tnf_byte & 0x7);

      index++;
      int typeLength = data[index];
      
      int payloadLength = 0;
      if (sr)
      {
        index++;
        payloadLength = data[index];
      }
      else
      {        
        payloadLength = ((0xFF & data[++index]) << 24) | ((0xFF & data[++index]) << 26) | ((0xFF & data[++index]) << 8) | (0xFF & data[++index]);
      }

      int idLength = 0;
      if (il)
      {
        index++;
        idLength = data[index];
      }

      index++; 
      // TODO can type be an Arduino array instead of byte[]? 
      byte type[typeLength];
      memcpy(type, &data[index], typeLength);
      index += typeLength;
      
      byte id[idLength];
      if (il)
      {
        memcpy(id, &data[index], idLength);
        index += idLength;
      }
      
      byte payload[payloadLength];
      memcpy(payload, &data[index], payloadLength);
      index += payloadLength;

      // TODO print out data until I can put into a class or struct
      Serial.print("tnf 0x");
      Serial.println(tnf, HEX);
      
      Serial.print("type (length=");
      Serial.print(typeLength);      
      Serial.println(")");
      nfc.PrintHexChar(type, typeLength);
      
      if (il)
      {
        Serial.print("id (length=");
        Serial.print(idLength);      
        Serial.println(")");
        nfc.PrintHexChar(id, idLength);
      }
      
      Serial.print("payload (length=");
      Serial.print(payloadLength);     
      Serial.println(")");
      nfc.PrintHexChar(payload, payloadLength);
      Serial.println("");
      
      if (me) break; // last message
  }    
  
}

// Classic
void printBlockForDebug(int currentblock, byte *data)
{
  Serial.print("Block ");Serial.print(currentblock, DEC);
  if (currentblock < 10)
  {
    Serial.print("  ");
  }
  else
  {
    Serial.print(" ");
  }
  // Dump the raw data
  nfc.PrintHexChar(data, 16);
}

// Ultralight
void printPageForDebug(int page, byte *data)
{
  Serial.print("Page ");Serial.print(page, DEC);
  if (page < 10)
  {
    Serial.print("  - ");
  }
  else
  {
    Serial.print(" - ");
  }
  // Dump the raw data
  nfc.PrintHexChar(data, 4);
}

// Mifare data starts in Sector 1 Block 4
// data is only in the first 3 blocks of each sector
// the 4th block is a trailing block for the signature
// this method calculates the number of blocks we need to read
// TODO check spec and handle larger block size Mifare Classic 4K tags
// TODO this is buggy, write some tests and compare with actual output
int calculateNumberBlocksToRead(int length)
{
    int headerBytes = 4;
    int blockSize = 16;
    int firstBlockSize = blockSize - headerBytes;
    
    // always read one block
    int blocks = 1;
    
    // read additional blocks
    int additionalBlocks = (length - firstBlockSize) / blockSize;
    blocks += additionalBlocks;
    
    // there may be partial bytes in the last block
    int remainder = length - firstBlockSize - (additionalBlocks * 16);
    
    if (remainder > 0)
    {
        blocks += 1;
    }
    
    int numberMifareSignatureBlocks = (blocks + 1) / 4;
    
    return blocks + numberMifareSignatureBlocks;
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
      // TODO this is a bad assumption, need to get tag type earlier
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
	  
      // Now we need to try to authenticate it for read/write access
      Serial.println("Trying to authenticate block 4 with default KEYA value");
      uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 }; //

      // Start with block 4 (the first block of sector 1) since sector 0
      // contains the manufacturer data
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, key);
	  
      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated\n");
        byte data[16];
        	
        // Try to read the contents of block 4
        success = nfc.mifareclassic_ReadDataBlock(4, data);
		
        if (success)
        {  
          // first block tells how much data to read
          printBlockForDebug(4, data);
          Serial.println();
          
          int length = getNdefLength(data);          
          Serial.print("NDEF data length is ");Serial.print(length);Serial.print(" 0x");Serial.println(length, HEX);
                    
          // array to hold the encoded NDEF data
          byte rawNdefData[length];
          byte *rawNdefData_ptr = &rawNdefData[0];
                    
          // read the first row          
          byte *data_ptr = &data[0];
          data_ptr += 4; // first 4 bytes are the Type and Length of TLV
          int length_first_block = 16 - 4;
          
          // copy just the data portion into our array
          memcpy(rawNdefData, data_ptr, length_first_block);
          rawNdefData_ptr += length_first_block;
    
          // ensure the copy was correct
          Serial.print("Data portion of first block ");nfc.PrintHexChar(rawNdefData, length_first_block); 
                    
          int numberBlocksToRead = calculateNumberBlocksToRead(length);
          Serial.print("\nNeed to read ");Serial.print(numberBlocksToRead - 1);Serial.print(" more block");
          numberBlocksToRead == 2 ? Serial.print("") : Serial.print("s");
          Serial.print(" of data from the tag.\n");
                         
          int currentblock;
          boolean authenticated;
          
          // Read addition data from block 5 forward. We already read block 4.
          for (currentblock = 5; currentblock < (5 + numberBlocksToRead); currentblock++)
          {
                // skip trailer blocks since it contains signatures, not data
                if (nfc.mifareclassic_IsTrailerBlock(currentblock))
                {
                   Serial.print("Skipping Trailer Block ");Serial.println(currentblock);
                   continue;
                }
              
                // Reset authentication for at the beginning of each sector
                if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;                
                
                if (!authenticated)
                {
                    // Starting of a new sector ... try to to authenticate
                    Serial.print("------------------------Sector ");Serial.print(currentblock/4, DEC);Serial.println("-------------------------");
                    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 0, key);
                    
                    if (success)
                    {
                        authenticated = true;
                    }
                    else
                    {
                        // TODO need to die if processing fails
                        Serial.println("Authentication error");
                    }
                }
                
                // TODO this needs to be an error for NDEF
                // If we're still not authenticated just skip the block 
                if (!authenticated)
                {
                  Serial.print("Block ");Serial.print(currentblock, DEC);Serial.println(" unable to authenticate");
                }
                else
                {
                  // Authenticated ... we should be able to read the block now
                  // Dump the data into the 'data' array
                  success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
                  if (success)
                  {
                    printBlockForDebug(currentblock, data);

                    // copy the data into rawNdefData
                    memcpy(rawNdefData_ptr, data, sizeof(data));  // TODO DON'T OVERWRITE THE END!
                    rawNdefData_ptr += sizeof(data);

                  }
                  else
                  {
                    // Oops ... something happened
                    Serial.print("Block ");Serial.print(currentblock, DEC);
                    Serial.println(" unable to read this block");
                  }
                }       
              }        


          Serial.println("\nNDEF data ");
          nfc.PrintHexChar(rawNdefData, length);
          Serial.println();
                              
          decode(rawNdefData, length);
                   		  
          // Wait a long time before reading the card again          
          delay(10000);
          
        }
        else
        {
          Serial.println("Ooops ... unable to read the requested block.  Try another key?");
        }
        
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
    }
    
    if (uidLength == 7)
    {
      // We probably have a Mifare Ultralight card ...
      Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");


      // Hack implementation
      int page = 5;
      int mifare_ultralight_bytes_per_page = 4;
      uint8_t page_data[32]; // TODO check library, pn532 command reads multiple pages
      success = nfc.mifareultralight_ReadPage (page, page_data);
      printPageForDebug(page, page_data);
      int length = page_data[2];
      Serial.print("NDEF message length is ");Serial.println(length);
      
      uint8_t ndef_data[length];
      ndef_data[0] = page_data[3];

      int index = 1; // ndef_data_index
      
      for (page = 6; page < 63; page++) {      
        nfc.mifareultralight_ReadPage (page, page_data);
        printPageForDebug(page, page_data);
        // assume success
        memcpy(&ndef_data[index], page_data, mifare_ultralight_bytes_per_page); // TODO don't overwrite array on the last page!        
        index += mifare_ultralight_bytes_per_page;
        if (index >= length) break;
      }
      
      nfc.PrintHexChar(ndef_data, length);
      
      decode(ndef_data, length);    

      delay(5000);
    }
  }
}

