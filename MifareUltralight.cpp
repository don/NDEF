#include "MifareClassic.h"
#include "Ndef.h"
#include <Adafruit_NFCShield_I2C.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4 // we should be able to read 16 bytes at a time

#define ULTRALIGHT_DATA_START_PAGE 4
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 1
#define ULTRALIGHT_DATA_START_INDEX 2
#define ULTRALIGHT_MAX_PAGE 63

// MIFARE ULTRALIGHT C - NFC FORUM TYPE 2

// TODO verify these assumptions!
// See AN1303 2.5 Card Identification Procedure
int getUltralightBufferSize(int messageLength)
{ 

  // TLV header is 3 bytes. TLV terminator is 1 byte.
  int bufferSize = messageLength + 4;

  // TODO ULTRALIGHT_PAGE_SIZE or ULTRALIGHT_READ_SIZE
  // bufferSize needs to be a multiple of ULTRALIGHT_PAGE_SIZE
  if (bufferSize % ULTRALIGHT_READ_SIZE != 0)
  {
      bufferSize = ((messageLength / ULTRALIGHT_READ_SIZE) + 1) * ULTRALIGHT_READ_SIZE;    
  }

  return bufferSize;
}


// we have the ide
// need to read in pages 2 through 6
// read reserved / locked  - to get read/write state
// read OTP - to get the size and locked and maybe other meta info
// read data - find the TLV for NDEF
// can I just skip the Type 01? See chapter 8
// TODO this would be simpler if Adafruit read 4 pages at a time

// buffer should alway re-read from page 4
// need to track the offset for the ndefMessageStart (in the buffer)
// trim a the end (like now)

// HANDLE MULTIPLE SECTORS IN FUTURE. PAGE limited to  < 64


// TODO read the first X pages to get the header info
// Probably pages 2 though 16

// getMaxPages()
// some have 16 pages, some have 63 pages

boolean isLockable() {

}

boolean isLocked() {
  // check OTP bytes 2 and 3?
}

// unformatted tags have 0xFF, 0xFF, 0xFF, 0xFF on page 4
// really should check OTP area too
boolean isUnformatted() {
  uint8_t page = 4;
  byte data[ULTRALIGHT_READ_SIZE]; 
  int success = nfc.mifareultralight_ReadPage (page, data);
  if (success)
  {
    return (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF);
  }
  else
  {
    Serial.print("Error. Failed read page ");Serial.println(page); 
    return false;
  }
}

void readOTP() // Capability Container or One Time Programmable
{
  uint8_t capacity;

  byte otp[4];
  int success = nfc.mifareultralight_ReadPage (3, data);
  if (success)
  {
    // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
    capacity = otp[2] * 8;
    // ignoring locked stuff for now
  }
}

// read first data pages
// need to know where the NDEF message starts and how long it is
void readHeader()
{
  byte header[16]; // 4 pages
  uint8_t messageLength; // TODO how can this be more than 0xFF
  uint8_t ndefStartIndex;

  // the read command reads 4 pages, unfortunately adafruit give me one pages at a time
  boolean success = true;
  for (page = 4; page < 6)
  {
    success = success && nfc.mifareultralight_ReadPage (page, data);
  }

  if (success)
  {
    if (data[0] == 0x03) 
    {
      messageLength = data[1];
      ndefStartIndex = data[2];
    }
    else if (data[5] == 0x3) // page 5 byte 1
    {
      // TODO should really have read the lock control tlv to ensure byte[5] is the right plcae
      messageLength = data[6];
      ndefStartIndex = data[7];
    } 
  }
}

void getMessageLength() // TODO need to make a class!
{
    uint8_t page = 4;
    byte data[ULTRALIGHT_READ_SIZE]; 
    
    // these need to be member variables
    uint8_t messageLength = 0;
    uint8_t ndefStartIndex = 0; // alway keep all the pages so page doesn't matter for index?
    uint8_t ndefStartPage = 0;
    
    // read page 4
    // data begins at page 4 index 0
    // we need to find ndef data, if page 4, index 0 is "03" it is the NDEF TLV start
    // if it is "01" another format, can skip by reading length
    int success = nfc.mifareultralight_ReadPage (page, data);
    if (success)
    {
      if (data[0] == 0x03) {
        messageLength = data[1];
        ndefStartIndex = 2;
        page = 4;
      }
    }
    else
    {
      Serial.print("Error. Failed read page ");Serial.println(page); 
      // TODO handle error, currently will return TNF_EMPTY below
    }

    if (messageLength == 0)
    {
      page++; // See AN1303 Chapter 8 for example
      success = nfc.mifareultralight_ReadPage (page, data);
      if (success)
      {
        if (data[2] == 0x03) {
          messageLength = data[2];
          ndefStartIndex = 3;
          page = 5;
        }
      }
      else
      {
        Serial.print("Error. Failed read page ");Serial.println(page); 
        // TODO handle error, currently will return TNF_EMPTY below
      }
    }

}

MifareUltralight::MifareUltralight(Adafruit_NFCShield_I2C& nfcShield)
{
  nfc = nfcShield;
}

