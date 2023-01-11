#include <OBD2UART.h>
#include <FastLED.h>

COBD obd;

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
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  brightness = map(analogRead(A0), 0, 512, 1, 15); //get current brightness for temp LED only
  
  FastLED.setBrightness(5); //welcome brightness (5)

  analogWrite(6, brightness);
  analogWrite(5, 0);
  
  for (int i = 15; i >= 0; i--) //welcome sequence
  {
    leds[i] = CRGB::OrangeRed;
    FastLED.show();
    delay((i*4));
  }
  delay(200);

  analogWrite(5, brightness);
  analogWrite(6, 0);
  
  for (int i = 0; i < 16; i++) //clear out
  {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(5);
  }

  // start communication with OBD-II UART adapter
  obd.begin();
  // initiate OBD-II connection until success
  while (!obd.init());  

  obd.readPID(PID_BAROMETRIC, ABP); //get initial ABP

}

void loop() {
  unsigned long currTime = millis();
  
  if (1) { 
    val = MAP - ABP;
      
    if (val > 0) { //positive pressure
      valScaled = map(val, 0, 70, 0, 15);
    } else
      valScaled = map(val, -10, 0, -8, 0);
    

    if (counter >= abs(valScaled-15)) { //go up (down) counter init 15
      if (counter > 14)
      {
        leds[counter-15] = CRGB::Black;
      } else
        leds[counter] = POS_COLOR;
      counter--;
    }
    
    if (counter < abs(valScaled-15)) { //go down (up)
      if (counter > 14)
      {
        leds[counter-15] = NEG_COLOR;
      } else
        leds[counter] = CRGB::Black;
      counter++;
    }

    // LOAD AREA
        if (LOAD >= 90 && LOAD < 135) {
            leds[CENTER_INDEX] = CRGB::Green;
        } else if (LOAD >= 135 && LOAD < 180) {
            leds[CENTER_INDEX] = CRGB::Yellow;
        } else if (LOAD >= 180)
            leds[CENTER_INDEX] = CRGB::Red; 
        else
            leds[CENTER_INDEX] = CENTER_COLOR;

    FastLED.show();

    // TEMPERATURE AREA
    //temperature selection is here so that when the engine turns on it immediately has a color instead of waiting 10 seconds.
    //this will constantly update temp_color, but TEMP is acquired every 10 seconds. LED is updated according to temp_color every second.

    //TEMP = 75;

    if (TEMP >= 60 && TEMP < 74) {
        temp_color = 1;
      } else if (TEMP >= 74) {
        temp_color = 2;
      } else {
        temp_color = 0;
      }

  
  if ((currTime - prevTime >= 50)) { //poll data every 50 ms and store
    obd.readPID(PID_INTAKE_MAP, MAP);
    obd.readPID(PID_ENGINE_LOAD, LOAD);

    prevTime = currTime;
  }

  if (currTime - prevTime2 >= 1000) //poll LDR and update brightness as well as temp LED
  {
    brightness = map(analogRead(A0), 0, 512, 1, 15);
    FastLED.setBrightness(brightness);
    lastBrightness = brightness;

    //set temperature LED
    if (temp_color == 1) { //OK - GREEN
      analogWrite(6, 0);
      analogWrite(5, brightness);
    }
    if (temp_color == 2) { //MIDPOINT - OFF
      analogWrite(5, 0);
      analogWrite(6, 0);
    }
    if (temp_color == 0) { //COLD - BLUE
      analogWrite(6, brightness);
      analogWrite(5, 0);
    }
    
    prevTime2 = currTime;
  }

  if (currTime - prevTime4 >= 10E3) //poll TEMP
  {
    obd.readPID(PID_COOLANT_TEMP, TEMP);

    prevTime4 = currTime;
  }

  if (currTime - prevTime3 >= 60E3) //poll ABP
  {
    obd.readPID(PID_BAROMETRIC, ABP);

    prevTime3 = currTime;
  }
}
}