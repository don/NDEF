# NDEF Library for Arduino 

Read and Write NDEF messages on NFC Tags

Supports 
 - Reading from Mifare Classic Tags with 4 byte UIDs.
 - Writing to Mifare Classic Tags with 4 byte UIDs.
 - Reading from Mifare Ultralight tags.

## Requires

[Adafruit NFC Shield](https://www.adafruit.com/products/364)

[Adafruit NFC Shield I2C library](https://github.com/don/Adafruit_NFCShield_I2C) you'll need the "guard" branch
	
	$ cd ~/Documents/Ardiuino/library
	$ git clone https://github.com/don/Adafruit_NFCShield_I2C
	$ cd Adafruit_NFCShield_I2C
	$ git branch --track guard origin/guard
	$ git checkout guard

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

Reading a tag with the sheild, returns a NfcTag object. The NfcTag object contains meta data about the tag UID, technology, size.  When an NDEF tag is read, the NfcTag object contains a NdefMessage.

### NdefMessage

A NdefMessage consist of one or more NdefRecords.

The NdefMessage object has helper methods for adding records.

    ndefMessage.addTextRecord("hello, world");
    ndefMessage.addUriRecord("http://arduino.cc");

The NdefMessage object is responsible for encoding NdefMessage into bytes so it can be written to a tag. The NdefMessage also decodes bytes read from a tag back into a NdefMessage object.

### NdefRecord

A NdefRecord carrys a payload and info about the payload within a NdefMessage.

### Specifcations

This code is based on the "NFC Data Exchange Format (NDEF) Technical Specification" and the "Record Type Definition Technical Specifications" that can be downloaded from the [NFC Forum](http://www.nfc-forum.org/specs/spec_license).

### Tests

To run the tests, you'll need [ArduinoUnit](https://github.com/mmurdoch/arduinounit). To "install", I clone the repo to my home directory and symlink the source into ~/Documents/Arduino/libraries/ArduinoUnit.

    $ cd ~
    $ git clone git@github.com:mmurdoch/arduinounit.git
    $ cd ~/Documents/Arduino/libraries/
    $ ln -s ~/arduinounit/src ArduinoUnit
    
Tests can be run on an Uno without a NFC shield, since the NDEF logic is what is being tested.
    
## Warning

This software is in active development. It works for the happy path. Error handling is bad. It runs out of memory, especially on the Uno board. Use small messages with the Uno. The Due board can write larger messages.
