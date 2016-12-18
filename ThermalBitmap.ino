#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"
#include <avr/pgmspace.h>

// Improved Perlin Noise from http://www.kasperkamperman.com/blog/perlin-noise-improved-noise-on-arduino/

// permutation array
PROGMEM const unsigned char p[] = {
   171,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
};

// array below generated with this code line in the INoise code.
PROGMEM const uint16_t fadeArray[] = {
0,    0,    0,    0,    0,    0,    0,    0,    1,    1,    2,    3,    3,    4,    6,    7,
9,    10,   12,   14,   17,   19,   22,   25,   29,   32,   36,   40,   45,   49,   54,   60,
65,   71,   77,   84,   91,   98,   105,  113,  121,  130,  139,  148,  158,  167,  178,  188,
199,  211,  222,  234,  247,  259,  273,  286,  300,  314,  329,  344,  359,  374,  390,  407,
424,  441,  458,  476,  494,  512,  531,  550,  570,  589,  609,  630,  651,  672,  693,  715,
737,  759,  782,  805,  828,  851,  875,  899,  923,  948,  973,  998,  1023, 1049, 1074, 1100,
1127, 1153, 1180, 1207, 1234, 1261, 1289, 1316, 1344, 1372, 1400, 1429, 1457, 1486, 1515, 1543,
1572, 1602, 1631, 1660, 1690, 1719, 1749, 1778, 1808, 1838, 1868, 1898, 1928, 1958, 1988, 2018,
2048, 2077, 2107, 2137, 2167, 2197, 2227, 2257, 2287, 2317, 2346, 2376, 2405, 2435, 2464, 2493,
2523, 2552, 2580, 2609, 2638, 2666, 2695, 2723, 2751, 2779, 2806, 2834, 2861, 2888, 2915, 2942,
2968, 2995, 3021, 3046, 3072, 3097, 3122, 3147, 3172, 3196, 3220, 3244, 3267, 3290, 3313, 3336,
3358, 3380, 3402, 3423, 3444, 3465, 3486, 3506, 3525, 3545, 3564, 3583, 3601, 3619, 3637, 3654,
3672, 3688, 3705, 3721, 3736, 3751, 3766, 3781, 3795, 3809, 3822, 3836, 3848, 3861, 3873, 3884,
3896, 3907, 3917, 3928, 3937, 3947, 3956, 3965, 3974, 3982, 3990, 3997, 4004, 4011, 4018, 4024,
4030, 4035, 4041, 4046, 4050, 4055, 4059, 4063, 4066, 4070, 4073, 4076, 4078, 4081, 4083, 4085,
4086, 4088, 4089, 4091, 4092, 4092, 4093, 4094, 4094, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
};

// default Adafruit Thermal Printer init code

#define TX_PIN 6 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 5 // Arduino receive   GREEN WIRE   labeled TX on printer
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor


// Set up an array to hold a row of bitmap pixels
// This _should_ be bytes, not pixels (bits) but I couldn't find the right number that
// wouldn't cause the Arduino to crash part-way through a print.
// For now: 384 = the number of dots the thermal printer can print on a line
uint8_t pixels[384];

// vars for the print
int row = 0; // current row
int printRows = 1000; // total number of rows in the image -- 1000 = ~4.88inches (~124mm)

// vars for the noise algo
int x = 0;
float x0 = 0.0f;
float y0 = 0.0f;
float r = -3.0f;
float r2 = 0.0f;

boolean isrunning = true; // bool to figure out when to stop

void setup() {
  delay(4000); // don't start printing immediately, giving time to plug in an upload new code.

  // cheap indicator of when done
  pinMode(13, OUTPUT);
  digitalWrite(13,LOW);

  randomSeed(analogRead(0));

  mySerial.begin(19200);  // Initialize SoftwareSerial
  printer.begin();        // Init printer (same regardless of serial type)

  // not sure this is needed here, but better safe than wonder
  printer.wake();
  printer.setDefault();
  printer.feed(1);

  row = 0;
  x0 = random(0.0,5000.0); // sadly this doesn't do what I wanted, but leaving for reference
}


// This is the main drawing logic code
// -- check which row is printing
// -- if first: draw a border row
// -- if last: draw a border row
// -- if inbetween: draw side borders and row's content

