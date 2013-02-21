#include "MifareClassic.h"
#include "Ndef.h"
#include <Adafruit_NFCShield_I2C.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4

// MIFARE ULTRALIGHT C - NFC FORUM TYPE 2

// TODO verify these assumptions!
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

NdefMessage readMifareUltralight(Adafruit_NFCShield_I2C& nfc)
{
    int currentPage = 5; // data starts on page 5
    int messageLength = 0;
    byte pageData[ULTRALIGHT_READ_SIZE]; // TODO RTFM, I think we get many pages, can I read less often?
 
    int success = nfc.mifareultralight_ReadPage (currentPage, pageData);
    if (success)
    {
      messageLength = pageData[2];      
    }
    else
    {
      Serial.print("Error. Failed read page ");Serial.println(currentPage); 
    }

    // this should be nested in the message length loop
    int index = 0;
    int bufferSize = getUltralightBufferSize(messageLength);
    uint8_t buffer[bufferSize];

    Serial.print("Message Length ");Serial.println(messageLength);
    Serial.print("Buffer Size ");Serial.println(bufferSize);

    // TODO special case 
    // Message Length == 0 should return an TNF_EMPTY data is 0x44 0x03 0x00 0xFE

    // for (page = 6; page < 63; page++) {      
    while (index < messageLength)
    {

      // read the data
      success = nfc.mifareultralight_ReadPage(currentPage, &buffer[index]);
      if (success) 
      {
        Serial.print("Read page ");Serial.println(currentPage);
      } 
      else 
      {
        Serial.print("Read failed");Serial.println(currentPage);
        // TODO error handling
      }

      index += ULTRALIGHT_PAGE_SIZE; // TODO ULTRALIGHT_READ_SIZE                  
      currentPage++;  // TODO jump multiple pages!!!

      // TODO don't read past block 63

    }

    nfc.PrintHex(&buffer[3], messageLength); // TODO MIFARE_DATA_START_INDEX 3

    NdefMessage ndefMessage = NdefMessage(&buffer[3], messageLength);
    return ndefMessage;
}

