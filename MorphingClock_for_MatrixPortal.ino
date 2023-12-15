String scrollText = "Rock and Scroll!";
int clockColors[] = {0, 0, 255}; // R, G, B
int menuColors[] = {0, 0, 255}; // R, G, B

/* ----------------------------------------------------------------------
Double-buffering (smooth animation) Protomatter library example.
PLEASE SEE THE "simple" EXAMPLE FOR AN INTRODUCTORY SKETCH.
Comments here pare down many of the basics and focus on the new concepts.

This example is written for a 64x32 matrix but can be adapted to others.
------------------------------------------------------------------------- */

#include <Adafruit_Protomatter.h>
#include <Fonts/FreeSans9pt7b.h>

/* ----------------------------------------------------------------------
The RGB matrix must be wired to VERY SPECIFIC pins, different for each
microcontroller board. This first section sets that up for a number of
supported boards.
------------------------------------------------------------------------- */

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

/* ----------------------------------------------------------------------
Matrix initialization is explained EXTENSIVELY in "simple" example sketch!
It's very similar here, but we're passing "true" for the last argument,
enabling double-buffering -- this permits smooth animation by having us
draw in a second "off screen" buffer while the other is being shown.
------------------------------------------------------------------------- */

Adafruit_Protomatter matrix(
  64,                         // Matrix width in pixels
  6,                          // Bit depth -- 6 here provides maximum color options
  1, rgbPins,                 // # of matrix chains, array of 6 RGB pins for each
  4, addrPins,                // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin,  // Other matrix control pins
  false);                     // Turn off double buffering because morph clock draws AND erases pixels to create animation.

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



/*
#include <SPI.h>
#include <WiFiNINA.h>

#if defined(ADAFRUIT_FEATHER_M4_EXPRESS) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ADAFRUIT_FEATHER_M0) || defined(ARDUINO_AVR_FEATHER32U4) || defined(ARDUINO_NRF52840_FEATHER) || defined(ADAFRUIT_ITSYBITSY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS) || defined(ARDUINO_AVR_ITSYBITSY32U4_3V) || defined(ARDUINO_NRF52_ITSYBITSY)
  // Configure the pins used for the ESP32 connection
#define SPIWIFI SPI      // The SPI port
#define SPIWIFI_SS 13    // Chip select pin
#define ESP32_RESETN 12  // Reset pin
#define SPIWIFI_ACK 11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1
#elif defined(ARDUINO_AVR_FEATHER328P)
#define SPIWIFI SPI     // The SPI port
#define SPIWIFI_SS 4    // Chip select pin
#define ESP32_RESETN 3  // Reset pin
#define SPIWIFI_ACK 2   // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1
#elif defined(TEENSYDUINO)
#define SPIWIFI SPI     // The SPI port
#define SPIWIFI_SS 5    // Chip select pin
#define ESP32_RESETN 6  // Reset pin
#define SPIWIFI_ACK 9   // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1
#elif defined(ARDUINO_NRF52832_FEATHER)
#define SPIWIFI SPI      // The SPI port
#define SPIWIFI_SS 16    // Chip select pin
#define ESP32_RESETN 15  // Reset pin
#define SPIWIFI_ACK 7    // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1
#elif !defined(SPIWIFI_SS)  // if the wifi definition isnt in the board variant
  // Don't change the names of these #define's! they match the variant ones
#define SPIWIFI SPI
#define SPIWIFI_SS 10   // Chip select pin
#define SPIWIFI_ACK 7   // a.k.a BUSY or READY pin
#define ESP32_RESETN 5  // Reset pin
#define ESP32_GPIO0 -1  // Not connected
#endif


#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key Index number (needed only for WEP)
unsigned int localPort = 2390;  // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28);  // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
*/




//==========================================================================

// MORPHING CLOCK
#include "MorphingClock.h"

#include "pixeldust.h"
#include "chords.h"

//==========================================================================
void drawPixel(uint16_t x, uint16_t y, uint16_t _color)
{
  const uint16_t height = 31;
  const uint16_t width = 63;
  uint16_t xOffset = 0;
  uint16_t yOffset = 0;
  matrix.drawPixel(xOffset + x, height - (y + yOffset), _color);
  matrix.show();
}

void drawLine(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint16_t c)
{
  const uint16_t height = 31;
  const uint16_t width = 63;
  uint16_t xOffset = 0;
  uint16_t yOffset = 0;
  matrix.drawLine(xOffset + x, height - (y + yOffset), xOffset + x2, height - (y2 + yOffset), c);
  matrix.show();
}

void drawPixel(Adafruit_Protomatter* d, uint16_t x, uint16_t y, uint16_t _color)
{
  const uint16_t height = 31;
  const uint16_t width = 63;
  uint16_t xOffset = 0;
  uint16_t yOffset = 0;
  d->drawPixel(xOffset + x, height - (y + yOffset), _color);
  d->show();
}

void drawLine(Adafruit_Protomatter* d, uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint16_t c)
{
  const uint16_t height = 31;
  const uint16_t width = 63;
  uint16_t xOffset = 0;
  uint16_t yOffset = 0;
  d->drawLine(xOffset + x, height - (y + yOffset), xOffset + x2, height - (y2 + yOffset), c);
  d->show();
}


//==========================================================================

void debugPrint(String label, int value)
{
  Serial.print(label);
  Serial.print(" = ");
  Serial.println(value);
}

