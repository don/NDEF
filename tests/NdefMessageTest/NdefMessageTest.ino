#include <Wire.h>
#include <PN532.h>
#include <NdefMessage.h>
#include <NdefRecord.h>
#include <ArduinoUnit.h>
#include <Bounce2.h>

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
}

test(messageDelete)
{
  int start = freeMemory();

  NdefMessage* m1 = new NdefMessage();
  m1->addTextRecord("Foo");
  delete m1;

  int end = freeMemory();
//  Serial.print("Start ");Serial.println(start);
//  Serial.print("End ");Serial.println(end);
  assertEqual(0, (start-end));
}


test(assign)
{
  int start = freeMemory();
  
  if (true) // bogus block so automatic storage duration objects are deleted
  {
    NdefMessage* m1 = new NdefMessage();
    m1->addTextRecord("We the People of the United States, in Order to form a more perfect Union...");
    
    NdefMessage* m2 = new NdefMessage();
    
    *m2 = *m1;
    
    NdefRecord r1 = m1->getRecord(0);
    NdefRecord r2 = m2->getRecord(0);
    
    assertEqual(r1.getTnf(), r2.getTnf());
    assertEqual(r1.getTypeLength(), r2.getTypeLength());
    assertEqual(r1.getPayloadLength(), r2.getPayloadLength());
    assertEqual(r1.getIdLength(), r2.getIdLength());
      
    //byte p1[r1.getPayloadLength()];
    //byte p2[r2.getPayloadLength()];
    const byte *p1 = r1.getPayload();
    const byte *p2 = r2.getPayload();
      
    int size = r1.getPayloadLength();
    assertBytesEqual(p1, p2, size);
  
    delete m2;
    delete m1;
  }
      
  int end = freeMemory();
  assertEqual(0, (start-end));
}

test(assign2)
{
  int start = freeMemory();
  
  if (true) // bogus block so automatic storage duration objects are deleted
  {
    NdefMessage m1 = NdefMessage();
    m1.addTextRecord("We the People of the United States, in Order to form a more perfect Union...");
    
    NdefMessage m2 = NdefMessage();
    
    m2 = m1;
    
    NdefRecord r1 = m1.getRecord(0);
    NdefRecord r2 = m2.getRecord(0);
    
    assertEqual(r1.getTnf(), r2.getTnf());
    assertEqual(r1.getTypeLength(), r2.getTypeLength());
    assertEqual(r1.getPayloadLength(), r2.getPayloadLength());
    assertEqual(r1.getIdLength(), r2.getIdLength());
  
    // TODO check type
  
    //byte p1[r1.getPayloadLength()];
    //byte p2[r2.getPayloadLength()];
    const byte * p1 = r1.getPayload();
    const byte * p2 = r2.getPayload();
 
    int size = r1.getPayloadLength();
    assertBytesEqual(p1, p2, size);
  }

  int end = freeMemory();
  assertEqual(0, (start-end));
}

test(assign3)
{
  int start = freeMemory();
  
  if (true) // bogus block so automatic storage duration objects are deleted
  {

    NdefMessage* m1 = new NdefMessage();
    const char text[77] = "We the People of the United States, in Order to form a more perfect Union...";
    Serial.print("strlen() of 77-char c-string: ");
    Serial.println(strlen(text));

    m1->addTextRecord(text);
    
    NdefMessage* m2 = new NdefMessage();
    
    *m2 = *m1;
    
    delete m1;
    
    NdefRecord r = m2->getRecord(0);
      
    assertEqual(TNF_WELL_KNOWN, r.getTnf());
    assertEqual(1, r.getTypeLength());
    assertEqual(81, r.getPayloadLength());    // 76 chars (excluding \0) of payload + 5-byte prefix
    assertEqual(0, r.getIdLength());
    
    ::String s = "We the People of the United States, in Order to form a more perfect Union...";
    Serial.print("length() of String: ");
    Serial.println(s.length());

    byte payload[s.length() + 1];
    s.getBytes(payload, sizeof(payload));   // This should copy 77 characters, so include the \0
    PrintHex(payload, 10);
    //byte p[r.getPayloadLength()];
    const byte *p = r.getPayload();
    PrintHex(p, 10);
    assertBytesEqual(payload, p+5, s.length());   // Offset must be 81 - 76 = 5
  
    delete m2;
  }
  
  int end = freeMemory();
  assertEqual(0, (start-end));
}

