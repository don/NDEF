#define NDEF_USE_SERIAL

#include "ntag.h"
#include "Arduino.h"
#define HARDI2C
#include <Wire.h>

#include <PN532.h>

#include <Ndef.h>
#include <NdefMessage.h>

#include <NdefRecord.h>
#include <Bounce2.h>

#include <Ndef.h>

#define FD_PIN    (4)
#define VOUT_PIN  (2)
#define LED_PIN (13)

// Troubleshooting
ISR(BADISR_vect)
{
    for (;;) UDR0='!';
}

Ntag ntag(Ntag::NTAG_I2C_2K, FD_PIN, VOUT_PIN);


void setup(){
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("start");
    
    // Create an NDEF record with a dummy payload and examine header
    NdefRecord rec = NdefRecord(3);
    rec.print();

    unsigned int header_len = rec.getHeaderSize();
    byte header[header_len];
    rec.getHeader(header, true, true);
    Serial.println(F("\nRecord header: "));
    showBlockInHex(header, header_len);

    Serial.print(F("\nSize of record in memory: "));
    Serial.println(sizeof(rec));

    Serial.print(F("\nPayload size: "));
    Serial.println(rec.getPayloadLength());
    Serial.print(F("\nEncoded size of record: "));
    Serial.println(rec.getEncodedSize());

    
    NdefMessage message = NdefMessage();
    if (message.addRecord(rec)) { Serial.println(F("Added record.")); }
    unsigned int msgHeader_len = message.getHeaderSize();
    byte msgHeader[msgHeader_len];
    message.getHeader(msgHeader);
    Serial.println(F("\nMessage header: "));
    showBlockInHex(msgHeader, msgHeader_len);

    Serial.print(F("Record count: "));
    Serial.println(message.getRecordCount());
    
    Serial.print(F("\nSize of message in memory: "));
    Serial.println(sizeof(message));

    Serial.print(F("Message encoded size (excluding NDEF TLV): "));
    Serial.println(message.getEncodedSize());

    delay(500);
    // Now we'll actually write to an NTAG
    Serial.print("Zeroing memory...");
    if (ntag.zeroEeprom()) { Serial.println("success."); }
    Serial.println("Configuring NTAG...");
    if (ntag.setContainerClass()) { Serial.println("success."); }          // Configure NTAG for an NDEF message to occupy all user EEPROM
    Serial.println("Writing NDEF message...");
    if (ntag.writeNdef(16, message, true))  { Serial.println("success."); } // Write the message to EEPROM (which starts at global address 16)
    
    byte block[16];
    ntag.readConfigBlock(block);
    Serial.println(F("\nConfig block: "));
    showBlockInHex(block, 16);
    Serial.println(F("\nUser EEPROM first block: "));
    ntag.readEeprom(16, block, 16);
    showBlockInHex(block, 16);
}

void loop() {
  // put your main code here, to run repeatedly:

}


void showBlockInHex(byte* data, byte size){
    for(int i=0;i<size;i++){
        Serial.print(data[i],HEX);
        Serial.print(" ");
        _delay_ms(10);
    }
    Serial.println();
}