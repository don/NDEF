#include "MifareClassic.h"
#include "Ndef.h"
#include <Adafruit_NFCShield_I2C.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4

#define ULTRALIGHT_DATA_START_PAGE 5
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 2
#define ULTRALIGHT_DATA_START_INDEX 3
#define ULTRALIGHT_MAX_PAGE 63

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
    int page = ULTRALIGHT_DATA_START_PAGE;
    int messageLength = 0;
    byte pageData[ULTRALIGHT_READ_SIZE]; 
 
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

