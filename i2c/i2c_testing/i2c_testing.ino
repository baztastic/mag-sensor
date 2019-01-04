#include <Wire.h>
//#include "TLV493D.h"

int i2cAddress = 0x5e; //B1011110

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.beginTransmission(i2cAddress);
  Wire.write(B1);
  Wire.endTransmission();
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(Wire.read());
}
