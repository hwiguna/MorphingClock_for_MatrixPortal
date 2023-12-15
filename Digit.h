#ifndef DIGIT_H
#define DIGIT_H

#include <Arduino.h>

#include <Adafruit_Protomatter.h>

class Digit {
  
  public:
    Digit();
    void Init(Adafruit_Protomatter* d, byte value, uint16_t xo, uint16_t yo, uint16_t color);
    void Draw(byte value);
    void Morph(byte newValue);
    byte Value();
    void DrawColon(uint16_t c);
    
  private:
    Adafruit_Protomatter* _display;
    byte _value;
    uint16_t _color;
    uint16_t xOffset;
    uint16_t yOffset;
    int animSpeed = 30;
    uint16_t _black;

    void drawPixel(int x, int y, uint16_t c);
    void drawFillRect(int x, int y, uint16_t w, uint16_t h, uint16_t c);
    void drawLine(int x, int y, int x2, int y2, uint16_t c);
    void drawSeg(byte seg);
    void Morph2();
    void Morph3();
    void Morph4();
    void Morph5();
    void Morph6();
    void Morph7();
    void Morph8();
    void Morph9();
    void Morph0();
    void Morph1();
};

#endif
