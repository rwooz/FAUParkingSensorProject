#include <Wire.h> //I2C Arduino Library

#define addr 0x0D //I2C Address for The QMC5883

//Global Variables
//cnt,avg, and sum are for calculating the average
//timocc times how long a space has been occupied, timemp times how long a space has been empty
//poscnt ticks up for a positive reading, ticks down for negative reading (eliminates false positives)
int cnt, avg, sum, occ, timocc, timemp, poscnt;

bool occupied; //Vacant is False(0), Occupied is True(1)

void setup() {
  Serial.begin(9600); //baud rate for serial port
  Wire.begin();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x0B); //QMC5883 to Continuously Measure
  Wire.write(0x01); // Set the Register
  Wire.endTransmission();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x09); //Define OSR = 512, Full Scale Range = 8 Gauss, ODR = 200Hz
  Wire.write(0x1D); // Set the Register
  Wire.endTransmission();
}

void loop() {

  short int x, y, z; //triple axis data
  short int t; //sum value of xyz

  //Tell the QMC5883 what register to begin writing data into
  Wire.beginTransmission(addr);
  Wire.write(0x00); //xyz data is read from 0x00 to 0x05
  Wire.endTransmission();

  //Read the data.. 2 bytes for each axis.. 6 total bytes
  Wire.requestFrom(addr, 6);
  if (6 <= Wire.available()) {
    x = (Wire.read() | (Wire.read()<<8));
    y = (Wire.read() | (Wire.read()<<8));
    z = (Wire.read() | (Wire.read()<<8));
  }
  
  t=x+y+z;

  //count up to x(5) value sample pool
  if(cnt<=10){
    cnt++;
  }
  //update sum value for future avg calculation
  sum += t;
  //when count reaches 100, get avg
  if(cnt==10){
    avg = sum/cnt;
  }
  //after average is calculated, be prepared to compare t value to avg
  if(cnt==11){
    //if t is under avg-offset(500), poscnt up
    if(t<avg-500){
      poscnt++;
      //when count is high enough change boolean to true and reset count and timers
      if(poscnt>=5){
        occupied=true;
        poscnt=5;
        timemp=0;
        timocc++;
      }
    }
    //if t is over avg+offset(500), poscnt up
    if(t>avg+500){
      poscnt++;
      //when count is high enough change boolean to true and reset count and timers
      if(poscnt>=5){
        occupied=true;
        poscnt=5;
        timemp=0;
        timocc++;
      }
    }
    //if t value is within range of avg(+-200), poscnt down
    if(avg-200<t && t<avg+200){
      poscnt--;
      //when poscnt is 0, occupied is false and reset poscnt and timers
      if(poscnt<=0){
        occupied=false;
        poscnt=0;
        timocc=0;
        timemp++;
      }
    }
  }

  //print values
  Serial.print("Real Time Value: ");
  Serial.print(t);
  Serial.println();

  Serial.print("Calibrated Value: ");
  Serial.print(avg);
  Serial.println();

  Serial.print("Occupied Count: ");
  Serial.print(timocc);
  Serial.println();

  Serial.print("Empty Count: ");
  Serial.print(timemp);
  Serial.println();

  Serial.print("Occupied: ");
  Serial.print(occupied);
  Serial.println();

  
  
  delay(1000);
}
