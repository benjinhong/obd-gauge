#include <AltSoftSerial.h>
//works with one PID
AltSoftSerial obd;

unsigned long prevTime1 = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;
bool done1 = false;
bool done2 = false;
bool done3 = false;
int MAP = 0;
int ABP = 0;
int LOAD = 0;
int ERR = 0;
const int PROCESS_MS = 60; //if you see STOPPED, you are reqeusting too fast.
const int RESET_MS = 70;

void getData() {
  unsigned long currTime = millis();
  String response;
  String MAPr;
  String ABPr;
  String LOADr;
  
  if ((currTime - prevTime1 >= 0) && !done1) { //multi-PID request: 0B 33 04
    done1 = true;
    obd.println("010B3304");
  }

  if ((currTime - prevTime2 >= PROCESS_MS) && !done2) { //process
    done2 = true;
    response = obd.readString();
    if (response[1] == '>') {
      response.remove(1, 1);
      //Serial.print("CORRECTED: ");
      //Serial.println(response);
    }
    
    //Serial.print("LOOKING: ");
    //Serial.println(response[0]);
    
    if (response[0] == '\r') {
      response.remove(0, 1);
      //Serial.print("CORRECTED: ");
      //Serial.println(response);
    }
    
    //Serial.print("LOOKING: ");
    //Serial.println(response[0]);
    
    if (response[0] == 'C') {
      ERR = -1;
      MAP = 0;
      ABP = 0;
      LOAD = 0;
    }
    else if (response[0] == 'N') {
      ERR = -2;
      MAP = 0;
      ABP = 0;
      LOAD = 0;
    }
    else if (response[0] == 'S') {
      ERR = -3;
      MAP = 0;
      ABP = 0;
      LOAD = 0;
    }
    else if (response[0] != '4') //unknown error. msg does not start with normal response
      ERR = -4;
    else { //no errors
      ERR = 0;
      MAPr = response;
      MAPr.remove(0, 4); //remove first 4
      MAPr.remove(2); //remove everything after value
      MAP = strtol(MAPr.c_str(), NULL, 16);
      
      ABPr = response;
      ABPr.remove(0, 8); //remove first 8
      ABPr.remove(2); //remove everything after value
      ABP = strtol(ABPr.c_str(), NULL, 16);

      LOADr = response;
      LOADr.remove(0, 12); //remove first 12
      LOADr.remove(2); //remove everything after value
      LOAD = strtol(LOADr.c_str(), NULL, 16);
    }
  }

  if (currTime - prevTime3 >= RESET_MS) {
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
  getData();
  Serial.print("MAP: ");
  Serial.println(MAP);
  Serial.print("ABP: ");
  Serial.println(ABP);
  Serial.print("TURBO: ");
  Serial.println(MAP - ABP);
  Serial.print("LOAD: ");
  Serial.println(LOAD);
  Serial.print("ERR: ");
  switch(ERR)
  {
    case -1: 
      Serial.println("CAN ERROR");
      break;
    case -2:
      Serial.println("NO DATA");
      break;
    case -3:
      Serial.println("STOPPED");
      break;
    case -4:
      Serial.println("BAD FORMAT");
      break;
    case 0:
      Serial.println("OK");
      break;
  }
}
