#include <AltSoftSerial.h>

AltSoftSerial obd;
String message;
unsigned long prevTime1 = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;
unsigned long prevTime4 = 0;
unsigned long prevTime5 = 0;
unsigned long prevTime6 = 0;
unsigned long prevTime7 = 0;
bool done1 = false;
bool done2 = false;
bool done3 = false;
bool done4 = false;
bool done5 = false;
bool done6 = false;
const int POLL_1    = 0;
const int PROCESS_1 = 80;
const int POLL_2    = 100;
const int PROCESS_2 = 180;
const int POLL_3    = 200;
const int PROCESS_3 = 280;
const int RESET     = 300;

void getData(int& MAP, int& ABP, int& LOAD) {
  unsigned long currTime = millis();
  
  if ((currTime - prevTime1 >= POLL_1) && !done1) { //request 01 0B
    done1 = true;
    obd.println("010B");
  }

  if ((currTime - prevTime2 >= PROCESS_1) && !done2) { //process
    done2 = true;
    message = obd.readString();
    
    if (message[1] == '>') {
      message.remove(1, 1);
      //Serial.println("FOUND");
    }
    Serial.println(message);
      
    if (message == "CAN ERROR\r\r>")
      MAP = -1;
    else if (message == "NO DATA\r\r>")
      MAP = -2;
    else if (message == "STOPPED\r\r>")
      MAP = -3;
    else if (message[0] != '4') //unknown error. msg does not start with normal response
      MAP = -4;
    else { //no errors
      message.remove(0, 4);
      message.remove(3);
      MAP = strtol(message.c_str(), NULL, 16);
    }
  }

//----------------------------------------------------------------------------------
  if ((currTime - prevTime3 >= POLL_2) && !done3) { //request 01 33
    done3 = true;
    obd.println("0133");
  }

  if ((currTime - prevTime4 >= PROCESS_2) && !done4) { //process
    done4 = true;
    message = obd.readString();
    
    if (message[1] == '>') {
      message.remove(1, 1);
      //Serial.println("FOUND");
    }
    Serial.println(message);
    
    if (message == "CAN ERROR\r\r>")
      ABP = -1;
    else if (message == "NO DATA\r\r>")
      ABP = -2;
    else if (message == "STOPPED\r\r>")
      ABP = -3;
    else if (message[0] != '4') //unknown error. msg does not start with normal response
      ABP = -4;
    else { //no errors
      message.remove(0, 4);
      message.remove(3);
      ABP = strtol(message.c_str(), NULL, 16);
    }
  }
//----------------------------------------------------------------------------------
  if ((currTime - prevTime5 >= POLL_3) && !done5) { //request 01 04
    done5 = true;
    obd.println("0104");
  }

  if ((currTime - prevTime6 >= PROCESS_3) && !done6) { //process
    done6 = true;
    message = obd.readString();
    
    if (message[1] == '>') {
      message.remove(1, 1);
      //Serial.println("FOUND");
    }
    Serial.println(message);
    
    if (message == "CAN ERROR\r\r>")
      LOAD = -1;
    else if (message == "NO DATA\r\r>")
      LOAD = -2;
    else if (message == "STOPPED\r\r>")
      LOAD = -3;
    else if (message[0] != '4') //unknown error. msg does not start with normal response
      LOAD = -4;
    else { //no errors
      message.remove(0, 4);
      message.remove(3);
      LOAD = strtol(message.c_str(), NULL, 16);
    }
  }

  
  if (currTime - prevTime7 >= RESET) {
    prevTime1 = currTime;
    prevTime2 = currTime;
    prevTime3 = currTime;
    prevTime4 = currTime;
    prevTime5 = currTime;
    prevTime6 = currTime;
    prevTime7 = currTime;
    done1 = false;
    done2 = false;
    done3 = false;
    done4 = false;
    done5 = false;
    done6 = false;
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
  getData(MAP, ABP, LOAD);
  if (MAP == -1 || MAP == -2 || MAP == -3 || MAP == -4 || ABP == -1 || ABP == -2 || ABP == -3 || ABP == -4 || LOAD == -1 || LOAD == -2 || LOAD == -3 || LOAD == -4)
  {
    Serial.println("ERROR");
  } else {
    Serial.print("BOOST: ");
    Serial.println(MAP-ABP);
    Serial.print("LOAD: ");
    Serial.println(LOAD);
  }
  //Serial.println(MAP-ABP);
  //Serial.print("MAP: ");
  //Serial.println(MAP);
  //Serial.print("ABP: ");
  //Serial.println(ABP);
  }
