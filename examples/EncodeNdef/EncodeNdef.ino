#include <Ndef.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n");
  
  // create a record the long way  
  NdefRecord* r = new NdefRecord();
  r->setTnf(TNF_MIME_MEDIA); 
  
  String type = String("foo");
  String payload = String("17");

  byte typeBytes[type.length() + 1];
  type.getBytes(typeBytes, sizeof(typeBytes));
  r->setType(typeBytes, type.length());
  
  byte payloadBytes[payload.length() + 1];
  payload.getBytes(payloadBytes, sizeof(payloadBytes));
  r->setPayload(payloadBytes, payload.length());

  // create a message and add record
  NdefMessage* message = new NdefMessage();
  message->add(*r);
  
  // add more records using the helper methods
  message->addMimeMediaRecord("foo", "17");
  message->addMimeMediaRecord("foo", payloadBytes, payload.length());
  message->addTextRecord("foo");
  message->addTextRecord("foo", "en");
  message->addEmptyRecord();
  message->addUriRecord("http://example.com");

  // verify the contents
  message->print();
  
  // create an array to hold the encoded message
  uint8_t encoded[message->getEncodedSize()];
 
  message->encode(encoded);
  // TODO need NdefUtil::PrintHex(encoded)
  
  Serial.println("\nExpecting");
  Serial.println("0xd2 0x03 0x02 0x66 0x6f 0x6f 0x31 0x37...");

}

void loop() {
  // put your main code here, to run repeatedly: 
  
}
