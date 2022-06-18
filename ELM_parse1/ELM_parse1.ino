#include <AltSoftSerial.h>

AltSoftSerial obd;

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
  delay(100);
}

void loop() {
  getMAP();
  //readIn();  
  delay(10);
}


int getMAP() { //whatever that is in the character array is still represented in hex.
  char response[32] = {};
  obd.println("010B"); //request MAP
  //delay(50); //wait for buffer to fill
  int index = 0; //reset array index
  int incoming = 0; //set ic to zero
  //reading takes 440us (0.4ms)
  while (incoming != -1) //reads in all the way until CR
  {
    incoming = obd.read(); //read in character (as integer) and destroy
    if (incoming != -1) //ignores -1
    {
      response[index] = incoming;
      index++;
    }
  }
  /*for (int i = 0; i < index; i++)  {
    Serial.print(response[i]); //"STOPPED>" for stopped
  } //41 0B 64 NORMAL 
  //Serial.print(response[4]);
  //Serial.println(response[5]);
  //Serial.println(response[4], HEX);
  int value = (int)strtol(&(response[4]), NULL, 16);
  Serial.println(value);*/
  
}

int readIn()
{
  obd.println("010B");
  while(true)
  {
    Serial.println(obd.read());
  }
  return 0;
}
