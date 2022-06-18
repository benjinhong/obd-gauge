#include <AltSoftSerial.h>
//works with one PID
AltSoftSerial obd;
String message;

unsigned long prevTime1 = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;
bool done1 = false;
bool done2 = false;
bool done3 = false;

void getData(int& data) {
  unsigned long currTime = millis();
  
  if ((currTime - prevTime1 >= 0) && !done1) { //request 01 0B
    done1 = true;
    obd.println("010B");
  }

  if ((currTime - prevTime2 >= 50) && !done2) { //process
    done2 = true;
    message = obd.readString();
    Serial.println(message);
    if (message == "CAN ERROR\r\r>")
      data = -1;
    else if (message == "NO DATA\r\r>")
      data = -2;
    else if (message == "STOPPED\r\r>")
      data = -3;
    else if (message[0] != '4') //unknown error. msg does not start with normal response
      data = -4;
    else { //no errors
      message.remove(0, 4);
      message.remove(3);
      data = strtol(message.c_str(), NULL, 16);
    }
  }

  if (currTime - prevTime3 >= 60) {
    prevTime1 = currTime;
    prevTime2 = currTime;
    prevTime3 = currTime;
    done1 = false;
    done2 = false;
  }
}

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
  delay(500);
}

void loop() { //MAP - BP
  int data;
  getData(data);
  Serial.println(data);
}
