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
    NdefRecord rec = NdefRecord(10);
    Serial.print("ID length: ");
    Serial.println(rec.getIdLength());
    rec.print();

    unsigned int header_len = rec.getHeaderSize();
    byte header[header_len];
    rec.getHeader(header, true, true);
    Serial.println("\n Record header: ");
    showBlockInHex(header, header_len);

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
