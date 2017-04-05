#include "ntag.h"
#ifdef ARDUINO_STM_NUCLEU_F103RB
#include "HardWire.h"
HardWire HWire(1, I2C_REMAP);// | I2C_BUS_RESET); // I2c1
#else
#include "Wire.h"
#define HWire Wire
#endif
#include <stdio.h>


Ntag::Ntag(DEVICE_TYPE dt, byte fd_pin, byte vout_pin, byte i2c_address):
    _dt(dt),
    _fd_pin(fd_pin),
    _vout_pin(vout_pin),
    _i2c_address(i2c_address),
    _rfBusyStartTime(0),
    _triggered(false)
{
    _debouncer = Bounce();
}

bool Ntag::begin(){
    bool bResult=true;
    HWire.begin();
#ifndef ARDUINO_SAM_DUE
    HWire.beginTransmission(_i2c_address);
    bResult=HWire.endTransmission()==0;
#else
    //Arduino Due always sends at least 2 bytes for every I²C operation.  This upsets the NTAG.
    return true;
#endif
    if(_vout_pin!=0){
        pinMode(_vout_pin, INPUT);
    }
    pinMode(_fd_pin, INPUT);
    _debouncer.attach(_fd_pin);
    _debouncer.interval(5); // interval in ms
    return bResult;
}

bool Ntag::isReaderPresent()
{
    if(_vout_pin==0)
    {
        return false;
    }
    return digitalRead(_vout_pin)==HIGH;
}

void Ntag::detectI2cDevices(){
    for(byte i=0;i<0x80;i++){
        HWire.beginTransmission(i);
        if(HWire.endTransmission()==0)
        {
            Serial.print("Found I²C device on : 0x");
            Serial.println(i,HEX);
        }
    }
}

byte Ntag::getUidLength()
{
    return UID_LENGTH;
}

bool Ntag::getUid(byte *uid, unsigned int uidLength)
{
    byte data[UID_LENGTH];
    if(!readBlock(CONFIG, 0,data,UID_LENGTH))
    {
        return false;
    }
    if(data[0]!=4)
    {
        return false;
    }
    memcpy(uid, data, UID_LENGTH < uidLength ? UID_LENGTH : uidLength);
    return true;
}


bool Ntag::setFd_ReaderHandshake(){
    //return writeRegister(NC_REG, 0x3C,0x18);
    return writeRegister(NC_REG, 0x3C,0x28);
    //0x28: FD_OFF=10b, FD_ON=10b : FD constant low
    //Start of read by reader always clears the FD-pin.
    //At the end of the read by reader, the FD-pin becomes high (most of the times)
    //0x18: FD pulse high (13.9ms wide) at the beginning of the read sequence, no effect on write sequence.
    //0x14: FD_OFF=01b, FD_ON=01b : FD constant high
    //0x24: FD constant high
}

bool Ntag::isRfBusy(){
    byte regVal;
    const byte RF_LOCKED=5;
    _debouncer.update();
    //Reading this register clears the FD-pin.
    //When continuously polling this register while RF reading or writing is ongoing, high will be returned for 2ms, followed
    //by low for 9ms, then high again for 2ms then low again for 9ms and so on.
    //To get a nice clean high or low instead of spikes, a software retriggerable monostable that triggers on rfBusy will be used.
    if(!readRegister(NS_REG, regVal))
    {
        Serial.println("Can't read register.");
    }
    if(bitRead(regVal,RF_LOCKED) || _debouncer.rose())
    {
        //retrigger monostable
        _rfBusyStartTime=millis();
        _triggered=true;
        return true;
    }
    if(_triggered && millis()<_rfBusyStartTime+30)
    {
        //a zero has been read, but monostable hasn't run out yet
        return true;
    }
    return false;
}

