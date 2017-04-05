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


void setup(){
    Serial.begin(115200);
    Serial.println("start");

    NdefMessage message = NdefMessage();

    // Create an NDEF record with a dummy payload and examine header
    NdefRecord rec = NdefRecord(80);
    Serial.print("ID length: ");
    Serial.println(rec.getIdLength());
    rec.print();

    unsigned int header_len = rec.getHeaderSize();
    byte header[header_len];
    rec.getHeader(header, true, true);
    Serial.println(F("\n Record header: "));
    showBlockInHex(header, header_len);

    Serial.print(F("\n Size of record in memory: "));
    Serial.println(sizeof(rec));

    if (message.addRecord(rec)) { Serial.println(F("Added record.")); }
    unsigned int msgHeader_len = message.getHeaderSize();
    byte msgHeader[msgHeader_len];
    message.getHeader(msgHeader);
    Serial.println("\n Message header: ");
    showBlockInHex(msgHeader, msgHeader_len);

    Serial.print("Record count: ");
    Serial.println(message.getRecordCount());
    
    Serial.print(F("\n Size of message in memory: "));
    Serial.println(sizeof(message));

    Serial.println(F("\n (Adding more copies of record to message)"));
    if (message.addRecord(rec)) { Serial.println(F("Added record.")); }
    if (message.addRecord(rec)) { Serial.println(F("Added record.")); }

    Serial.print("Record count: ");
    Serial.println(message.getRecordCount());
    msgHeader_len = message.getHeaderSize();
    byte msgHeader2[msgHeader_len];
    message.getHeader(msgHeader2);
    Serial.println("\n Message header: ");
    showBlockInHex(msgHeader2, msgHeader_len);

    Serial.print("\n Size of message in memory: ");
    Serial.println(sizeof(message));
}

void loop() {
  // put your main code here, to run repeatedly:

}

void showBlockInHex(byte* data, byte size){
    for(int i=0;i<size;i++){
        Serial.print(data[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
}