MifareUltralight::~MifareUltralight()
{
  delete nfc;
}

// page 3 has tag capabilities
void MifareUltralight::readCapabilityContainer()
{
  byte data[ULTRALIGHT_PAGE_SIZE];
  int success = nfc.mifareultralight_ReadPage (3, data);
  if (success)
  {
    // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
    tagCapacity = otp[2] * 8;
    Serial.print("Tag capacity ");Serial.println(tagCapacity);Serial.println(" bytes");

    // TODO future versions should get lock information
  }
}

// read enough of the message to find the ndef message length
void MifareUltralight::findNdefMessageSize()
{
  byte header[12]; // 3 pages

  // the nxp read command reads 4 pages, unfortunately adafruit give me one page at a time
  boolean success = true;
  for (page = 4; page < 6)
  {
    success = success && nfc.mifareultralight_ReadPage (page, data);
  }

  if (success)
  {
    if (data[0] == 0x03) 
    {
      messageLength = data[1];
      ndefStartIndex = 2;
    }
    else if (data[5] == 0x3) // page 5 byte 1
    {
      // TODO should really read the lock control TLV to ensure byte[5] is correct
      messageLength = data[6];
      ndefStartIndex = 7;
    } 
  }
}

// buffer is larger than the message, need to handle some data before and after
// message and need to ensure we read full pages
void MifareUltralight::calculateBufferSize()
{
  // TLV terminator 0xFE is 1 byte
  bufferSize = messageLength + ndefStartIndex + 1;

  if (bufferSize % ULTRALIGHT_READ_SIZE != 0)
  {
      bufferSize = ((messageLength / ULTRALIGHT_READ_SIZE) + 1) * ULTRALIGHT_READ_SIZE;    
  }
}


NdefMessage MifareUltralight::read()
{
  
  if (isUnformatted())
  {
    // TODO message needs to return a tag
    return;
  }

  readCapabilityContainer(); // meta info for tag
  findNdefMessageSize();
  calculateBufferSize();

  if (messageLength == 0) { // data is 0x44 0x03 0x00 0xFE
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return message;
  }

  int page;
  byte buffer[bufferSize];

  for (page = ULTRALIGHT_DATA_START_PAGE; page < ULTRALIGHT_MAX_PAGE; page++)     
    {

      // read the data
      success = nfc.mifareultralight_ReadPage(page, &buffer[index]);
      if (success) 
      {
        Serial.print("Page ");Serial.print(page);Serial.print(" ");
        nfc.PrintHexChar(&buffer[index], ULTRALIGHT_PAGE_SIZE);
      } 
      else 
      {
        Serial.print("Read failed");Serial.println(page);
        // TODO error handling
        break;
      }
      
      if (index > messageLength)
      {
        break;
      }

      index += ULTRALIGHT_PAGE_SIZE;               

    }

    NdefMessage ndefMessage = NdefMessage(&buffer[ndefStartIndex], messageLength);
    return ndefMessage;

}

NdefMessage readMifareUltralight(Adafruit_NFCShield_I2C& nfc)
{
    int page = ULTRALIGHT_DATA_START_PAGE;
    int messageLength = 0;
    byte pageData[ULTRALIGHT_READ_SIZE]; 
 
    // read page 4


    int success = nfc.mifareultralight_ReadPage (page, pageData);
    if (success)
    {
      messageLength = pageData[ULTRALIGHT_MESSAGE_LENGTH_INDEX];      
    }
    else
    {
      Serial.print("Error. Failed read page ");Serial.println(page); 
      // TODO handle error, currently will return TNF_EMPTY below
    }

    // this should be nested in the message length loop
    int index = 0;
    int bufferSize = getUltralightBufferSize(messageLength);
    uint8_t buffer[bufferSize];

    Serial.print("Message Length ");Serial.println(messageLength);
    Serial.print("Buffer Size ");Serial.println(bufferSize);

    // TODO handle unformatted tags properly
    // 2.1.2 MIFARE Ultralight Blank Card Settings "bytes on page 4 are set to 0xFF"

    if (messageLength == 0) { // data is 0x44 0x03 0x00 0xFE
      NdefMessage message = NdefMessage();
      message.addEmptyRecord();
      return message;
    }

    for (page = ULTRALIGHT_DATA_START_PAGE; page < ULTRALIGHT_MAX_PAGE; page++)     
    {

      // read the data
      success = nfc.mifareultralight_ReadPage(page, &buffer[index]);
      if (success) 
      {
        Serial.print("Page ");Serial.print(page);Serial.print(" ");
        nfc.PrintHexChar(&buffer[index], ULTRALIGHT_PAGE_SIZE);
      } 
      else 
      {
        Serial.print("Read failed");Serial.println(page);
        // TODO error handling
        break;
      }
      
      if (index > messageLength)
      {
        break;
      }

      index += ULTRALIGHT_PAGE_SIZE;               

    }

    NdefMessage ndefMessage = NdefMessage(&buffer[ULTRALIGHT_DATA_START_INDEX], messageLength);
    return ndefMessage;
}



