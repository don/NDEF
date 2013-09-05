# NDEF Library for Arduino

Read and Write NDEF messages on NFC Tags with Arduino.

NFC Data Exchange Format (NDEF) is a common data format that operates across all NFC devices, regardless of the underlying tag or device technology.

This code works with the [Adafruit NFC Shield](https://www.adafruit.com/products/789), [Seeed Studio NFC Shield v2.0](http://www.seeedstudio.com/depot/nfc-shield-v20-p-1370.html) and the [Seeed Studio NFC Shield](http://www.seeedstudio.com/depot/nfc-shield-p-916.html?cPath=73). The library supports I2C for the Adafruit shield and SPI with the Seeed shields. The Adafruit Shield can also be modified to use SPI. It should also work with the [Adafruit NFC Breakout Board]([Adafruit NFC Shield](https://www.adafruit.com/products/364) but hasn't been tested.

### Supports 
 - Reading from Mifare Classic Tags with 4 byte UIDs.
 - Writing to Mifare Classic Tags with 4 byte UIDs.
 - Reading from Mifare Ultralight tags.

### Requires

[Yihui Xiong's PN532 Library](https://github.com/Seeed-Studio/PN532)

## Getting Started

To use the Ndef library in your code, include the following in your sketch

For the Adafruit Shield using I2C 

    #include <Wire.h>
    #include <PN532_I2C.h>
    #include <PN532.h>
    #include <NfcAdapter.h>
    
    PN532_I2C pn532_i2c(Wire);
    NfcAdapter nfc = NfcAdapter(pn532_i2c);

For the Seeed Shield using SPI

    #include <SPI.h>
    #include <PN532_SPI.h>
    #include <PN532.h>
    #include <NfcAdapter.h>
    
    PN532_SPI pn532spi(SPI, 10);
    NfcAdapter nfc = NfcAdapter(pn532spi);

### NfcAdapter

The user interacts with the NfcAdapter to read and write NFC tags using the NFC shield.

Read a message from a tag

    if (nfc.tagPresent()) {
        NfcTag tag = nfc.read();
        tag.print();
    }

Write a message to a tag

    if (nfc.tagPresent()) {
        NdefMessage message = NdefMessage();
        message.addTextRecord("Hello, Arduino!");
        nfc.write(message);
    }

### NfcTag 

Reading a tag with the shield, returns a NfcTag object. The NfcTag object contains meta data about the tag UID, technology, size.  When an NDEF tag is read, the NfcTag object contains a NdefMessage.

### NdefMessage

A NdefMessage consist of one or more NdefRecords.

The NdefMessage object has helper methods for adding records.

    ndefMessage.addTextRecord("hello, world");
    ndefMessage.addUriRecord("http://arduino.cc");

The NdefMessage object is responsible for encoding NdefMessage into bytes so it can be written to a tag. The NdefMessage also decodes bytes read from a tag back into a NdefMessage object.

### NdefRecord

A NdefRecord carries a payload and info about the payload within a NdefMessage.

### Specifications

This code is based on the "NFC Data Exchange Format (NDEF) Technical Specification" and the "Record Type Definition Technical Specifications" that can be downloaded from the [NFC Forum](http://www.nfc-forum.org/specs/spec_license).

### Tests

To run the tests, you'll need [ArduinoUnit](https://github.com/mmurdoch/arduinounit). To "install", I clone the repo to my home directory and symlink the source into ~/Documents/Arduino/libraries/ArduinoUnit. Note that I'm using an [old version of Arduino Unit](https://github.com/mmurdoch/arduinounit/tree/43a74e716ad18412f147cebe621038ed1b1d085d).

    $ cd ~
    $ git clone git@github.com:mmurdoch/arduinounit.git
    $ cd ~/Documents/Arduino/libraries/
    $ ln -s ~/arduinounit/src ArduinoUnit
    
Tests can be run on an Uno without a NFC shield, since the NDEF logic is what is being tested.
    
## Warning

This software is in development. It works for the happy path. Error handling could use improvement. It runs out of memory, especially on the Uno board. Use small messages with the Uno. The Due board can write larger messages. Please submit patches.

## License

[BSD License](https://github.com/don/Ndef/blob/master/LICENSE.txt) (c) 2013, Don Coleman
