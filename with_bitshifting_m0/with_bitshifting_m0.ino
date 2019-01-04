// ONLY WORKS FOR ARDUINO UNO
#include <C:\Users\Administrator\Dropbox\Link to Safe_Window_Box\Transformer Monitoring Project\MachineLearning\Magnetic Field Sensor\new_i2c\I2C.h>

uint8_t HallAddressWrite = 0x1F;    // Default device address
uint8_t X_Axis_Register1 = 0x01;    // Data register

uint8_t const numOfBytes = 7;
int baudRate = 14400;
byte readByte[numOfBytes];

uint8_t configReg1 = 0x00;   // Configuration register address MOD1
uint8_t configReg2 = 0x01;   // Configuration register address MOD2
//uint8_t powerMode = 0x05;    // Set to low power mode
uint8_t powerMode = 0x06;    // Set to fast power mode
//uint8_t tempDisable = 0x80;  // Disable temperature measurement
uint8_t tempDisable = 0x00;  // Enable temperature measurement

float temp,x,y,z; // Variables for temperature,x,y,z measurements
long oldCounter = 0;
long counter = 0;
int channel = 1;
int pd = 1;
//byte factorySettings[3];
byte error;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(baudRate);
//  delay(1000);
  I2c.begin();
  I2c.setSpeed(0);
  I2c.timeOut(100);
//  I2c.scan();
  Serial.println("Starting!");
  I2c.write(HallAddressWrite, configReg1, powerMode);
  I2c.write(HallAddressWrite, configReg2, tempDisable);
  delay(1000);
  Serial.println("t,x,y,z,sum,frame");
}

void loop() {
  getBytes(HallAddressWrite, X_Axis_Register1, numOfBytes, readByte); //Read first 7 bytes
//  I2c.read(HallAddressWrite, X_Axis_Register1, numOfBytes, readByte);
//  channel = getChannel();
//  pd = getPD();

  oldCounter = counter;
  counter += getCounter();

  if ( (counter == oldCounter) || (pd != 0) || (channel != 0) ){ return; }

//  temp = getMagT();
  x = getMagX();
  y = getMagY();
  z = getMagZ();
  
  Serial.print(String(millis())+",");
//  Serial.print(String(temp) + ",");
  Serial.print(String(x) + ",");
  Serial.print(String(y) + ",");
  Serial.print(String(z) + ",");
  Serial.print(String(x-y+z) + ",");
  Serial.print(String(counter));
  Serial.print("\n");
  
//  delay(200);
}

void getBytes(uint8_t address, uint8_t registry, uint8_t numOfBytes, byte* readByte)
{
  error = 1;
  channel = 1;
  pd = 1;
  int retries = 0;
  while( pd != 0 || channel != 0 ){
    if(retries >= 10){break;} // try to prevent getting stuck in loop
    digitalWrite(13, HIGH);
    error = I2c.read(address, registry, numOfBytes, readByte);
//    Serial.print(String(error) + " ");
    digitalWrite(13, LOW);
    channel = getChannel();
    pd = getPD();
    retries++;
  }
  if(retries >= 10){
    // kick the I2c bus back into action if it gets stuck
    delay(10);
    I2c.end();
    I2c.begin();
    I2c.setSpeed(1);
    I2c.timeOut(100);
    return;
  }
//  Serial.println();
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
  return zz;
//  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
}

float getMagY(){
  char msb = readByte[1];
  char lsb = readByte[4];
  lsb = lsb & 0x0f;
  lsb = lsb << 4;
  
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
  return zz;
//  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
}

float getMagZ(){
  char msb = readByte[2];
  char lsb = readByte[5];
  lsb = lsb & 0x0f;
  lsb = lsb << 4;
  
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
  return zz;
//  return (float)zz * 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT;
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
