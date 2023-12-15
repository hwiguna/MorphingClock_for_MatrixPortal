#include "chords.h"

/* ----------------------------------------------------------------------
Double-buffering (smooth animation) Protomatter library example.
PLEASE SEE THE "simple" EXAMPLE FOR AN INTRODUCTORY SKETCH.
Comments here pare down many of the basics and focus on the new concepts.

This example is written for a 64x32 matrix but can be adapted to others.
------------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
The RGB matrix must be wired to VERY SPECIFIC pins, different for each
microcontroller board. This first section sets that up for a number of
supported boards.
------------------------------------------------------------------------- */

/*
#if defined(_VARIANT_MATRIXPORTAL_M4_)  // MatrixPortal M4
uint8_t rgbPins[] = { 7, 8, 9, 10, 11, 12 };
uint8_t addrPins[] = { 17, 18, 19, 20, 21 };
uint8_t clockPin = 14;
uint8_t latchPin = 15;
uint8_t oePin = 16;
#elif defined(_VARIANT_FEATHER_M4_)           // Feather M4 + RGB Matrix FeatherWing
uint8_t rgbPins[] = { 6, 5, 9, 11, 10, 12 };
uint8_t addrPins[] = { A5, A4, A3, A2 };
uint8_t clockPin = 13;
uint8_t latchPin = 0;
uint8_t oePin = 1;
#elif defined(__SAMD51__)                     // M4 Metro Variants (Express, AirLift)
uint8_t rgbPins[] = { 6, 5, 9, 11, 10, 12 };
uint8_t addrPins[] = { A5, A4, A3, A2 };
uint8_t clockPin = 13;
uint8_t latchPin = 0;
uint8_t oePin = 1;
#elif defined(_SAMD21_)                       // Feather M0 variants
uint8_t rgbPins[] = { 6, 7, 10, 11, 12, 13 };
uint8_t addrPins[] = { 0, 1, 2, 3 };
uint8_t clockPin = SDA;
uint8_t latchPin = 4;
uint8_t oePin = 5;
#elif defined(NRF52_SERIES)                   // Special nRF52840 FeatherWing pinout
uint8_t rgbPins[] = { 6, A5, A1, A0, A4, 11 };
uint8_t addrPins[] = { 10, 5, 13, 9 };
uint8_t clockPin = 12;
uint8_t latchPin = PIN_SERIAL1_RX;
uint8_t oePin = PIN_SERIAL1_TX;
#elif USB_VID == 0x239A && USB_PID == 0x8113  // Feather ESP32-S3 No PSRAM
// M0/M4/RP2040 Matrix FeatherWing compatible:
uint8_t rgbPins[] = { 6, 5, 9, 11, 10, 12 };
uint8_t addrPins[] = { A5, A4, A3, A2 };
uint8_t clockPin = 13;  // Must be on same port as rgbPins
uint8_t latchPin = RX;
uint8_t oePin = TX;
#elif USB_VID == 0x239A && USB_PID == 0x80EB  // Feather ESP32-S2
// M0/M4/RP2040 Matrix FeatherWing compatible:
uint8_t rgbPins[] = { 6, 5, 9, 11, 10, 12 };
uint8_t addrPins[] = { A5, A4, A3, A2 };
uint8_t clockPin = 13;  // Must be on same port as rgbPins
uint8_t latchPin = RX;
uint8_t oePin = TX;
#elif defined(ESP32)
// 'Safe' pins, not overlapping any peripherals:
// GPIO.out: 4, 12, 13, 14, 15, 21, 27, GPIO.out1: 32, 33
// Peripheral-overlapping pins, sorted from 'most expendible':
// 16, 17 (RX, TX)
// 25, 26 (A0, A1)
// 18, 5, 9 (MOSI, SCK, MISO)
// 22, 23 (SCL, SDA)
uint8_t rgbPins[] = { 4, 12, 13, 14, 15, 21 };
uint8_t addrPins[] = { 16, 17, 25, 26 };
uint8_t clockPin = 27;  // Must be on same port as rgbPins
uint8_t latchPin = 32;
uint8_t oePin = 33;
#elif defined(ARDUINO_TEENSY40)
uint8_t rgbPins[] = { 15, 16, 17, 20, 21, 22 };  // A1-A3, A6-A8, skip SDA,SCL
uint8_t addrPins[] = { 2, 3, 4, 5 };
uint8_t clockPin = 23;  // A9
uint8_t latchPin = 6;
uint8_t oePin = 9;
#elif defined(ARDUINO_TEENSY41)
uint8_t rgbPins[] = { 26, 27, 38, 20, 21, 22 };  // A12-14, A6-A8
uint8_t addrPins[] = { 2, 3, 4, 5 };
uint8_t clockPin = 23;  // A9
uint8_t latchPin = 6;
uint8_t oePin = 9;
#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040)
// RP2040 support requires the Earle Philhower board support package;
// will not compile with the Arduino Mbed OS board package.
// The following pinout works with the Adafruit Feather RP2040 and
// original RGB Matrix FeatherWing (M0/M4/RP2040, not nRF version).
// Pin numbers here are GP## numbers, which may be different than
// the pins printed on some boards' top silkscreen.
uint8_t rgbPins[] = { 8, 7, 9, 11, 10, 12 };
uint8_t addrPins[] = { 25, 24, 29, 28 };
uint8_t clockPin = 13;
uint8_t latchPin = 1;
uint8_t oePin = 0;
#endif
*/