void DrawText(int nRows, int row, char *str)
{
  int16_t x1, y1;
  uint16_t strWidth, charHeight;
  matrix.getTextBounds(str, 0, 0, &x1, &y1, &strWidth, &charHeight);  // How big is it?
  debugPrint("x1", x1);
  debugPrint("y1", y1);
  debugPrint("strWidth", strWidth);
  debugPrint("charHeight", charHeight);
  int nGaps = nRows+1;
  int gapSize = (matrix.height() - charHeight*nRows)/nGaps;

  textY = (gapSize+charHeight)*row - (y1+charHeight) ;
  debugPrint("gapSize", gapSize);
  debugPrint("textY", textY);

  textX = matrix.width() /2 - (x1 + strWidth / 2);         // Center text horizontally
  matrix.setCursor(textX, textY);
  matrix.print(str);
}


void setupButtons()
{
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

void setupMatrix()
{
    // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    // DO NOT CONTINUE if matrix setup encountered an error.
    for (;;)
      ;
  }

  uint16_t menuColor = matrix.color565(menuColors[0], menuColors[2], menuColors[1]);
  matrix.setTextColor(menuColor);

  matrix.setFont(&FreeSans9pt7b);  // FreeSans9pt7b=nice bitmap font, TomThumb
  matrix.setTextWrap(false);            // Allow text off edge
}


void setupMorphClock()
{
  setup_MorphClock(&matrix, matrix.color565(clockColors[0], clockColors[2],clockColors[1]));
}

void setupScroll()
{
  sprintf(str, scrollText.c_str());

  matrix.setFont(&FreeSans9pt7b);  // Use nice bitmap font
  matrix.setTextWrap(false);            // Allow text off edge
  matrix.setTextColor(0xFFFF);          // White
  int16_t x1, y1;
  uint16_t w, h;
  matrix.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);  // How big is it?
  textMin = -w;                                       // All text is off left edge when it reaches this point
  textY = matrix.height() / 2 - (y1 + h / 2);         // Center text vertically
  // Note: when making scrolling text like this, the setTextWrap(false)
  // call is REQUIRED (to allow text to go off the edge of the matrix),
  // AND it must be BEFORE the getTextBounds() call (or else that will
  // return the bounds of "wrapped" text).

  // Set up the colors of the bouncy balls.
  ballcolor[0] = matrix.color565(20, 20, 0);  // Dark green
  ballcolor[1] = matrix.color565(20, 0, 20);  // Dark blue
  ballcolor[2] = matrix.color565(20, 0, 0);   // ark red
}

void runApp(int appIndex)
{
  switch (appIndex)
  {
     case 0: setupMorphClock(); break;
     case 1: setupScroll(); break;
     case 2: setupChords(&matrix); break;
     case 3: setupDust(&matrix); break;
  }
}

const int appMax = 4;
String appNames[][2] = {
  {"MORPH","CLOCK"},
  {"Scroll","TEXT"},
  {"The","Chords"},
  {"Pixel","DUST"}};
int currentApp = 0;

void runMenu()
{
  unsigned long lastPress = millis();
  int lastApp = -1;
  unsigned long pauseLength = 3000; // 3 seconds

  while ((millis() - lastPress) < pauseLength)
  {
    bool upButtonPressed = digitalRead(2)==0;
    bool downButtonPressed = digitalRead(3)==0;

    if (upButtonPressed) {
      lastPress = millis();
      if (--currentApp<0) currentApp=appMax-1;
      delay(300);
    }

    if (downButtonPressed) {
      lastPress = millis();
      if (++currentApp>=appMax) currentApp=0;
      delay(300);
    }

    if (currentApp != lastApp)
    {
      matrix.fillScreen(0);

      sprintf(str, appNames[currentApp][0].c_str());
      DrawText(2, 1, str);

      sprintf(str, appNames[currentApp][1].c_str());
      DrawText(2, 2, str);
      matrix.show();

      lastApp = currentApp;
    }
  }

  matrix.fillScreen(0);
  runApp(currentApp);
}


// SETUP - RUNS ONCE AT PROGRAM START --------------------------------------

void setup(void) {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  // while (!Serial) {
  //   ;  // wait for serial port to connect. Needed for native USB port only
  // }

  setupMatrix();
  setupButtons(); 

  runMenu();
}

void loopScroll()
{
  // Every frame, we clear the background and draw everything anew.
  // This happens "in the background" with double buffering, that's
  // why you don't see everything flicker. It requires double the RAM,
  // so it's not practical for every situation.

  matrix.fillScreen(0);  // Fill background black

  matrix.setCursor(textX, textY);
  matrix.print(str);

  // Update text position for next frame. If text goes off the
  // left edge, reset its position to be off the right edge.
  if((--textX) < textMin) textX = matrix.width();

  // Draw the three bouncy balls on top of the text...
  for(byte i=0; i<3; i++) {
    // Draw 'ball'
    matrix.fillCircle(ball[i][0], ball[i][1], 5, ballcolor[i]);
    // Update ball's X,Y position for next frame
    ball[i][0] += ball[i][2];
    ball[i][1] += ball[i][3];
    // Bounce off edges
    if((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1)))
      ball[i][2] *= -1;
    if((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1)))
      ball[i][3] *= -1;
  }

  // AFTER DRAWING, A show() CALL IS REQUIRED TO UPDATE THE MATRIX!
  matrix.show(); //This is now done after every pixel/line drawn in digit.cpp

}


// LOOP - RUNS REPEATEDLY AFTER SETUP --------------------------------------

void loop(void) {
  switch (currentApp)
  {
    case 0: loop_MorphClock(); break;
    case 1: loopScroll(); break;
    case 2: loopChords(&matrix); break;
    case 3: loopDust(&matrix);  break;
  }

  delay(20);  // 20 milliseconds = ~50 frames/second
}
