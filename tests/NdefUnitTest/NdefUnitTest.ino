#include <Wire.h>
#include <PN532.h>
#include <NdefRecord.h>
#include <ArduinoUnit.h>

void assertBytesEqual(const uint8_t* expected, const uint8_t* actual, uint8_t size) {
  for (int i = 0; i < size; i++) {
    assertEqual(expected[i], actual[i]);
  }
}

void setup() {
    Serial.begin(9600);
}

test(accessors) {
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEqual(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));
  
  assertEqual(TNF_WELL_KNOWN, record.getTnf());
  assertEqual(sizeof(recordType), record.getTypeLength());
  assertEqual(1, record.getTypeLength());
  assertEqual(sizeof(payload), record.getPayloadLength());
  assertEqual(12, record.getPayloadLength());
  
  uint8_t typeCheck[record.getTypeLength()];
  record.getType(typeCheck);

  assertEqual(0x54, typeCheck[0]);
  assertBytesEqual(recordType, typeCheck, sizeof(recordType));

  uint8_t payloadCheck[record.getPayloadLength()];
  record.getPayload(&payloadCheck[0]);
  assertBytesEqual(payload, payloadCheck, sizeof(payload));  
}

test(newaccessors) {
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEqual(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));
  
  assertEqual(TNF_WELL_KNOWN, record.getTnf());
  assertEqual(sizeof(recordType), record.getTypeLength());
  assertEqual(1, record.getTypeLength());
  assertEqual(sizeof(payload), record.getPayloadLength());
  assertEqual(12, record.getPayloadLength());
    
  ::String typeCheck = record.getType();
  assertTrue(typeCheck.equals("T"));
  
  byte payloadCheck[record.getPayloadLength()];
  record.getPayload(payloadCheck);
  assertBytesEqual(payload, payloadCheck, sizeof(payload));  
}

test(assignment)
{
  NdefRecord record = NdefRecord();
  record.setTnf(TNF_WELL_KNOWN);
  uint8_t recordType[] = { 0x54 }; // "T" Text Record
  assertEqual(0x54, recordType[0]);
  record.setType(recordType, sizeof(recordType));
  // 2 + "en" + "Unit Test"
  uint8_t payload[] = { 0x02, 0x65, 0x6e, 0x55, 0x6e, 0x69, 0x74, 0x20, 0x54, 0x65, 0x73, 0x74 };
  record.setPayload(payload, sizeof(payload));

  NdefRecord record2 = NdefRecord();
  record2 = record;

  assertEqual(TNF_WELL_KNOWN, record.getTnf());
  assertEqual(sizeof(recordType), record2.getTypeLength());
  assertEqual(sizeof(payload), record2.getPayloadLength());

  ::String typeCheck = record.getType();
  assertTrue(typeCheck.equals("T"));
  
  byte payload2[record2.getPayloadLength()];
  record2.getPayload(payload2); 
  assertBytesEqual(payload, payload2, sizeof(payload));  
}

test(getEmptyPayload)
{
  NdefRecord r = NdefRecord();
  assertEqual(TNF_EMPTY, r.getTnf());
  assertEqual(0, r.getPayloadLength());
  
  byte payload[r.getPayloadLength()];
  r.getPayload(payload);
  
  byte empty[0];
  assertBytesEqual(empty, payload, sizeof(payload));
}

void loop() {
  Test::run();
}
