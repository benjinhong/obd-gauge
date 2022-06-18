#include <AltSoftSerial.h>

AltSoftSerial obd;
String message;

void setup() {
  Serial.begin(38400);
  obd.begin(38400);
  obd.setTimeout(10);
  delay(1E3);
  obd.println("ATSP6"); //set protocol 6
  delay(100);
  obd.println("ATAL"); //allow >7 byte msg
  delay(100);
  obd.println("ATE0"); //turn off echo
  delay(100);
  obd.println("ATS0"); //turn off spaces
  delay(100);
  obd.readString(); //clear
}

void loop() {
  int value;
  obd.println("010B");
  if (obd.available())
  {
    message = obd.readString();
    Serial.println(message);
    message.remove(0, 4);
    message.remove(3);
    value = strtol(message.c_str(), NULL, 16);
    Serial.println(value);
  }
  
  /*obd.println("0133");
  delay(50);
  if (obd.available())
  {
    message = obd.readString();
    Serial.println(message);
    message.remove(0, 4);
    message.remove(3);
    value = strtol(message.c_str(), NULL, 16);
    Serial.println(value);
  }*/

  delay(50);
}
