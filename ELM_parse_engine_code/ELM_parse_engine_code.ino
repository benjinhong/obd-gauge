#include <AltSoftSerial.h>
#include <FastLED.h>

//Serial stuff
AltSoftSerial obd;

//LED stuff
#define NUM_LEDS 256
#define DATA_PIN 3
#define POS_COLOR CRGB::DarkRed
#define NEG_COLOR CRGB::Cyan
#define CENTER_COLOR CRGB::White
#define CENTER_INDEX 15

CRGB leds[NUM_LEDS];

//misc
int counter = 0;
int val;
int valScaled = 0;

int MAP = 0;
int ABP = 0;
int LOAD = 0;
int ERR = 0;
int PID_SEL = 1;
int lastBrightness = 0;

unsigned long prevTime = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;

void setup() {
  Serial.begin(38400);
  obd.begin(38400);
  obd.setTimeout(1);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  
  FastLED.setBrightness(5); //initial brightness
  
  for (int i = 15; i >= 0; i--) //welcome sequence
  {
    leds[i] = CRGB::OrangeRed;
    FastLED.show();
    delay((i*3));
  }
  delay(200);
  
  for (int i = 0; i < 16; i++)
  {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(5);
  }
  
  delay(100); //1E3 worked
  //obd.println("ATZ"); //set protocol 6
  delay(100);
  obd.println("ATSP6"); //set protocol 6
  delay(100);
  obd.println("ATAL"); //allow >7 byte msg
  delay(100);
  obd.println("ATE0"); //turn off echo
  delay(100);
  obd.println("ATS0"); //turn off spaces
  delay(100);
  obd.println("ATL0"); //turn off spaces
  delay(100);
  obd.readString(); //clear
  delay(100);
}

void loop() {
  unsigned long currTime = millis();
  
  if (1) { //ERR == 0

    //Serial.println(counter);
  } if (ERR == -1) { //CAN ERROR
    leds[CENTER_INDEX] = CRGB::Purple;
    FastLED.show();
  } else if (ERR == -2) { //NO DATA
    leds[CENTER_INDEX] = CRGB::Pink;
    FastLED.show();
  } else if (ERR == -3) { //STOPPED (engine off, not start/stop)
    leds[CENTER_INDEX] = CRGB::Green;
    FastLED.show();
  } else if (ERR == -4) { //BAD FRAME
    leds[CENTER_INDEX] = CRGB::Magenta;
    FastLED.show();
  }
  
  if ((currTime - prevTime >= 1000)) { //poll and process every 50 ms
    getData();
    prevTime = currTime;
  }

  if (currTime - prevTime2 >= 2000) //poll LDR and update brightness
  {
    
    prevTime2 = currTime;
  }

  if (currTime - prevTime3 >= 60E3) //poll ABP
  {
   
    prevTime3 = currTime;
  }

  delay(17);
  /*
   * poll 50 delay 18 good
   */
}



int getData() { //whatever that is in the character array is still represented in hex.
  char response[32] = {};

    obd.println("031"); //MAP + ABP
    //Serial.println("Requesting MAP+ABP");
  
  
    //obd.println(""); //MAP + LOAD 010B041
  
  //delay(50); //wait for buffer to fill
  int index = 0; //reset array index
  int incoming = 0; //set ic to zero
  
  while (incoming != -1) //reads in all the way until CR
  {
    incoming = obd.read(); //read in character (as integer) and destroy
    if (incoming != -1) //ignores -1
    {
      response[index] = incoming;
      index++;
    }
  }
  
  //error checking
  if (response[0] == 'C') { //CAN ERROR
    //Serial.println("CAN ERROR");
    ERR = -1;
  } else if (response[0] == 'N') { //NO DATA
    //Serial.println("NO DATA");
    ERR = -2;
  } else if (response[0] == 'S') { //STOPPED
    //Serial.println("STOPPED");
    ERR = -3;
  } else if (response[0] != '4') { //BAD FRAME
    //Serial.println("BAD FRAME");
    ERR = -4;
  } else {
    ERR = 0;
    
    //worked in theory but not in practice. turns out response is one cycle late. 
    /*if (PID_SEL == 1) { //ABP
      Serial.println("Setting ABP to LOAD (ABP requested)");
      ABP = LOAD;
      PID_SEL = 0;
    }*/
    //this will wait until it sees 33. if it does, parse as expected.
    /*if (response[6] == '3')
    {
      ABP = strtol(&LOADarr[0], NULL, 16);
    } else {
      LOAD = strtol(&LOADarr[0], NULL, 16);
    }*/
    
    //print array contents
    //41 0B 64 normal response

    /*
    If you want to read the ready-to-receive character ">" then look for 0x3E (ASCII).
    */

    for (int i = 2; i < index; i++)  {
      if (response[i] != 0x3E)
        Serial.print(response[i]);
      
    }
    Serial.println();
  }
}
