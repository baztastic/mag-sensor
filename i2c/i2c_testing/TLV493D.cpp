#include "TLV493D.h"
#include "Arduino.h"
#include "Wire.h"
//
//TLV493D::TLV493D(){
//  i2cAddress = B1011110;
//}
//
//bool TLV493D::readReg(byte addr, byte *val){
//  Wire.beginTransmission(i2cAddress);
//  Wire.write(addr);
//  Wire.endTransmission();
//
//  Wire.requestFrom(i2cAddress,1);
//  int timeouts=0;
//  while(!Wire.available() && timeouts++<=LR_MAX_TRIES){
//    delay(10);
//  }
//  if (Wire.available()){
//    *val=Wire.read();
//    return true;
//  }
//  else{
//    return false;
//  }
//}

