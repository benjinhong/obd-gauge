#include <AltSoftSerial.h>

AltSoftSerial obd;
String message1;
String message2;

unsigned long prevTime1 = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;
unsigned long prevTime4 = 0;
unsigned long prevTime5 = 0;
unsigned long prevTime6 = 0;
bool done1 = false;
bool done2 = false;
bool done3 = false;
bool done4 = false;
bool done5 = false;
bool done6 = false;

void getData(int& MAP, int& ABP) {
  unsigned long currTime = millis();
  
  if ((currTime - prevTime1 >= 0) && !done1) { //request 01 0B
    done1 = true;
    obd.println("010B");
  }

  if ((currTime - prevTime2 >= 50) && !done2) { //process
    done2 = true;
    message1 = obd.readString();
    //Serial.println(message1);
    if (message1 == "CAN ERROR\r\r>")
      MAP = -1;
    else if (message1 == "NO DATA\r\r>")
      MAP = -2;
    else if (message1 == "STOPPED\r\r>")
      MAP = -3;
    else if (message1[0] != '4') //unknown error. msg does not start with normal response
      MAP = -4;
    else { //no errors
      message1.remove(0, 4);
      message1.remove(3);
      MAP = strtol(message1.c_str(), NULL, 16);
    }
  }

//----------------------------------------------------------------------------------
  if ((currTime - prevTime3 >= 100) && !done3) { //request 01 0B
    done3 = true;
    obd.println("0133");
  }

  if ((currTime - prevTime4 >= 150) && !done4) { //process
    done4 = true;
    message1 = obd.readString();
    //Serial.println(message2);
    if (message1 == "CAN ERROR\r\r>")
      ABP = -1;
    else if (message1 == "NO DATA\r\r>")
      ABP = -2;
    else if (message1 == "STOPPED\r\r>")
      ABP = -3;
    else if (message1[0] != '4') //unknown error. msg does not start with normal response
      ABP = -4;
    else { //no errors
      message1.remove(0, 4);
      message1.remove(3);
      ABP = strtol(message1.c_str(), NULL, 16);
    }
  }



  
  if (currTime - prevTime5 >= 200) {
    prevTime1 = currTime;
    prevTime2 = currTime;
    prevTime3 = currTime;
    prevTime4 = currTime;
    prevTime5 = currTime;
    done1 = false;
    done2 = false;
    done3 = false;
    done4 = false;
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
  int MAP, ABP, LOAD;
  getData(MAP, ABP);
  if (MAP == -1 || MAP == -2 || MAP == -3 || MAP == -4 || ABP == -1 || ABP == -2 || ABP == -3 || ABP == -4)
  {
    Serial.println("ERROR");
  } else {
    Serial.println(MAP-ABP);
  }
  //Serial.println(MAP-ABP);
  //Serial.print("MAP: ");
  //Serial.println(MAP);
  //Serial.print("ABP: ");
  //Serial.println(ABP);
  }
