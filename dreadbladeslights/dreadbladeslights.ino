
#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#define PIN 0
#define MAX_PIXELS 24
#define SKULL_CIRCUMFERENCE 20
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(MAX_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

uint8_t  skull_color   = 0, // Current skull color
        animation = 0,
// "left" is closest to cpu
         leftOff = 7, // Position of spinny eyes
         rightOff = 2,
         pos = 0;
#define MAX_ANIMATIONS 2
// ^ zero based
#define C_LEDS 2
uint8_t  i; // generic index

uint8_t testpos = 0;
uint32_t color  = 0xFF0000; // Start red
uint32_t prevTime;
uint8_t slow_counter = 0;
uint8_t slow_factor = 3;

int32_t hires_pos = 0, // 100x actual pos so we can fake floats
  inertia = 0,
  moment,
  spring_force = 0;


#define system_size 800
#define scale2pixel 100 // scale to 16 pixels (800*2/16)
#define friction  90 // (100-89)/100
#define spring_constant 36 // 36/100 = .36

#define BUTTON_PIN 1
uint8_t button_state = 0;
uint8_t prev_button_state = 0xFF;
uint8_t button_seen_up = 0;
unsigned long button_state_start_time = 0;
#define BUTTON_BOUNCE_TIME 80


const byte angle_to_pixel[] ={
 0 + C_LEDS, 11 + C_LEDS,
 1 + C_LEDS, 12 + C_LEDS,
 2 + C_LEDS, 13 + C_LEDS,
 3 + C_LEDS, 14 + C_LEDS,
 4 + C_LEDS, 15 + C_LEDS,
 5 + C_LEDS, 16 + C_LEDS,
 6 + C_LEDS, 17 + C_LEDS,
 7 + C_LEDS, 18 + C_LEDS,
 8 + C_LEDS, 19 + C_LEDS,
 9 + C_LEDS, 10 + C_LEDS
};


void setup() {
  randomSeed(analogRead(2));
    pinMode(BUTTON_PIN, INPUT);

  #ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  //pixels.setBrightness(255);
  prevTime = millis();
    skull_color=99;
  //SolidRing(0xFF0000);
//  SolidRing(Wheel(random(255)));
}





void loop() {
  boolean mode_change = GetButtonState();

  uint32_t actual_color;

  // up the mopde if button is pressed:
  if (mode_change){
    if (++animation > MAX_ANIMATIONS){
        animation = 0;
        skull_color++;
        if (skull_color>16) {skull_color = 0;}
      }
    }

    actual_color = Wheel(skull_color*16);
  switch(animation) {

    case 0:
      SolidRing(actual_color);
    break;



    case 1000: // Spinny wheels (8 LEDs on at a time)
   // ======================================================
     for(i=0; i<16; i++) {
       uint32_t c = 0; // turn off non-selected pixels
       if(((pos + i) & 7) < 2) c = actual_color; // 4 pixels on...
       pixels.setPixelColor(  NormalizeRingPos(i+leftOff), c); // First eye
       pixels.setPixelColor(16 + NormalizeRingPos(16-i+rightOff)  , c); // Second eye (flipped)
     }
     pixels.show();
     pos = pos++ % 16;
     delay(50);

     break;


   case 99: // Random sparks - just one LED on at a time!
   // ======================================================
    i = random(MAX_PIXELS);
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(10);
    pixels.setPixelColor(i, 0);
    break;



case 2:
  slow_counter++;
  if (slow_counter > slow_factor){
    slow_counter = 0;
    pixels.setPixelColor( angle_to_pixel[pos], 0) ;
    pixels.setPixelColor( angle_to_pixel[pos+(SKULL_CIRCUMFERENCE/2)], 0) ;

    pos++;
    if(pos>=SKULL_CIRCUMFERENCE/2){pos=0;}
    //pixels.setPixelColor( pos, color) ;
    pixels.setPixelColor( angle_to_pixel[pos], actual_color) ;
    pixels.setPixelColor( angle_to_pixel[pos+(SKULL_CIRCUMFERENCE/2)], actual_color) ;
    pixels.show();

  }
    delay(20);
  break;
case 1: // rotating dot
    //pixels.setPixelColor( pos, 0) ;
    pixels.setPixelColor( angle_to_pixel[pos], 0) ;
    pixels.setPixelColor( angle_to_pixel[pos+(SKULL_CIRCUMFERENCE/2)], 0) ;

    pos++;
    if(pos>=SKULL_CIRCUMFERENCE/2){pos=0;}
    //pixels.setPixelColor( pos, color) ;
    pixels.setPixelColor( angle_to_pixel[pos], actual_color) ;
    pixels.setPixelColor( angle_to_pixel[pos+(SKULL_CIRCUMFERENCE/2)], actual_color) ;
    pixels.show();
    delay(20);
break;


      case 102: // sequencer
  // ======================================================
      for(i=0; i<16; i++) {
      uint32_t c = 0; // turn off non-selected pixels
      if(testpos == i) {c= 0xFFFF00;} // 4 pixels on...
      pixels.setPixelColor(  NormalizeRingPos(i+leftOff), c); // First eye
      pixels.setPixelColor(16 + NormalizeRingPos(i+rightOff)  , c); // Second eye (flipped)
    }
    testpos++;
    if (testpos>15){testpos=0;}
    delay(60);
    pixels.show();
  }


  /*
    t = millis();
  if((t - prevTime) > 8000) {      // Every 8 seconds... change modes
    skull_color++;                        // Next mode
    if(skull_color > 3) {                 // End of modes?
      skull_color = 0;                    // Start modes over
    NextColor();
    }
    ClearRings();
    prevTime = t;
  }
  */
}

void ClearRings(){
    SolidRing(0);
}

void SolidRing(uint32_t c){
    for(i=0; i<MAX_PIXELS; i++) pixels.setPixelColor(i, c);
    pixels.show();
}

void FlashRing(){
  SolidRing(0x222222);
  delay(100);
  SolidRing(0);
}

void NextColor(){
        color >>= 8;                 // Next color R->G->B
      if(!color) color = 0xFF0000; // Reset to red
}

uint8_t NormalizeRingPos(uint8_t realPos){

  while (realPos < 0) { realPos += 16;}
  while (realPos > 15) { realPos -= 16; }
  return realPos;
}

uint8_t ReflectVertical(uint8_t ringPos){
}

/*
Left-to-right sweep:
0: C C
1: D B
2: E A
3: F 9
4: 0 8
5: 1 7
6: 2 6
7: 3 5
8: 4 4

Reflect:
0: 8
1: 7
2: 6
3: 5
4: 4
5: 3
6: 2
7: 1
8: 0
9: F
A: E
B: D
C: C
D: B
E: A
F: 9
*/


// BUTTON WITH DEBOUNCE

boolean GetButtonState(){
// no inputes - everything is global
  boolean retVal = false;
  unsigned long now = millis();


    if (digitalRead(BUTTON_PIN) == HIGH){

      button_state = 1;
    } else {
      button_state = 0;

    }

    if(button_state == prev_button_state){
      if (now - button_state_start_time > BUTTON_BOUNCE_TIME){
        // button is stable: update
        if (button_state){ // button pressed
          if (button_seen_up){
            // new press - actually do something
            retVal =  true;

            button_seen_up  = 0;
          }
        } else { // but not pressed:
          button_seen_up =  1;
        }
      }

    } else { // button state changed
      prev_button_state = button_state;
      button_state_start_time = now;

    }

    return retVal;
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b){
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