//Mirror SRAM to EEPROM
//Remark that the SRAM mirroring is only valid for the RF-interface.
//For the I²C-interface, you still have to use blocks 0xF8 and higher to access SRAM area (see datasheet Table 6)
bool Ntag::setSramMirrorRf(bool bEnable, byte mirrorBaseBlockNr){
    _mirrorBaseBlockNr = bEnable ? mirrorBaseBlockNr : 0;
    if(!writeRegister(SRAM_MIRROR_BLOCK,0xFF,mirrorBaseBlockNr)){
        return false;
    }
    //disable pass-through mode (because it's not compatible with SRAM⁻mirroring: datasheet §11.2).
    //enable/disable SRAM memory mirror
    return writeRegister(NC_REG, 0x42, bEnable ? 0x02 : 0x00);
}

bool Ntag::readSram(word address, byte *pdata, byte length)
{
    return read(SRAM, address+SRAM_BASE_ADDR, pdata, length);
}

bool Ntag::writeSram(word address, byte *pdata, byte length)
{
    return write(SRAM, address+SRAM_BASE_ADDR, pdata, length);
}

bool Ntag::readEeprom(word address, byte *pdata, byte length)
{
    return read(USERMEM, address+EEPROM_BASE_ADDR, pdata, length);
}

bool Ntag::writeEeprom(word address, byte *pdata, byte length)
{
    return write(USERMEM, address+EEPROM_BASE_ADDR, pdata, length);
}

void Ntag::releaseI2c()
{
    //reset I2C_LOCKED bit
    writeRegister(NS_REG,0x40,0);
}

bool Ntag::write(BLOCK_TYPE bt, word address, byte* pdata, byte length)
{
    byte readbuffer[NTAG_BLOCK_SIZE];
    byte writeLength;
    byte* wptr=pdata;
    byte blockNr=address/NTAG_BLOCK_SIZE;

    if(address % NTAG_BLOCK_SIZE !=0)
    {
        //start address doesn't point to start of block, so the bytes in this block that precede the address range must
        //be read.
        if(!readBlock(bt, blockNr, readbuffer, NTAG_BLOCK_SIZE))
        {
            return false;
        }
        writeLength=min(NTAG_BLOCK_SIZE - (address % NTAG_BLOCK_SIZE), length);
        memcpy(readbuffer + (address % NTAG_BLOCK_SIZE), pdata, writeLength);
        if(!writeBlock(bt, blockNr, readbuffer))
        {
            return false;
        }
        wptr+=writeLength;
        blockNr++;
    }
    while(wptr < pdata+length)
    {
        writeLength=(pdata+length-wptr > NTAG_BLOCK_SIZE ? NTAG_BLOCK_SIZE : pdata+length-wptr);
        if(writeLength!=NTAG_BLOCK_SIZE){
            if(!readBlock(bt, blockNr, readbuffer, NTAG_BLOCK_SIZE))
            {
                return false;
            }
            memcpy(readbuffer, wptr, writeLength);
        }
        if(!writeBlock(bt, blockNr, writeLength==NTAG_BLOCK_SIZE ? wptr : readbuffer))
        {
            return false;
        }
        wptr+=writeLength;
        blockNr++;
    }
    _lastMemBlockWritten = --blockNr;
    return true;
}

bool Ntag::read(BLOCK_TYPE bt, word address, byte* pdata,  byte length)
{
    byte readbuffer[NTAG_BLOCK_SIZE];
    byte readLength;
    byte* wptr=pdata;

    readLength=min(NTAG_BLOCK_SIZE, (address % NTAG_BLOCK_SIZE) + length);
    if(!readBlock(bt, address/NTAG_BLOCK_SIZE, readbuffer, readLength))
    {
        return false;
    }
    readLength-=address % NTAG_BLOCK_SIZE;
    memcpy(wptr,readbuffer + (address % NTAG_BLOCK_SIZE), readLength);
    wptr+=readLength;
    for(byte i=(address/NTAG_BLOCK_SIZE)+1;wptr<pdata+length;i++)
    {
        readLength=(pdata+length-wptr > NTAG_BLOCK_SIZE ? NTAG_BLOCK_SIZE : pdata+length-wptr);
        if(!readBlock(bt, i, wptr, readLength))
        {
            return false;
        }
        wptr+=readLength;
    }
    return true;
}

