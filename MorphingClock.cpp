#include "MorphingClock.h"

#include <Fonts/Picopixel.h>

#include <TimeLib.h> // Time Library provides Time and Date conversions
#include <Timezone.h>

//#include <WiFi.h>   // AdaFruit Matrix Portal M4 uses WiFiNINA instead.
#include <WiFiUdp.h>  // To communicate with NTP server
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

/*-------- TIME SERVER ----------*/
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
// static const char ntpServerName[] = "time.nist.gov";
// static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
// static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
// static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets
TimeChangeRule *tcr;           // pointer to the time change rule, use to get TZ abbrevTimeChangeRule *tcr;           // pointer to the time change rule, use to get TZ abbrev
time_t prevDisplay = 0; // when the Digital clock was displayed


//=== PREFERENCES ===
String credentials[][2] = {
    {"SSID", "SSDPassword"},
    {"OptionalOtherSSID", "OptionalOtherSSDPassword"},
};
const bool SHOW_24HOUR = false;

// Info about these settings at https://github.com/JChristensen/Timezone#coding-timechangerules
TimeChangeRule myStandardTime = {"CST", First, Sun, Nov, 2, -6 * 60};
TimeChangeRule myDaylightSavingsTime = {"CDT", Second, Sun, Mar, 2, -5 * 60};
Timezone myTZ(myStandardTime, myDaylightSavingsTime);
static const int ntpSyncIntervalInSeconds = 300; // How often to sync with time server (300 = every five minutes)


/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0

  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision

  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0)
    ; // discard any previously received packets

  Serial.println("Transmit NTP Request");
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);

  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      // Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      // NTP server responds within 50ms, so it does not account for the one second lag.
      // unsigned long secsSinceNTPRequest = (millis() - beginWait);
      // Debug("secsSinceNTPRequest=",secsSinceNTPRequest);
      unsigned long hackFactor = 1; // Without this the clock is 1 second behind (even when drawing without animation)

      return secsSince1900 - 2208988800UL + hackFactor;
    }
  }
  Serial.println("No NTP Response :-(");
  return timeNotSet; // return 0 if unable to get the time
}


void DrawText(Adafruit_Protomatter* matrix, int nRows, int row, const char *str)
{
  int16_t x1, y1;
  uint16_t strWidth, charHeight;
  matrix->getTextBounds(str, 0, 0, &x1, &y1, &strWidth, &charHeight);  // How big is it?
  int nGaps = nRows+1;
  int gapSize = (matrix->height() - charHeight*nRows)/nGaps;

  int16_t textY = (gapSize+charHeight)*row - (y1+charHeight) ;
  int16_t textX = matrix->width() /2 - (x1 + strWidth / 2);         // Center text horizontally
  matrix->setCursor(textX, textY);
  matrix->print(str);
}


void SetupWiFi(Adafruit_Protomatter* matrix)
{
  char str[50];

  while (WiFi.status() != WL_CONNECTED)
  {
    int numSSIDs = sizeof(credentials) / sizeof(credentials[0]);
    for (size_t i = 0; (i < numSSIDs); i++)
    {
      String ssid = credentials[i][0];
      String pass = credentials[i][1];

      Serial.print("Connecting to ");
      Serial.println();
      Serial.print("pass: ");
      Serial.println(credentials[i][1]);

      matrix->fillScreen(0);  // Fill background black
      matrix->setFont(&Picopixel);
      sprintf(str, "Connecting to");
      DrawText(matrix, 3, 1, str);

      DrawText(matrix, 3, 2, ssid.c_str());

      sprintf(str, ". . .");
      DrawText(matrix, 3, 3, str);
      matrix->show();
      
      WiFi.begin(ssid.c_str(), pass.c_str());

      int tries = 5 * 2;
      String dots = "";
      while (WiFi.status() != WL_CONNECTED && tries-- > 0)
      {
        delay(500);
        Serial.print(".");
        dots = dots + ".";
        //tft.drawString(dots, 320 / 2, 240 / 2 + 20);
      }

      if (WiFi.status() == WL_CONNECTED)
        break;
    }
  }
}


void SetupNTP()
{
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(ntpSyncIntervalInSeconds);
  while (timeStatus() == timeNotSet)
  {
    Serial.print(".");
    delay(100);
  }
}


//=== SEGMENTS ===
#include "Digit.h"
Digit digit0;
Digit digit1;
Digit digit2;
Digit digit3;
Digit digit4;
Digit digit5;

