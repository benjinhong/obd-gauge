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

int DEBUG_MODE = 0;
// 0 = off
// 1 = show all
// 2 = suppress CAN data

CRGB leds[NUM_LEDS];

//misc
int counter = 0;
int val;
int valScaled = 0;
int valScaledPre = 0;
int lastValScaledPre = 0;

int MAP = 0;
int ABP = 0;
int LOAD = 0;
int TEMP = 0;
int ERR = 0;
int ERR_LAST = 0;
int PID_SEL = 1;
int lastBrightness = 0;
int brightness = 1;
int temp_color = 0; //0 blue, 1 green, 2 off

bool lastFlag1 = true;

unsigned long prevTime = 0;
unsigned long prevTime2 = 0;
unsigned long prevTime3 = 0;
unsigned long prevTime4 = 0;

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
    delay((i*4));
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
    if (ERR != 0) {
        val = 0;
        TEMP = 74; //MIDPOINT - OFF. forces temp LED to turn off when car is stopped from driving or in park.
    }
      
    
    //Serial.println(val);
    
    if (val > 0) { //positive pressure
      valScaled = map(val, 0, 70, 0, 15);
    } else
      valScaled = map(val, -10, 0, -8, 0);
    //if (ERR != 0)
    //  valScaled = 0;

      //Serial.print("valScaled:" );
      //Serial.println(valScaled);
      //Serial.print("counter:" );
      //Serial.println(counter);

    /*if ( (abs(valScaledPre - lastValScaledPre) > 1) ) { //check delta only in negative pressure zone
      Serial.println("boost delta > 1");
      valScaled = valScaledPre;

      lastValScaledPre = valScaledPre;
    } 
    else if (valScaledPre >= 0)
    {
      Serial.println("POSITIVE PRESSURE");
      valScaled = valScaledPre;
    }*/
      

    
      
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

    ///TEMPERATURE AREA
    //temperature selection is here so that when the engine turns on it immediately has a color instead of waiting 10 seconds.
    //this will constantly update temp_color, but TEMP is acquired every 10 seconds. LED is updated according to temp_color every second.

    //TEMP = 59;

    if (TEMP >= 60 && TEMP < 74) {
        if (DEBUG_MODE)
          Serial.println("OK - GREEN");
        temp_color = 1;
      } else if (TEMP >= 74) {
        if (DEBUG_MODE)
          Serial.println("MIDPOINT - OFF");
        temp_color = 2;
      } else {
        if (DEBUG_MODE)
          Serial.println("COLD - BLUE");
        temp_color = 0;
      }

    //ERROR AREA
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
    brightness = map(analogRead(A0), 0, 512, 1, 15);

    /*if (DEBUG_MODE == 1 || DEBUG_MODE == 2)
    {
      Serial.print("BRIGHTNESS: ");
      Serial.println(brightness);
    }*/

    //if (abs(brightness - lastBrightness) > 5)   //20 max probably?
    //{
      if (DEBUG_MODE == 1 || DEBUG_MODE == 2)
        Serial.println("BRIGHTNESS UPDATE");
    FastLED.setBrightness(brightness);
    //Serial.println(brightness);
    lastBrightness = brightness;
    //}

    //set temperature LED
    if (temp_color == 1) { //OK - GREEN
      analogWrite(5, 0);
      analogWrite(6, brightness);
    }
    if (temp_color == 2) { //MIDPOINT - OFF
      analogWrite(5, 0);
      analogWrite(6, 0);
    }
    if (temp_color == 0) { //COLD - BLUE
      analogWrite(5, brightness);
      analogWrite(6, 0);
    }
    
    prevTime2 = currTime;
  }

  if (currTime - prevTime4 >= 10E3) //poll TEMP
  {
    if (DEBUG_MODE == 1 || DEBUG_MODE == 2)
      Serial.println("POLL TEMP");
    PID_SEL = 2;
  

    prevTime4 = currTime;
  }

  if (currTime - prevTime3 >= 60E3) //poll ABP
  {
    if (DEBUG_MODE == 1 || DEBUG_MODE == 2)
      Serial.println("POLL ABP");
    PID_SEL = 1;
    prevTime3 = currTime;
  }

  delay(17); 
  /*
   * poll 50 delay 18 good, 17 working good longest. 16 trial
   */
}

int getData() { //whatever that is in the character array is still represented in hex.
  char response[32] = {};
  char MAParr[3] = {};
  char AUXarr[3] = {};
  if (PID_SEL == 1) //1 = poll ABP + MAP
  {
    obd.println("010B331"); //MAP + ABP //01 0B 33 1 //PERIODIC ABP REQUEST
    if (DEBUG_MODE == 1 || DEBUG_MODE == 2)
      Serial.println("--------Requesting MAP+ABP---------");
    PID_SEL = 2; //request TEMP right after ABP request at power on
  } 
  else if (PID_SEL == 2) 
  {
    obd.println("010B051");
    if (DEBUG_MODE == 1 || DEBUG_MODE == 2) {
      //if (lastFlag1) {
        Serial.println("--------Requesting MAP+TEMP--------");
        Serial.println(TEMP);
        //lastFlag1 = false;
     // }
    } 
    PID_SEL = 1; //flip back to ABP 
  } 
  else
    obd.println("010B041"); //MAP ONLY is 010B1. currently polling MAP + LOAD //NORMAL REQUEST

  /*
  The PID_SEL flipping only happens at the idle STOPPED state. Once flow passes through OK state,
  PID_SEL gets set to 0 to call normal request. If timer requests ABP, PID_SEL = 2 does get set
  to 2, but because we are still in OK state, PID_SEL gets set back to 0, overriding 2. Same
  thing if timer requests TEMP. 
  */
  
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

   lastFlag1 = true; //reset flag for serial debugging in STOPPED mode.

    MAParr[0] = response[4]; //4
    MAParr[1] = response[5]; //5
  
    AUXarr[0] = response[8]; //8
    AUXarr[1] = response[9]; //9

    MAP = strtol(&MAParr[0], NULL, 16);
    
    //worked in theory but not in practice. turns out response is one cycle late. 
    /*if (PID_SEL == 1) { //ABP
      Serial.println("Setting ABP to LOAD (ABP requested)");
      ABP = LOAD;
      PID_SEL = 0;
    }*/
    //this will wait until it sees 33. if it does, parse as expected.
    if (response[6] == '3') {
      ABP = strtol(&AUXarr[0], NULL, 16);
    } else if (response[7] == '5') { //if it sees 5 in "05" (coolant temp)
      TEMP = strtol(&AUXarr[0], NULL, 16) - 40; //-40 for zero offset
    } else {
      LOAD = strtol(&AUXarr[0], NULL, 16);
    }


    PID_SEL = 0;
    
    //print array contents
    //41 0B 64 normal response
    if (DEBUG_MODE == 1) {
      for (int i = 0; i < index; i++)  {
          Serial.print(response[i]);
        }
        //Serial.println();
          Serial.print("  TURBO: ");
          Serial.print(val);
          Serial.print("  LOAD: ");
          Serial.print(LOAD);
          Serial.print("  ABP: ");
          Serial.print(ABP);    

         Serial.print("  TEMP: ");
         Serial.print(TEMP); 
         Serial.print(" C");   
         Serial.println();
    }
    
  }
}
