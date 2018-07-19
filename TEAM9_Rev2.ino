#include <Wire.h> //I2C Arduino Library
#include <MKRWAN.h>

#define addr 0x0D //I2C Address for The QMC5883

//Global Variables
//cnt,avg, and sum are for calculating the average
//timocc times how long a space has been occupied, timemp times how long a space has been empty
//poscnt ticks up for a positive reading, ticks down for negative reading (eliminates false positives)
int cnt, avg, sum, occ, timocc, timemp, poscnt;
int err;

bool occupied; //Vacant is False(0), Occupied is True(1)

LoRaModem modem;
String appEui = "70B3D57ED001063A";
String appKey = "E0E6247573FFFA880CCA5CE935D2B2CD";

void setup() {
  Serial.begin(115200); //baud rate for serial port
  Wire.begin();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x0B); //QMC5883 to Continuously Measure
  Wire.write(0x01); // Set the Register
  Wire.endTransmission();
  Wire.beginTransmission(addr); //start talking
  Wire.write(0x09); //Define OSR = 512, Full Scale Range = 8 Gauss, ODR = 200Hz
  Wire.write(0x1D); // Set the Register
  Wire.endTransmission();

  //Serial.begin(115200);
  while (!Serial);
  if (!modem.begin(US915)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  
  int connected;
  connected = modem.joinOTAA(appEui, appKey); 
  
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");;
    while(1){}
  }

  Serial.println("Connected!");
  
  delay(20000);//Delay from connecting to sending a msg

  modem.beginPacket();
        modem.write(0x00); //converted to false by ttn payload
        err = modem.endPacket(true);
        if (err > 0) {
          Serial.println("Occupied is false.");
          } else {
            Serial.println("Error sending message :(");
            }
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
      if(occupied == false){
        modem.beginPacket();
        modem.write(0x01); //converted to true by ttn payload
        err = modem.endPacket(true);
        if (err > 0){
          Serial.println("Message sent correctly!");
          } else {
            Serial.println("Error sending message :(");
          }
        }
        occupied=true;
        if(poscnt>=5){
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
        if(occupied == false){
        modem.beginPacket();
        //modem.print("True");
        modem.write(0x01);
        err = modem.endPacket(true);
        if (err > 0){
          Serial.println("occupied is true.");
          } else {
            Serial.println("Error sending message :(");
          }
        }
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
        if(occupied == true){
        modem.beginPacket();
        modem.write(0x00); //converted to false by ttn payload
        err = modem.endPacket(true);
        if (err > 0){
          Serial.println("Message sent correctly!");
          } else {
            Serial.println("Error sending message :(");
          }
        }
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

  Serial.print("Occupied: ");
  Serial.print(occupied);
  Serial.println();

  
  
  delay(1000);
}
