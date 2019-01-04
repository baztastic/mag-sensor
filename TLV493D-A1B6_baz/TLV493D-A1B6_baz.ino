// Adapted from https://forum.arduino.cc/index.php?topic=419380.0
// Define includes
#include <Wire.h>        // Wire header file for I2C and 2 wire

const int xMask[2][8] = {
  { -2048, 1024, 512, 256, 128, 64, 32, 16},
  {8, 4, 2, 1, 0, 0, 0, 0}
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
  { -2048, 1024, 512, 256, 0, 0, 0, 0},  // {msb},
  {128, 64, 32, 16, 8, 4, 2, 1}          // {lsb}
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

int HallAddressWrite = 0x1F;   // Default device address
byte X_Axis_Register1 = 0x1;  //

// Declare some variables
int const numOfBytes = 7;
int baudRate = 115200;
byte readByte[numOfBytes];

byte configReg = 0x00;  // Address of Configuration
//byte powerMode = 0x05;   // Set to low power mode
byte powerMode = 0x06;   // Set to fast power mode

float temp,x,y,z; // variables for x,y,z measurements
long oldCounter = 0;
long counter = 0;
int channel = 1;
int pd = 1;
byte factorySettings[3];

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(baudRate);            // Set Serial Port speed
  delay(1000);
//  Wire.pins(4, 5);
  Wire.begin();                      // Join the I2C bus as a master
  Wire.setClock(400000L);
  Serial.println("Starting!");

  Wire.beginTransmission(HallAddressWrite);       // Address the sensor
  Wire.write(configReg);              // Address the Configuration register
  Wire.write(powerMode);              // Set the Power Mode
  Wire.endTransmission();             // Stop transmitting
  delay(100);
//  getFactorySettings(HallAddressWrite, X_Axis_Register1, 7, factorySettings);
//  Serial.println(factorySettings[0]);
//  Serial.println(factorySettings[1]);
//  Serial.println(factorySettings[2]);
//  Serial.println("Finished setup");
}

// Main Program Infinite loop
void loop() {
  getBytes(HallAddressWrite, X_Axis_Register1, numOfBytes, readByte); //Read first 7 bytes
  oldCounter = counter;
//  getFrameCounter();
  counter += (long)getMagnitude(frameBytes, frameMask, 'f');
    
//  channel = (int)getMagnitude(chBytes, chMask, 'c');
//  pd = (int)getMagnitude(pdBytes, pdMask, 'p');
//  Serial.println(String(counter) + "\t" + String(channel) + "\t" + String(pd));
  
  if ( counter == oldCounter ){
    return;
  }
  
  temp = getMagnitude(tempBytes, tempMask, 'T');
  if(temp<-50){ //re-read address banks, bad measurement
    while(temp<-50){
      getBytes(HallAddressWrite, X_Axis_Register1, numOfBytes, readByte); //Read first 8 bytes
      temp = getMagnitude(tempBytes, tempMask, 'T');
    }
  }

  x = getMagnitude(xBytes, xMask, 'x');
  y = getMagnitude(yBytes, yMask, 'y');
  z = getMagnitude(zBytes, zMask, 'z');
  
  Serial.print(String(millis())+",");
  Serial.print(String(temp) + ",");
  Serial.print(String(x) + ",");
  Serial.print(String(y) + ",");
  Serial.print(String(z) + ",");
  Serial.print(String(x-y+z));
  Serial.print("," + String(counter));
  Serial.print("\n");
  delay(10);
}

void getBytes(byte address, byte registry, int numOfBytes, byte* readByte)
{
  byte error = 1;
  digitalWrite(13, HIGH);
  
  while(error != 0){
    Serial.print("requesting ");
    Wire.beginTransmission(address);  //Begin Transmission
    //Ask for data register
    Wire.write(registry);
    error = Wire.endTransmission(true);             //End Transmission
    Serial.println(error);
  }
//  else
//  {
//    Serial.println(error);
//  }
  while(pd != 0 && channel != 0){
    delay(10);
    Serial.print("try ");
    Wire.requestFrom(address, numOfBytes, true);     //Request Transmission
    Serial.print("before ");
    int i = 0;
    while(Wire.available())
    {
      Serial.print(String(i));
      readByte[i] = Wire.read();
      i++;
    }
    if(i == 0 || i < numOfBytes)
    {
      delay(100);
      Serial.println("BUFFER NOT FULL");
      break;
    }
    Serial.print(" after ");
    channel = (int)getMagnitude(chBytes, chMask, 'c');
    pd = (int)getMagnitude(pdBytes, pdMask, 'p');
  }
  Serial.println("done");
  channel = 1;
  pd = 1;
  digitalWrite(13, LOW);
}

void getFactorySettings(byte address, byte registry, int numToSkip, byte* factorySettings)
{
  Wire.beginTransmission(address);
  Wire.write(registry);
  Wire.endTransmission();
  Wire.requestFrom(address, 10); // first 7 bytes are readings, next 3 are factory settings
  for (int i = 0; i < 10; i++){
    if (i >= numToSkip)
    {
      factorySettings[i] = Wire.read();
    }
    else
    {
      Wire.read();
    }
  }
  Wire.endTransmission();
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
  
  if ( dataType == ( 'x' || 'y' || 'z' || 'd' ) ){
    mag *= 0.098 * 10.0; //0.098mT/LSB 10Gauss/mT
//  if(abs(magX)<4)   //the sensor has about a 0.2mT | 2Gauss units drift
//    magX = 0;       //this is a software filter that suppress most of the noise
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
