#include <MifareUltralight.h>
#include <Ndef.h>
#include <Adafruit_NFCShield_I2C.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4 // we should be able to read 16 bytes at a time

#define ULTRALIGHT_DATA_START_PAGE 4
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 1
#define ULTRALIGHT_DATA_START_INDEX 2
#define ULTRALIGHT_MAX_PAGE 63

// MIFARE ULTRALIGHT C - NFC FORUM TYPE 2

MifareUltralight::MifareUltralight(Adafruit_NFCShield_I2C& nfcShield)
{
  nfc = &nfcShield;
  ndefStartIndex = 0;
  messageLength = 0;  
}

MifareUltralight::~MifareUltralight()
{
  //delete nfc;
}

NdefMessage MifareUltralight::read()
{  
  if (isUnformatted())
  {
    // TODO message needs to return a tag
    Serial.println("WARNING: Tag is not formatted.");
  }

  readCapabilityContainer(); // meta info for tag
  findNdefMessage();
  calculateBufferSize();
  
  if (messageLength == 0) { // data is 0x44 0x03 0x00 0xFE
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return message;
  }

  boolean success;
  uint8_t page;
  uint8_t index = 0;
  byte buffer[bufferSize];
  for (page = ULTRALIGHT_DATA_START_PAGE; page < ULTRALIGHT_MAX_PAGE; page++)     
  {
    // read the data
    success = nfc->mifareultralight_ReadPage(page, &buffer[index]);
    if (success) 
    {
      Serial.print("Page ");Serial.print(page);Serial.print(" ");
      nfc->PrintHexChar(&buffer[index], ULTRALIGHT_PAGE_SIZE);
    } 
    else 
    {
      Serial.print("Read failed ");Serial.println(page);
      // TODO error handling
      messageLength = 0;
      break;
    }
    
    if (index > messageLength)
    {
      break;
    }

    index += ULTRALIGHT_PAGE_SIZE;               

  }

    //nfc->PrintHexChar(buffer, sizeof(buffer));

  NdefMessage ndefMessage = NdefMessage(&buffer[ndefStartIndex], messageLength);
  return ndefMessage;
}

boolean MifareUltralight::isUnformatted() 
{
  uint8_t page = 4;
  byte data[ULTRALIGHT_READ_SIZE]; 
  boolean success = nfc->mifareultralight_ReadPage (page, data);
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

// page 3 has tag capabilities
void MifareUltralight::readCapabilityContainer()
{
  byte data[ULTRALIGHT_PAGE_SIZE];
  int success = nfc->mifareultralight_ReadPage (3, data);
  if (success)
  {
    // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
    tagCapacity = data[2] * 8;
    Serial.print("Tag capacity ");Serial.print(tagCapacity);Serial.println(" bytes");

    // TODO future versions should get lock information
  }
}

// read enough of the message to find the ndef message length
void MifareUltralight::findNdefMessage()
{
  int page;
  byte data[12]; // 3 pages
  byte* data_ptr = &data[0];

  // the nxp read command reads 4 pages, unfortunately adafruit give me one page at a time
  boolean success = true;
  for (page = 4; page < 6; page++)
  {
    success = success && nfc->mifareultralight_ReadPage(page, data_ptr);
    Serial.print("Page ");Serial.print(page);Serial.print(" - ");
    nfc->PrintHexChar(data_ptr, 4);
    data_ptr += ULTRALIGHT_PAGE_SIZE;
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

  Serial.print("messageLength ");Serial.println(messageLength);
  Serial.print("ndefStartIndex ");Serial.println(ndefStartIndex);

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
