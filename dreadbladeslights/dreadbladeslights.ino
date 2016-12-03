
#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif
#define PIN 0
#define MAX_PIXELS 20
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(MAX_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

uint8_t  mode   = 0, // Current animation effect
// "left" is closest to cpu
         leftOff = 7, // Position of spinny eyes
         rightOff = 2,
         pos = 0;
uint8_t  i; // generic index

uint8_t testpos = 0;
uint32_t color  = 0xFF0000; // Start red
uint32_t prevTime;

int32_t hires_pos = 0, // 100x actual pos so we can fake floats
  inertia = 0,
  moment,
  spring_force = 0;


#define system_size 800
#define scale2pixel 100 // scale to 16 pixels (800*2/16)
#define friction  90 // (100-89)/100
#define spring_constant 36 // 36/100 = .36

const byte polar_mapped_coords[] ={
00, 19, 9, 19,  8, 10,  7, 11,  6, 12,
 5, 13,  4, 14, 3, 15,  2, 16,  1, 17
};


void setup() {
  randomSeed(analogRead(1));
  #ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
  if(F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();
  //pixels.setBrightness(255);
  prevTime = millis();
  SolidRing(0xFF0000);
}

void loop() {

  uint32_t t;
mode= 99;
  switch(mode) {

   case 0: // Random sparks - just one LED on at a time!
   // ======================================================
    i = random(MAX_PIXELS);
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(10);
    pixels.setPixelColor(i, 0);
    break;



   case 1: // Spinny wheels (8 LEDs on at a time)
  // ======================================================
    for(i=0; i<16; i++) {
      uint32_t c = 0; // turn off non-selected pixels
      if(((pos + i) & 7) < 2) c = color; // 4 pixels on...
      pixels.setPixelColor(  NormalizeRingPos(i+leftOff), c); // First eye
      pixels.setPixelColor(16 + NormalizeRingPos(16-i+rightOff)  , c); // Second eye (flipped)
    }
    pixels.show();
    pos = pos++ % 16;
    delay(50);

    break;

case 2: // rotating dot
    //pixels.setPixelColor( pos, 0) ;
     pixels.setPixelColor( polar_mapped_coords[pos], 0) ;

  pos++;
  if(pos>=MAX_PIXELS){pos=0;}
  //pixels.setPixelColor( pos, color) ;
   pixels.setPixelColor( polar_mapped_coords[pos], color) ;
  pixels.show();
  delay(20);
break;


      case 3: // sequencer
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

  t = millis();
  /*
  if((t - prevTime) > 8000) {      // Every 8 seconds... change modes
    mode++;                        // Next mode
    if(mode > 3) {                 // End of modes?
      mode = 0;                    // Start modes over
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
