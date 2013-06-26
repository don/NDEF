#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>
#include <NdefRecord.h>
#include <ArduinoUnit.h>

TestSuite suite;

void assertBytesEqual(Test& __test__, const uint8_t* expected, const uint8_t* actual, uint8_t size) {
  for (int i = 0; i < size; i++) {
    assertEquals(expected[i], actual[i]);
  }
}

void setup() {
    Serial.begin(9600);
}

test(foo) {
  assertEquals(3, (1 + 2));
}

test(accessors) {
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEquals(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));
  
  assertEquals(TNF_WELL_KNOWN, record.getTnf());
  assertEquals(sizeof(recordType), record.getTypeLength());
  assertEquals(1, record.getTypeLength());
  assertEquals(sizeof(payload), record.getPayloadLength());
  assertEquals(12, record.getPayloadLength());
  
  uint8_t typeCheck[record.getTypeLength()];
  record.getType(typeCheck);

  assertEquals(0x54, typeCheck[0]);
  assertBytesEqual(__test__, recordType, typeCheck, sizeof(recordType));

  uint8_t payloadCheck[record.getPayloadLength()];
  record.getPayload(&payloadCheck[0]);
  assertBytesEqual(__test__, payload, payloadCheck, sizeof(payload));  
}

test(newaccessors) {
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEquals(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));
  
  assertEquals(TNF_WELL_KNOWN, record.getTnf());
  assertEquals(sizeof(recordType), record.getTypeLength());
  assertEquals(1, record.getTypeLength());
  assertEquals(sizeof(payload), record.getPayloadLength());
  assertEquals(12, record.getPayloadLength());
    
  String typeCheck = record.getType();
  assertTrue(typeCheck.equals("T"));
  
  uint8_t* payloadCheck = record.getPayload();
  assertBytesEqual(__test__, payload, payloadCheck, sizeof(payload));  
  free(payloadCheck);
}

test(assignment)
{
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEquals(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));

  NdefRecord record2 = NdefRecord();
  record2 = record;

  assertEquals(TNF_WELL_KNOWN, record.getTnf());
  assertEquals(sizeof(recordType), record2.getTypeLength());
  assertEquals(sizeof(payload), record2.getPayloadLength());

  String typeCheck = record.getType();
  assertTrue(typeCheck.equals("T"));
  
  uint8_t* payload2 = record2.getPayload();  
  assertBytesEqual(__test__, payload, payload2, sizeof(payload));  
  free(payload2);  
}

test(getEmptyPayload)
{
  NdefRecord r = NdefRecord();
  assertEquals(TNF_EMPTY, r.getTnf());
  assertEquals(0, r.getPayloadLength());
  uint8_t* payload = r.getPayload();
  // assume we can free something malloc'd with 0 
  free(payload);
  // TODO need an assertion here
}

void loop() {
  suite.run();  
}
