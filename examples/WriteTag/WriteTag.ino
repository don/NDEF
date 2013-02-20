#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <NfcAdapter.h>

NfcAdapter nfc = NfcAdapter();

void setup() {
  Serial.begin(115200);
  Serial.println("NDEF Writer");
  nfc.begin();
}

void loop() {
  Serial.println("\nPlace a formatted Mifare Classic NFC tag on the reader.");
  if (nfc.tagPresent()) {
    NdefMessage message = NdefMessage();
    message.addTextRecord("Hello, Arduino!");
    nfc.write(message);
    Serial.println("Try reading this tag with your phone.");  
  }
  delay(5000);
}