void makeBitmapImage() {

    memset(pixels, 0, sizeof(pixels)); // clear bitmap row

    // handle borders
    // kept first and last their own in case wanted to do something different
    
    if (row == 0) { // first row (top border)
      for (int i=0;i<384;i++) {
        setBitPixel(pixels, i,1);
      }
    } else if (row == printRows) { // last row (bottom border)
      for (int i=0;i<384;i++) {
        setBitPixel(pixels, i,1);
      }
    } else { // everything inbetween
      setBitPixel(pixels, 1, 3); // left border
      setBitPixel(pixels, 382, 3); // right border

      int inc = floor(384/20); // number of lines in the noise drawing
      int pixw = 3; // width of the lines

      // create noise for each line in drawing, based on the noise function
      // r2 = multiplier of noise amount that raises then falls over the length of print
      //      to create flat lines at top and bottom and chaos in the middle
      for (int y=0;y<=383;y+=inc) {
        setBitPixel(pixels, y+(-r2/2.0f + inoise(x0,y*600.0f+y0,0.0f)/4096.0*r2), pixw);
      }
    }

    // move in the noise for the next row. Mess with these numbers!
    x0 += 170.0f;
    y0 += 484.0f;

    // these lines create the multiplier describes above
    r+= (row < printRows/2) ? 0.029f : -0.029f;
    r2 = max(r,0.0f); // clamp multiplier to a min of 0

    // finally, print the row to the printer
    printer.printBitmap(384, 1, pixels, false);
}

// This is the code to take a row of pixels, a pixel index, and a width and draw the correct points.
// This automatically sets the correct bit in each byte across the row

void setBitPixel(uint8_t *pxlBytes, int pxlIndex, short width) {
  for (int i=1;i<=width;i++) {
    int tmpIndex = pxlIndex - (floor(width/2)) + i; // center the width around desires pixel (use odd numbers)
    int byteIndex = floor(tmpIndex/8); // e.g.: 33 / 8 = 4.125 -> floor 4
    int bitIndex = floor(tmpIndex%8); // 3.g.: 33 % 8 = 1

    // clamp to left/right bounds
    if (byteIndex < 0) byteIndex = 0;
    if (bitIndex < 0) bitIndex = 0;
    if (byteIndex > 383) byteIndex = 383;
    if (bitIndex > 7) bitIndex = 7;

    // finally, set the appropriate bit in the byte
    bitSet(pxlBytes[byteIndex], 7-bitIndex);
  }
}

void loop() {
  if (isrunning) {
    if (row < printRows) {
      makeBitmapImage();
      row++;
    } else {
      isrunning = false;
      // make sure to print the last row (border)
      makeBitmapImage();
      // wrap things up
      finishPrint();
    }
  }
}

void finishPrint() {
  // turn on LED for fun
  digitalWrite(13,HIGH);
  // feed paper in printer to just tear off
  printer.feed(4);
}


// Improved Perlin Noise from http://www.kasperkamperman.com/blog/perlin-noise-improved-noise-on-arduino/

#define P(x) pgm_read_byte_near(p + ((x)&255))

long inoise(unsigned long x, unsigned long y, unsigned long z) 
{
      long X = x>>16 & 255, Y = y>>16 & 255, Z = z>>16 & 255, N = 1L<<16;
      x &= N-1; y &= N-1; z &= N-1;
     
      long u=fade(x),v=fade(y),w=fade(z), A=P(X)+Y, AA=P(A)+Z, AB=P(A+1)+Z, B=P(X+1)+Y, BA=P(B)+Z, BB=P(B+1)+Z;
      
      return lerp(w, lerp(v, lerp(u, grad(P(AA), x   , y   , z   ),  
                                     grad(P(BA), x-N , y   , z   )), 
                             lerp(u, grad(P(AB), x   , y-N , z   ),  
                                     grad(P(BB), x-N , y-N , z   ))),
                     lerp(v, lerp(u, grad(P(AA+1), x   , y   , z-N ),  
                                     grad(P(BA+1), x-N , y   , z-N )), 
                             lerp(u, grad(P(AB+1), x   , y-N , z-N ),
                                     grad(P(BB+1), x-N , y-N , z-N ))));
}
   
long lerp(long t, long a, long b) { return a + (t * (b - a) >> 12); }
 
long grad(long hash, long x, long y, long z) 
{ long h = hash&15, u = h<8?x:y, v = h<4?y:h==12||h==14?x:z;
  return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
 
#define F(x) pgm_read_word_near(fadeArray + (x))

long fade(long t) 
{
  long t0 = F(t >> 8), t1 = F(min(255, (t >> 8) + 1));
  return t0 + ( (t & 255) * (t1 - t0) >> 8 );
}

