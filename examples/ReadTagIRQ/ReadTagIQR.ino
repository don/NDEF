#include <Wire.h>
#include "PN532_I2C.h"
#include "PN532.h"
#include "NfcAdapter.h"
#include "PN532Interface.h"

#define CARD_DELAY    1000
#define IRQ_PIN       14

// IRQ only works in I2C mode for the PN532
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c, IRQ_PIN);

long lastRead = 0;
bool enabled = true;
int irqCurr;
int irqPrev;

void irqListen() {
  irqPrev = irqCurr = HIGH;
  nfc.startPassive();
  Serial.println("\nScan a NFC tag\n");
}

void readCard() {
  if (nfc.tagPresent()) {
    NfcTag tag = nfc.read();
    tag.print();
    lastRead = millis();
  }
  enabled = false;
}

void setup () {
  Serial.begin(9600);
  nfc.begin();
  irqListen();
}

void loop () {
  if (!enabled) {
    if (millis() - lastRead > CARD_DELAY) {
      enabled = true;
      irqListen();
    }
  } else {
    irqCurr = digitalRead(IRQ_PIN);

    if (irqCurr == LOW && irqPrev == HIGH) {
      readCard();
    }
  
    irqPrev = irqCurr;
  }
}