#include <Ndef.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\nNDEF Library\n");

  // 2 Text Records - TNF_WELL_KNOWN, RTD_TEXT, { "first", "second" }
  uint8_t data[] = { 0x91, 0x01, 0x08, 0x54, 0x02, 0x65, 0x6e, 0x66, 0x69, 0x72, 0x73, 0x74,
      0x51, 0x01, 0x09, 0x54, 0x02, 0x65, 0x6e, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64};

  NdefMessage message = NdefMessage(data, sizeof(data));
  
  message.print();

  Serial.println("\n");
  uint8_t encoded[message.getEncodedSize()];
  int i;
  for (i = 0; i < sizeof(encoded); i++)
  {
  	// zero out to help debugging
  	encoded[i] = 0x0;
  }

  message.encode(encoded);
  
  // assert encoded == data
  if (sizeof(data) == sizeof(encoded)) 
  {
    int i;
    for (i = 0; i < sizeof(data); i++) 
    {
      //Serial.print(data[i], HEX);Serial.print(" ");Serial.println(encoded[i], HEX);
    	if (data[i] != encoded[i])
    	{
    		Serial.print("Data mismatch at index ");Serial.println(i);    		
    		break;
    	}
    }        
  } 
  else 
  {
    Serial.println("Size Mismatch");
  }

}

void loop() {
  // put your main code here, to run repeatedly: 
  
}
