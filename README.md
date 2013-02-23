# NDEF Library for Arduino 

Read and Write NDEF messages on NFC Tags

Supports 
 - Reading from Mifare Classic Tags with 4 byte UIDs.
 - Writing to Mifare Classic Tags with 4 byte UIDs.
 - Reading from Mifare Ultralight tags.

## Requires

[Adafruit NFC Shield](https://www.adafruit.com/products/364)

[Adafruit NFC Shield I2C library](https://github.com/don/Adafruit_NFCShield_I2C) you'll need the "guard" branch)
	
	$ cd ~/Documents/Ardiuino/library
	$ git clone https://github.com/don/Adafruit_NFCShield_I2C
	$ cd Adafruit_NFCShield_I2C
	$ git branch --track guard origin/guard
	$ git checkout guard

## Warning

This software is in active development. It works for the happy path. Error handling is bad. It runs out of memory, especially on the Uno board. Use small messages with the Uno. The Due board can write larger messages.