test(assign4)
{
  int start = freeMemory();
  
  if (true) // bogus block so automatic storage duration objects are deleted
  {

    NdefMessage* m1 = new NdefMessage();
    m1->addTextRecord("We the People of the United States, in Order to form a more perfect Union...");
    
    NdefMessage* m2 = new NdefMessage();
    m2->addTextRecord("Record 1");
    m2->addTextRecord("RECORD 2");
    m2->addTextRecord("Record 3");

    assertEqual(3, m2->getRecordCount());    
    *m2 = *m1;
    assertEqual(1, m2->getRecordCount());
    
//    NdefRecord ghost = m2->getRecord(1);
//    ghost.print();
//    
//    NdefRecord ghost2 = m2->getRecord(3);
//    ghost2.print();

//    
//    delete m1;
//    
//    NdefRecord r = m2->getRecord(0);
//      
//    assertEqual(TNF_WELL_KNOWN, r.getTnf());
//    assertEqual(1, r.getTypeLength());
//    assertEqual(79, r.getPayloadLength());
//    assertEqual(0, r.getIdLength());
//    
//    String s = "We the People of the United States, in Order to form a more perfect Union...";
//    byte payload[s.length() + 1];
//    s.getBytes(payload, sizeof(payload));
//  
//    uint8_t* p = r.getPayload();  
//    int size = r.getPayloadLength();
//    assertBytesEqual(payload, p+3, s.length());
//    free(p);
  
    delete m1;
    delete m2;
  }
  
  int end = freeMemory();
  assertEqual(0, (start-end));
}

// really a record test
test(doublePayload)
{
  int start = freeMemory();
  
  NdefRecord* r = new NdefRecord();
  uint8_t p1[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 };
  r->setPayload(p1, sizeof(p1));
  r->setPayload(p1, sizeof(p1));
  
  delete r;
  
  int end = freeMemory();
  assertEqual(0, (start-end));
}


test(message_packaging_size)
{
  NdefMessage m;
  m.addTextRecord("012345678901234567890123456789012345678901234567890123456789");  // 60-char string (excluding \0)
  m.addTextRecord("012345678901234567890123456789012345678901234567890123456789");
  m.addTextRecord("012345678901234567890123456789012345678901234567890123456789");

  NdefRecord r = m.getRecord(0);
  uint8_t rSize = r.getEncodedSize();
  Serial.print("Encoded record uses ");
  Serial.print(rSize);
  Serial.println(" bytes.");  

  // Confirms expected headers when total message payload length < 254 bytes
  assertEqual(rSize*3, m.getEncodedSize());
  assertEqual(rSize*3 + 3, m.getPackagedSize());

  m.addTextRecord("012345678901234567890123456789012345678901234567890123456789");

  // Now 4*60 = 240 bytes, plus record headers, puts us over 254 bytes 
  //  -> 3-byte TLV length specification + 0x03 (start) + 0xFE (end) = 5 bytes 
  assertEqual(rSize*4, m.getEncodedSize());
  assertEqual(rSize*4 + 5, m.getPackagedSize());
}


test(message_packaged_content)
{
  NdefMessage m;
  NdefRecord r;
  byte payload[3] = {0xAA, 0xBB, 0xCC};

  r.setTnf(TNF_UNKNOWN);
  r.setPayload(payload, 3);
  m.addRecord(r);

  uint8_t len = m.getPackagedSize();
  uint8_t p[len];
  m.getPackaged(p);

  PrintHex(p, len);
  assertEqual(0x03, p[0]);      // Start of message tag
  assertEqual(0xFE, p[len-1]);  // End of message terminator
  assertEqual(0xCC, p[len-2]);  // End of record contents
}


