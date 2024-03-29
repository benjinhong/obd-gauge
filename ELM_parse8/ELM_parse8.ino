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

bool DEBUG_MODE = false;

CRGB leds[NUM_LEDS];

//misc
int counter = 0;
int val;
int valScaled = 0;

int MAP = 0;
int ABP = 0;
int LOAD = 0;
int ERR = 0;
int ERR_LAST = 0;
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
  
  FastLED.setBrightness(5); //welcome brightness
  
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

  FastLED.setBrightness(1); //initial brightness. assume night
  
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
  obd.readString(); //clear
  delay(100);
}

void loop() {
  unsigned long currTime = millis();
  
  if (1) { //ERR == 0
    //Serial.println("OK");
    //Serial.print("Counter: ");
    //Serial.prfintln(counter);
    val = MAP - ABP;
    if (ERR != 0)
      val = 0;
    
    //Serial.println(val);
    
    if (val > 0) { //positive pressure
      valScaled = map(val, 0, 70, 0, 15);
    } else
      valScaled = map(val, -10, 0, -10, 0);
    //if (ERR != 0)
    //  valScaled = 0;

      //Serial.print("valScaled:" );
      //Serial.println(valScaled);
      //Serial.print("counter:" );
      //Serial.println(counter);
      
    if (counter >= abs(valScaled-15)) { //go up (down) counter init 15
      if (counter > 14)
      {
        //Serial.println("neg");
        leds[counter-15] = CRGB::Black;
      } else
        leds[counter] = POS_COLOR;
      counter--;
    }
    
    if (counter < abs(valScaled-15)) { //go down (up)
      if (counter > 14)
      {
        //Serial.println("neg");
        leds[counter-15] = NEG_COLOR;
      } else
        leds[counter] = CRGB::Black;
      counter++;
    }
    if (ERR == 0) //only set center if no errors. saw flickering in error states
        if (LOAD >= 90 && LOAD < 135) {
            leds[CENTER_INDEX] = CRGB::Green;
        } else if (LOAD >= 135 && LOAD < 180) {
            leds[CENTER_INDEX] = CRGB::Yellow;
        } else if (LOAD >= 180)
            leds[CENTER_INDEX] = CRGB::Red; 
        else
            leds[CENTER_INDEX] = CENTER_COLOR;

    FastLED.show();

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
  
  if ((currTime - prevTime >= 50)) { //poll and process every 50 ms
    getData();
    prevTime = currTime;
  }

  if (currTime - prevTime2 >= 1000) //poll LDR and update brightness
  {
    int brightness = map(analogRead(A0), 0, 512, 1, 20);

    if (DEBUG_MODE)
      Serial.println(brightness);

    if (abs(brightness - lastBrightness) > 15)
    {
      Serial.print("brightness------------------------------");
      Serial.println(brightness);
      FastLED.setBrightness( brightness );
      lastBrightness = brightness;
    }
    
    prevTime2 = currTime;
  }

  if (currTime - prevTime3 >= 60E3) //poll ABP
  {
    //Serial.println("poll ABP---------------------------------");
    PID_SEL = 1;
    prevTime3 = currTime;
  }

  delay(17);
  /*
   * poll 50 delay 18 good
   */
}

int getData() { //whatever that is in the character array is still represented in hex.
  char response[32] = {};
  char MAParr[3] = {};
  char LOADarr[3] = {};
  if (PID_SEL == 1) //1 = poll ABP + MAP
  {
    obd.println("010B331"); //MAP + ABP //01 0B 33 1
    //Serial.println("Requesting MAP+ABP");
  }
  else
    obd.println("010B041"); //MAP ONLY is 010B1. currently polling MAP + LOAD
  
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
    ERR = -1;
    if (ERR_LAST != ERR)
      Serial.println("CAN ERROR");
    ERR_LAST = ERR;
  } else if (response[0] == 'N') { //NO DATA
    ERR = -2;
    if (ERR_LAST != ERR)
      Serial.println("NO DATA");
    ERR_LAST = ERR;
  } else if (response[0] == 'S') { //STOPPED
    ERR = -3;
    if (ERR_LAST != ERR)
      Serial.println("STOPPED");
    ERR_LAST = ERR;
  } else if (response[0] != '4') { //BAD FRAME
    ERR = -4;
    if (ERR_LAST != ERR)
      Serial.println("BAD FRAME");
    ERR_LAST = ERR;
  } else {
    ERR = 0;
    if (ERR_LAST != ERR)
      Serial.println("DATA OK");
    ERR_LAST = ERR;

    MAParr[0] = response[4]; //4
    MAParr[1] = response[5]; //5
  
    LOADarr[0] = response[8]; //8
    LOADarr[1] = response[9]; //9

    MAP = strtol(&MAParr[0], NULL, 16);
    
    //worked in theory but not in practice. turns out response is one cycle late. 
    /*if (PID_SEL == 1) { //ABP
      Serial.println("Setting ABP to LOAD (ABP requested)");
      ABP = LOAD;
      PID_SEL = 0;
    }*/
    //this will wait until it sees 33. if it does, parse as expected.
    if (response[6] == '3')
    {
      ABP = strtol(&LOADarr[0], NULL, 16);
    } else {
      LOAD = strtol(&LOADarr[0], NULL, 16);
    }


    PID_SEL = 0;
    
    //print array contents
    //41 0B 64 normal response
    if (DEBUG_MODE) {
        for (int i = 0; i < index; i++)  {
        Serial.print(response[i]);
      }
        //Serial.println();
        //Serial.print("TURBO: ");
        //Serial.println(val);
        Serial.print("LOAD: ");
        Serial.println(LOAD);
        //Serial.print("ABP: ");
        //Serial.println(ABP);      
    }
    
  }
}
