#define HARDI2C
#include <Wire.h>
#include <Ntag.h>
#include <PN532.h>
#include <NdefMessage.h>
#include <NdefRecord.h>
#include <ArduinoUnit.h>
#include <Bounce2.h>



// In addition to I2C, these must be wired to Arduino for
// the features that use them to work.
#define FD_PIN    (4)
#define VOUT_PIN  (2)
#define VCC_PIN   (3)   // Assumes NTAG Vcc supplied using GPIO

// Single global ntag instance
Ntag ntag(Ntag::NTAG_I2C_2K, FD_PIN, VOUT_PIN);


// Custom Assertion
void assertNoLeak(void (*callback)())
{
  int start = freeMemory();
  (*callback)();
  int end = freeMemory();
  assertEqual(0, (start - end));
}

void assertBytesEqual(const uint8_t* expected, const uint8_t* actual, int size) {
  for (int i = 0; i < size; i++) {
    // Serial.print("> ");Serial.print(expected[i]);Serial.print(" ");Serial.println(actual[i]);
    if (expected[i] != actual[i]) {
      Serial.print("\nassertBytesEqual() failing at index ");
      Serial.println(i);
    }
    assertEqual(expected[i], actual[i]);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(VCC_PIN, OUTPUT);
  digitalWrite(VCC_PIN, HIGH);
  if(!ntag.begin()){
    Serial.println("Can't find ntag. Expect tests to fail.");
  }
}

////////////// Begin Tests //////////////////////////////////////////////////////////////////////////

test(basic_config) {

  byte config[ntag.NTAG_BLOCK_SIZE];
  assertTrue(ntag.readConfigBlock(config));

  // Memory address 0 (first byte of block 0) always reads as manufacturer ID
  assertEqual(0x04 /*ntag.NXP_MFR_ID*/, config[0]);

  ntag.setContainerClass();
  assertTrue(ntag.readConfigBlock(config));

  // Confirm we've successfully written an intelligent default Container Class data set
  byte ccCheck[4] = NTAG_CC_NDEF_FULL;
  assertBytesEqual(&config[12], ccCheck, 4);
}


// From test created by LieBtrau
test(eeprom_read_write) {
    byte eepromdata[2*16];
    byte readeeprom[16];

    for(byte i=0;i<2*16;i++){
        eepromdata[i]=0x80 | i;
    }

    Serial.println("Writing block 1");
    assertTrue(ntag.writeEeprom(0,eepromdata,16));
  
    Serial.println("Writing block 2");
    assertTrue(ntag.writeEeprom(16,eepromdata+16,16));
    
    Serial.println("\nReading memory block 1");
    assertTrue(ntag.readEeprom(0,readeeprom,16));
    PrintHex(readeeprom,16);
    assertBytesEqual(eepromdata, readeeprom, 16);
    
    Serial.println("Reading memory block 2");
    assertTrue(ntag.readEeprom(16,readeeprom,16));
    PrintHex(readeeprom,16);
    assertBytesEqual(&eepromdata[16], readeeprom, 16);
    
    Serial.println("Reading bytes 10 to 20: partly block 1, partly block 2");
    assertTrue(ntag.readEeprom(10,readeeprom,10));
    PrintHex(readeeprom,10);
    assertBytesEqual(&eepromdata[10], readeeprom, 10);
    

    Serial.println("Writing byte 15 to 20: partly block 1, partly block 2");
    for(byte i=0;i<6;i++){
        eepromdata[i]=0x70 | i;
    }
    assertTrue(ntag.writeEeprom(15,eepromdata,6));

    Serial.println("\nReading memory block 1");
    assertTrue(ntag.readEeprom(0,readeeprom,16));
    PrintHex(readeeprom,16);

    
    Serial.println("Reading memory block 2");
    assertTrue(ntag.readEeprom(16,readeeprom,16));
    PrintHex(readeeprom,16);
    assertBytesEqual(&eepromdata[1], readeeprom, 5);
}


test(write_big_ndef) {
  NdefRecord r;
  uint16_t len = 512;
  byte payload[len];
  for (uint16_t i = 0x0; i < len; payload[i++] = i);

  r.setPayload(payload, len);

  assertEqual(len, r.getPayloadLength());

  NdefMessage m;
  m.addRecord(r);
  assertEqual(r.getEncodedSize(), m.getEncodedSize());
  assertEqual(r.getEncodedSize() + 5, m.getPackagedSize());

  ntag.zeroEeprom();
  uint16_t p_len = m.getPackagedSize();
  Serial.print("Packaged length in bytes: ");
  Serial.println(p_len);

  byte data[p_len];
  m.getPackaged(data);
  //ntag.writeEeprom(0, data, m.getPackagedSize());
  Serial.println("End of package: ");
  PrintHex(&data[p_len-8], 16);
  assertEqual(0xFE, data[p_len-1]);
  assertEqual(0x03, data[0]);

  ntag.writeNdef(16, m, true);
  Serial.println("Completed message write.");
  byte readback[16];
  ntag.readEeprom(m.getPackagedSize()-1, readback, 16);
  Serial.print("Confirming termination byte at page 0x"); Serial.println((m.getPackagedSize()-1)/4 + 4, HEX); Serial.flush();
  PrintHex(readback, 16);
  assertEqual(0xFE, readback[0]);                       // Confirm termination byte in right place

  ntag.readEeprom(0, readback, 16);
  assertEqual(0x03, readback[0]);                       // Confirm TLV start byte
}


void loop() {
  Test::run();
}