/* ----------------------------------------------------------------------
Matrix initialization is explained EXTENSIVELY in "simple" example sketch!
It's very similar here, but we're passing "true" for the last argument,
enabling double-buffering -- this permits smooth animation by having us
draw in a second "off screen" buffer while the other is being shown.
------------------------------------------------------------------------- */

/*
Adafruit_Protomatter matrix(
  64,                         // Matrix width in pixels
  6,                          // Bit depth -- 6 here provides maximum color options
  1, rgbPins,                 // # of matrix chains, array of 6 RGB pins for each
  4, addrPins,                // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin,  // Other matrix control pins
  true);                      // HERE IS THE MAGIC FOR DOUBLE-BUFFERING!

// Sundry globals used for animation ---------------------------------------

int16_t textX = matrix.width(),  // Current text position (X)
  textY,                         // Current text position (Y)
  textMin,                       // Text pos. (X) when scrolled off left edge
  hue = 0;
char str[50];  // Buffer to hold scrolling message text
int8_t ball[3][4] = {
  { 3, 0, 1, 1 },  // Initial X,Y pos+velocity of 3 bouncy balls
  { 17, 15, 1, -1 },
  { 27, 4, -1, 1 }
};
uint16_t ballcolor[3];  // Colors for bouncy balls (init in setup())
*/

const int chordNodeMax = 10;
int16_t chordNodes[chordNodeMax][2];
uint16_t chordColor;

void setupChords(Adafruit_Protomatter* matrix) {
}

void drawChord(Adafruit_Protomatter* matrix, int16_t chordX0, int16_t chordY0, int chordNodeCount, int chordRadius, float chordRot) {
  // Compute nodes
  if (chordNodeCount>chordNodeMax) chordNodeCount=chordNodeMax;
  for (int i = 0; i < chordNodeCount; i++) {
    float a = chordRot + (2 * PI * i / chordNodeCount);
    chordNodes[i][0] = chordX0 + cos(a) * chordRadius;
    chordNodes[i][1] = chordY0 + sin(a) * chordRadius;
  }

  // Draw Chords
  matrix->fillScreen(0);
  for (int i = 0; i < (chordNodeCount - 1); i++) {
    uint16_t x1 = chordNodes[i][0];
    uint16_t y1 = chordNodes[i][1];
    for (int j = i + 1; j < chordNodeCount; j++) {
      uint16_t x2 = chordNodes[j][0];
      uint16_t y2 = chordNodes[j][1];
      matrix->drawLine(x1, y1, x2, y2, chordColor);
    }
  }
}

void loopChords(Adafruit_Protomatter* matrix) {
  int chordNodeCount = random(3,9);
  int chordRadius = random(16,64);;
  int chordX0 = random(0,64);
  int chordY0 = random(0,32);
  float chordRot = 0;

  int chordX = chordX0;
  int chordY = chordY0;
  int xOffset = 0;
  int yOffset = 0;
  float rotStep = 2 * PI * random(-30,30) / 360;
  int frameCount = 31;
  chordColor = matrix->color565(random(0,20), random(0,20), random(0,20));
  for (int f = 0; f < frameCount; f++) {
    chordX += xOffset;
    chordY += yOffset;
    chordRot += rotStep;
    int r = (10+f) * chordRadius / frameCount;
    drawChord(matrix, chordX, chordY, chordNodeCount, r, chordRot);
    matrix->show();
    delay(100);
  }
}

// SETUP - RUNS ONCE AT PROGRAM START --------------------------------------

/*
void setup(void) {
  Serial.begin(9600);

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    // DO NOT CONTINUE if matrix setup encountered an error.
    for (;;)
      ;
  }

  setupChords();
}
*/
