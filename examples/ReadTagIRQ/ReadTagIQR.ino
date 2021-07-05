/* -------------------------------------------------------------- */
/*  This example uses the IRQ port on the PN532 which is only     */
/*  available when the PN532 is in IRQ mode. It will not work in  */
/*  other modes. You will need to connect the IRQ pin to an IO    */
/* -------------------------------------------------------------- */

#include <Wire.h>
#include "PN532_I2C.h"
#include "PN532.h"
#include "NfcAdapter.h"

//how long to wait between card reads
#define CARD_DELAY    1000  // wait 1s before reading another card
#define IRQ_PIN       14    // pin the IRQ on the PN532 is connected to

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c, IRQ_PIN);

long lastRead = 0;
bool enabled = true;
int irqCurr;
int irqPrev;

void irqListen() {
  irqPrev = irqCurr = HIGH;
  nfc.startPassive();
  Serial.println("Scan a NFC tag");
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
