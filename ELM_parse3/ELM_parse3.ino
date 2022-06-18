#include <AltSoftSerial.h>
#include <FastLED.h>

//Serial stuff
AltSoftSerial obd;

//LED stuff
#define NUM_LEDS 256
#define DATA_PIN 3
CRGB leds[NUM_LEDS];

//misc
int counter = 0;
int val;
int lastVal;

int MAP = 0;
int ABP = 0;
int LOAD = 0;
int ERR = 0;

unsigned long prevTime = 0;

void setup() {
  Serial.begin(38400);
  obd.begin(38400);
  obd.setTimeout(1);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
  FastLED.setBrightness(10);
  
    
  delay(1E3);
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
  val = (MAP - ABP) + 10;
  
  

  if (counter < val) { //constantly update counter
    leds[counter] = CRGB::Red;
    counter++;
    }
  if (counter > val) {
    leds[counter-1] = CRGB::Black;
    counter--;
    }
  FastLED.show();
  Serial.println(counter);
  if (currTime - prevTime >= 50) { //poll and process every 50 ms
    getData();
    prevTime = currTime;
  }
  delay(18);
}


int getData() { //whatever that is in the character array is still represented in hex.
  char response[32] = {};
  char MAParr[3] = {};
  char ABParr[3] = {};
  obd.println("010B33041"); //request MAP
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
  /*if (response[0] == 'C')
  {
    Serial.println("CAN ERROR");
  }*/

  //print array contents
  //41 0B 64 normal response
  for (int i = 0; i < index; i++)  {
    Serial.print(response[i]);
  }
  Serial.println();
  
  MAParr[0] = response[4]; //4
  MAParr[1] = response[5]; //5
  
  ABParr[0] = response[8]; //8
  ABParr[1] = response[9]; //9
  
  MAP = strtol(&MAParr[0], NULL, 16);
  ABP = strtol(&ABParr[0], NULL, 16);
  
  Serial.print("MAP: ");
  Serial.println(MAP);
  Serial.print("ABP: ");
  Serial.println(ABP);
  
}