bool Ntag::readBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data, byte data_size)
{
    if(data_size>NTAG_BLOCK_SIZE || !writeBlockAddress(bt, memBlockAddress)){
        return false;
    }
    if(!end_transmission()){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,data_size)!=data_size){
        return false;
    }
    byte i=0;
    while(HWire.available())
    {
        p_data[i++] = HWire.read();
    }
    return i==data_size;
}

bool Ntag::setLastNdefBlock()
{
    //When SRAM mirroring is used, the LAST_NDEF_BLOCK must point to USERMEM, not to SRAM
    return writeRegister(LAST_NDEF_BLOCK, 0xFF, isAddressValid(SRAM, _lastMemBlockWritten) ?
                             _lastMemBlockWritten - (SRAM_BASE_ADDR>>4) + _mirrorBaseBlockNr : _lastMemBlockWritten);
}

bool Ntag::writeBlock(BLOCK_TYPE bt, byte memBlockAddress, byte *p_data)
{
    if(!writeBlockAddress(bt, memBlockAddress)){
        return false;
    }
    for (int i=0; i<NTAG_BLOCK_SIZE; i++)
    {
        if(HWire.write(p_data[i])!=1){
            break;
        }
    }
    if(!end_transmission()){
        return false;
    }
    switch(bt){
    case CONFIG:
    case USERMEM:
        delay(5);//16 bytes (one block) written in 4.5 ms (EEPROM)
        break;
    case REGISTER:
    case SRAM:
        delayMicroseconds(500);//0.4 ms (SRAM - Pass-through mode) including all overhead
        break;
    }
    return true;
}

bool Ntag::readRegister(REGISTER_NR regAddr, byte& value)
{
    value=0;
    bool bRetVal=true;
    if(regAddr>6 || !writeBlockAddress(REGISTER, 0xFE)){
        return false;
    }
    if(HWire.write(regAddr)!=1){
        bRetVal=false;
    }
    if(!end_transmission()){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    if(HWire.requestFrom(_i2c_address,(byte)1)!=1){
        return false;
    }
    value=HWire.read();
    return bRetVal;
}

bool Ntag::writeRegister(REGISTER_NR regAddr, byte mask, byte regdat)
{
    bool bRetVal=false;
    if(regAddr>7 || !writeBlockAddress(REGISTER, 0xFE)){
        return false;
    }
    if (HWire.write(regAddr)==1 &&
            HWire.write(mask)==1 &&
            HWire.write(regdat)==1){
        bRetVal=true;
    }
    return end_transmission() && bRetVal;
}

bool Ntag::writeBlockAddress(BLOCK_TYPE dt, byte addr)
{
    if(!isAddressValid(dt, addr)){
        return false;
    }
    HWire.beginTransmission(_i2c_address);
    return HWire.write(addr)==1;
}

bool Ntag::end_transmission(void)
{
    return HWire.endTransmission()==0;
    //I2C_LOCKED must be either reset to 0b at the end of the I2C sequence or wait until the end of the watch dog timer.
}

bool Ntag::isAddressValid(BLOCK_TYPE type, byte blocknr){
    switch(type){
    case CONFIG:
        if(blocknr!=0){
            return false;
        }
        break;
    case USERMEM:
        switch (_dt) {
        case NTAG_I2C_1K:
            if(blocknr < 1 || blocknr > 0x38){
                return false;
            }
            break;
        case NTAG_I2C_2K:
            if(blocknr < 1 || blocknr > 0x78){
                return false;
            }
            break;
        default:
            return false;
        }
        break;
    case SRAM:
        if(blocknr < 0xF8 || blocknr > 0xFB){
            return false;
        }
        break;
    case REGISTER:
        if(blocknr != 0xFE){
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}