//=== CLOCK ===
bool doAnimation = false;  // Animation rely on previous digit already drawn. If none yet, don't animate. Just draw the digit.
byte prevhh;
byte prevmm;
byte prevss;

void setup_MorphClock(Adafruit_Protomatter* matrix, uint16_t clockColor) {
  SetupWiFi(matrix);
  SetupNTP();


  digit0.Init(matrix, 0, 63 - 1 - 9 * 1, 8, clockColor);
  digit1.Init(matrix, 0, 63 - 1 - 9 * 2, 8, clockColor);
  digit2.Init(matrix, 0, 63 - 4 - 9 * 3, 8, clockColor);
  digit3.Init(matrix, 0, 63 - 4 - 9 * 4, 8, clockColor);
  digit4.Init(matrix, 0, 63 - 7 - 9 * 5, 8, clockColor);
  digit5.Init(matrix, 0, 63 - 7 - 9 * 6, 8, clockColor);

  matrix->fillScreen(matrix->color565(0, 0, 0));  // Fill background black
  digit1.DrawColon(clockColor);
  digit3.DrawColon(clockColor);

  // DRAW VERSION #
  digit3.Draw(0);
  digit2.Draw(0);

  digit1.Draw(0);
  digit0.Draw(0);

  digit5.Draw(0);
  digit4.Draw(0);

  // delay(2000);
  // digit0.Morph(1); delay(2000);
  // digit0.Morph(2); delay(2000);
  // digit0.Morph(3); delay(2000);
  // digit0.Morph(4); delay(2000);
  // digit0.Morph(5); delay(2000);
  // digit0.Morph(6); delay(2000);
  // digit0.Morph(7); delay(2000);
  // digit0.Morph(8); delay(2000);
  // digit0.Morph(9); delay(2000);
  // digit0.Morph(0); delay(2000);

  // delay(15000);
}

int GetHours(unsigned long currentTime) {
  int hours = (currentTime % 86400L) / 3600;
  // Always display as 12 hour clock
  if (hours == 0) hours = 12;   // Midnight in military time is 0:mm, but we want midnight to be 12:mm
  if (hours > 12) hours -= 12;  // After noon 13:mm should show as 01:mm, etc...
  return hours;
}

int GetMinutes(unsigned long currentTime) {
  return (currentTime % 3600) / 60;
}

int GetSeconds(unsigned long currentTime) {
  return currentTime % 60;
}


void loop_MorphClock() {
  time_t current = now();
  if (current != prevDisplay) {
    // Convert UTC to local time
    time_t local = myTZ.toLocal(current, &tcr);
    int hh = SHOW_24HOUR ? hour(local) : hourFormat12(local);
    int mm = minute(local);
    int ss = second(local);

    if (doAnimation) {
      if (ss != prevss) {
        Serial.println();
        Serial.print("=== New Seconds ");
        Serial.print(ss);
        Serial.println(" ===");
        int s0 = ss % 10;
        int s1 = ss / 10;
        if (s0 != digit0.Value()) digit0.Morph(s0);
        if (s1 != digit1.Value()) digit1.Morph(s1);
        prevss = ss;
      }

      if (mm != prevmm) {
        Serial.println();
        Serial.print("=== New Minutes ");
        Serial.print(mm);
        Serial.println(" ===");
        int m0 = mm % 10;
        int m1 = mm / 10;
        if (m0 != digit2.Value()) digit2.Morph(m0);
        if (m1 != digit3.Value()) digit3.Morph(m1);
        prevmm = mm;
      }

      if (hh != prevhh) {
        Serial.println();
        Serial.print("=== New Hours ");
        Serial.print(hh);
        Serial.println(" ===");
        int h0 = hh % 10;
        int h1 = hh / 10;
        if (h0 != digit4.Value()) digit4.Morph(h0);
        if (h1 != digit5.Value()) digit5.Morph(h1);
        prevhh = hh;
      }
    } else {
      // If we didn't have a previous time. Just draw it without morphing.
      digit0.Draw(ss % 10);
      digit1.Draw(ss / 10);
      digit2.Draw(mm % 10);
      digit3.Draw(mm / 10);
      digit4.Draw(hh % 10);
      digit5.Draw(hh / 10);

      prevss = ss;
      prevmm = mm;
      prevhh = hh;
      doAnimation = true;
    }

    prevDisplay = current;
  }
}
