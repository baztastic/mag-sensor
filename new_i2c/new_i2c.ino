// ONLY WORKS FOR ARDUINO UNO
#include "I2C.h"

const int xMask[2][8] = {
  { -2048, 1024, 512, 256, 128, 64, 32, 16}, // {msb},
  {8, 4, 2, 1, 0, 0, 0, 0}                   // {lsb}
};
const int yMask[2][8] = {
  { -2048, 1024, 512, 256, 128, 64, 32, 16},
  {0, 0, 0, 0, 8, 4, 2, 1}
};
const int zMask[2][8] = {
  { -2048, 1024, 512, 256, 128, 64, 32, 16},
  {0, 0, 0, 0, 8, 4, 2, 1}
};
const int tempMask[2][8] = {
  { -2048, 1024, 512, 256, 0, 0, 0, 0},  
  {128, 64, 32, 16, 8, 4, 2, 1}          
};
const int frameMask[2][8] = {
  {0, 0, 0, 0, 1, 1, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const int chMask[2][8] = {
  {0, 0, 0, 0, 0, 0, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const int pdMask[2][8] = {
  {0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
const int xBytes[2] =     {0, 4}; // {msb, lsb}
const int yBytes[2] =     {1, 4};
const int zBytes[2] =     {2, 5};
const int tempBytes[2] =  {3, 6};
const int frameBytes[2] = {3, 3}; // frame counter
const int chBytes[2] =    {3, 3}; // channel
const int pdBytes[2] =    {5, 5}; // power-down

uint8_t HallAddressWrite = 0x1F;    // Default device address
uint8_t X_Axis_Register1 = 0x01;    // Data register

uint8_t const numOfBytes = 7;
int baudRate = 19200;
byte readByte[numOfBytes];

uint8_t configReg1 = 0x00;   // Configuration register address
uint8_t configReg2 = 0x01;   // Configuration register address
//uint8_t powerMode = 0x05;    // Set to low power mode
uint8_t powerMode = 0x06;    // Set to fast power mode
//uint8_t tempDisable = 0x80;  // Disable temperature measurement
uint8_t tempDisable = 0x00;  // Enable temperature measurement

float temp,x,y,z; // Variables for temperature,x,y,z measurements
long oldCounter = 0;
long counter = 0;
int channel = 1;
int pd = 1;
byte factorySettings[3];

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(baudRate);
//  delay(1000);
  I2c.begin();
  I2c.setSpeed(1);
  I2c.timeOut(100);
//  I2c.scan();
  Serial.println("Starting!");
  I2c.write(HallAddressWrite, configReg1, powerMode);
  I2c.write(HallAddressWrite, configReg2, tempDisable);
  delay(100);
  Serial.println("t,x,y,z,sum,frame");
}

void loop() {
  getBytes(HallAddressWrite, X_Axis_Register1, numOfBytes, readByte); //Read first 7 bytes
  
  oldCounter = counter;
  counter += (long)getMagnitude(frameBytes, frameMask, 'f');

  if ( counter == oldCounter ){
    return;
  }
  else if (pd != 0){
    return;
  }
  else if (channel != 0){
    return;
  }
  else{
  }

//  temp = getMagT();
  x = getMagX();
  y = getMagY();
  z = getMagZ();
  
  Serial.print(String(millis())+",");
//  Serial.print(String(temp) + ",");
  Serial.print(String(x) + ",");
  Serial.print(String(y) + ",");
  Serial.print(String(z) + ",");
//  Serial.print(String(x-y+z) + ",");
  Serial.print(String(counter));
  Serial.print("\n");
//  delay(200);
}

void getBytes(uint8_t address, uint8_t registry, uint8_t numOfBytes, byte* readByte)
{
  byte error = 1;
  channel = 1;
  pd = 1;
  int retries = 0;
  while( pd != 0 || channel != 0 ){
    if(retries >= 5){break;} // prevent getting stuck in loop
    digitalWrite(13, HIGH);
    error = I2c.read(address, registry, numOfBytes, readByte);
//    Serial.println(String(error));
    digitalWrite(13, LOW);
    channel = (int)getMagnitude(chBytes, chMask, 'c');
    pd = (int)getMagnitude(pdBytes, pdMask, 'p');
    retries++;
  }
}

int getCounter(){
  char msb = readByte[3];
  msb = msb & 0xC;
  msb = msb >> 2;
  return msb;
}

int getChannel(){
  char msb = readByte[3];
  msb = msb & 0x3;
  return msb;
}

int getPD(){
  char msb = readByte[5];
  msb = msb & 0x10;
  msb = msb >> 4;
  return msb;
}

float getMagX(){
  char msb = readByte[0];
  char lsb = readByte[4];
  lsb = lsb & 0xf0;
  
  unsigned short xx;
  void *vp;
  char *pmsb;
  char *plsb;
  signed short zz;
  vp = &xx;
  plsb = (char *)vp;
  pmsb = plsb + 1;
  *pmsb = msb;
  *plsb = lsb;
  zz = xx & 0xfff0;
  zz = zz / 16;
  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
}

float getMagY(){
  char msb = readByte[1];
  char lsb = readByte[4];
  lsb = lsb & 0x0f;
  
  unsigned short xx;
  void *vp;
  char *pmsb;
  char *plsb;
  signed short zz;
  vp = &xx;
  plsb = (char *)vp;
  pmsb = plsb + 1;
  *pmsb = msb;
  *plsb = lsb;
  zz = xx & 0xfff0;
  zz = zz / 16;
  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
}

float getMagZ(){
  char msb = readByte[2];
  char lsb = readByte[5];
  lsb = lsb & 0x0f;
  
  unsigned short xx;
  void *vp;
  char *pmsb;
  char *plsb;
  signed short zz;
  vp = &xx;
  plsb = (char *)vp;
  pmsb = plsb + 1;
  *pmsb = msb;
  *plsb = lsb;
  zz = xx & 0xfff0;
  zz = zz / 16;
  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
}

float getMagT(){
  char msb = readByte[3];
  char lsb = readByte[6];
  msb = msb & 0xf0;
  msb = msb + (lsb >> 4);
  lsb = lsb & 0xf0;
  
  unsigned short xx;
  void *vp;
  char *pmsb;
  char *plsb;
  signed short zz;
  vp = &xx;
  plsb = (char *)vp;
  pmsb = plsb + 1;
  *pmsb = msb;
  *plsb = lsb;
  zz = xx & 0xfff0;
  zz = zz / 16;
  return ((float)zz - 320) * 1.1;
}

float getMagnitude(const int byteNum[2], const int bitMask[2][8], char dataType){
  int i;
  float mag = 0;
  int msb = readByte[byteNum[0]];
  int lsb = readByte[byteNum[1]];
  
  for ( i = 7; i >= 0; i--)
  {
    //msb conversion
    if ( (1 << i) & msb)
      mag += 1 * bitMask[0][7 - i];
    else
      mag += 0;
      
    //lsb conversion
    if ( (1 << i) & lsb)
      mag += 1 * bitMask[1][7 - i];
    else
      mag += 0;
  }
  
  if ( dataType ==  'H' ){
    mag *= 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT
  }
  else if ( dataType == 'T') {
    mag -= 320;
    mag *= 1.1;
  }
  else {
    // just return without calibration
  }
  return mag;
}