// Opportunities for screw-ups in size type width and header logic exist once records are >254 bytes
test(big_record_handling)
{
  NdefRecord r;
  uint16_t len = 300;
  byte payload[len];
  randomSeed(0xB8);
  for (uint16_t i = 0; i < len; i++) {
    payload[i] = random(len);
  }

  r.setPayload(payload, len);
  r.setTnf(TNF_UNKNOWN);

  assertEqual(len, r.getPayloadLength());

  Serial.print("Record with payload size ");
  Serial.print(len);
  Serial.print(" bytes has encoded size ");
  uint16_t e_len = r.getEncodedSize();
  Serial.println(e_len);
  assertTrue(e_len > len);

  NdefMessage m;
  m.addRecord(r);
  assertEqual(r.getEncodedSize(), m.getEncodedSize());
  assertEqual(r.getEncodedSize() + 5, m.getPackagedSize());
  Serial.print("NDEF package size: ");
  Serial.println(m.getPackagedSize());

  NdefRecord r2;
  uint16_t len2 = 64;
  byte payload2[len2];
  payload2[0] = 0x12;
  r2.setPayload(payload2, len2);
  r2.setTnf(TNF_UNKNOWN);

  m.addRecord(r2);
  assertEqual(r.getEncodedSize() + r2.getEncodedSize() + 5, m.getPackagedSize());

  Serial.print("Memory before packaging: ");
  Serial.println(freeMemory());
  byte package[m.getPackagedSize()];
  m.getPackaged(package);
  Serial.print("Memory after packaging: ");
  Serial.println(freeMemory());

  bool found = false;
  for (uint16_t i = 0; i < m.getPackagedSize(); i++) {
    if (package[i] == 0x12) { found = true; break; }
  }
  assertTrue(found);

  //Confirm that the termination byte is in the right spot
  assertEqual(0xFE, package[m.getPackagedSize()-1]);

  //Confirm that the entire payload was copied into place (sounds paranoid, right?)
  uint16_t offset = m.getHeaderSize() + r.getEncodedSize() - len;
  for (uint16_t i = 0; i < len; i++) {
    if (payload[i] != package[i+offset]) {
      Serial.print("Package inconsistency at payload index ");
      Serial.print(i);
      Serial.println("; data:");
      PrintHex(&payload[i], 8);
      Serial.print("Package data at index ");
      Serial.println(i+offset);
      PrintHex(&package[i+offset], 8);
    }
    assertEqual(payload[i], package[i+offset]);
  } 
}


test(payload_offset_calculation) {
  NdefRecord r;
  uint16_t len = 300;
  byte payload[len];
  payload[0] = 0xAA;
  r.setPayload(payload, len);
  r.setTnf(TNF_UNKNOWN);


  NdefRecord r2(r);
  payload[0] = 0xBB;
  r2.setPayload(payload, len);

  NdefMessage m;
  m.addRecord(r);
  m.addRecord(r2);

  uint16_t pSize = m.getPackagedSize();
  byte package[pSize];
  m.getPackaged(package);

  PrintHex(&package[m.getOffset(0)-3], 8);

  assertEqual(0xAA, package[m.getOffset(0)]);
  assertEqual(0xBB, package[m.getOffset(1)]);
}


test(aaa_printFreeMemoryAtStart)  //  warning: relies on fact tests are run in alphabetical order
{
  Serial.println(F("---------------------"));
  Serial.print("Free Memory Start ");Serial.println(freeMemory());
  Serial.println(F("---------------------"));
}

test(zzz_printFreeMemoryAtEnd)   // warning: relies on fact tests are run in alphabetical order
{
  // unfortunately the user needs to manually check this matches the start value
  Serial.println(F("====================="));
  Serial.print("Free Memory End ");Serial.println(freeMemory());
  Serial.println(F("====================="));
}



void loop() {
  Test::run();
}