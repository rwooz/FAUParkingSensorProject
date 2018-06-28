/*
LoRa test
*/

#include <MKRWAN.h>

LoRaModem modem;

//Obtained through thethingsnetwork
String appEui = "70B3D57ED0010130";
String appKey = "6F5FA5D39E67EEF7FD874D4DAA25F6FD";

void setup() {
  Serial.begin(115200);
  while (!Serial);
  if (!modem.begin(US915)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  int connected;
  appKey.trim();
  appEui.trim();
  connected = modem.joinOTAA(appEui, appKey); 
  
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  delay(5000);

  int err;
  modem.setPort(3);
  modem.beginPacket();
  modem.print("HeLoRA world!"); //test message sent to thethingsnetwork
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
  }
}

void loop() {
  while (modem.available()) {
    Serial.write(modem.read());
  }
  modem.poll();
}
