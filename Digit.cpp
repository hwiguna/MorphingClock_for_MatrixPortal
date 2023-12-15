#include "Digit.h"

const byte sA = 0;
const byte sB = 1;
const byte sC = 2;
const byte sD = 3;
const byte sE = 4;
const byte sF = 5;
const byte sG = 6;
const int segHeight = 6;
const int segWidth = segHeight;
const uint16_t height = 31;
const uint16_t width = 63;

byte digitBits[] = {
  B11111100,  // 0 ABCDEF--
  B01100000,  // 1 -BC-----
  B11011010,  // 2 AB-DE-G-
  B11110010,  // 3 ABCD--G-
  B01100110,  // 4 -BC--FG-
  B10110110,  // 5 A-CD-FG-
  B10111110,  // 6 A-CDEFG-
  B11100000,  // 7 ABC-----
  B11111110,  // 8 ABCDEFG-
  B11110110,  // 9 ABCD_FG-
};

//byte sunBitmap[] {
//  B100100100,
//  B010001000,
//  B001110000,
//  B101110100,
//  B001110000,
//  B010001000,
//  B100100100
//};

//uint16_t black;

Digit::Digit() {
}

void Digit::Init(Adafruit_Protomatter* d, byte value, uint16_t xo, uint16_t yo, uint16_t color) {
  _display = d;
  _value = value;
  xOffset = xo;
  yOffset = yo;
  _color = color;
  _black = d->color565(0, 0, 0);
}

byte Digit::Value() {
  return _value;
}
void Digit::drawPixel(int x, int y, uint16_t c) {
  int xx = xOffset + x;
  int yy = height - (y + yOffset);
  // Serial.print("drawPixel at (");
  // Serial.print(xx);
  // Serial.print(",");
  // Serial.print(yy);
  // Serial.print(") color=");
  // Serial.println(c);
  _display->drawPixel(xx, yy, c);
  //_display->show();
}

void Digit::drawLine(int x, int y, int x2, int y2, uint16_t c) {
  int xx = xOffset + x;
  int yy = height - (y + yOffset);
  int xx2 =  xOffset + x2;
  int yy2 = height - (y2 + yOffset);

  // Serial.print("drawLine from (");
  // Serial.print(xx);
  // Serial.print(",");
  // Serial.print(yy);
  // Serial.print(") to (");
  // Serial.print(xx2);
  // Serial.print(",");
  // Serial.print(yy2);
  // Serial.print(") color=");
  // Serial.println(c);

  //_display->drawLine(xOffset + x, height - (y + yOffset), xOffset + x2, height - (y2 + yOffset), c);
  _display->drawLine(xx,yy, xx2, yy2, c);
  //_display->show();
}

void Digit::drawFillRect(int x, int y, uint16_t w, uint16_t h, uint16_t c) {
  int xx = xOffset + x;
  int yy = height - (y + yOffset) - (h-1); // Matrix (0,0) is on top left, yy is top left, thus, we need to also subtract the rect height)

  Serial.print("drawFillRect at (");
  Serial.print(xx);
  Serial.print(",");
  Serial.print(yy);
  Serial.print(") width,height = (");
  Serial.print(w);
  Serial.print(",");
  Serial.print(h);
  Serial.print(") color=");
  Serial.println(c);

  //_display->fillRect(xOffset + x, height - (y + yOffset), w,h, c);
  _display->fillRect(xx, yy, w, h, c);
}

void Digit::DrawColon(uint16_t c) {
  // Colon is drawn to the left of this digit
  drawFillRect(-3, segHeight - 2, 2, 2, c);
  drawFillRect(-3, segHeight + 1 + 2, 2, 2, c);
  _display->show();
}

void Digit::drawSeg(byte seg) {
  uint16_t segColor = _color; //_display->color565(0, 0, 255);
  switch (seg) {
    case sA: drawLine(1, segHeight * 2 + 2, segWidth, segHeight * 2 + 2, segColor); break;
    case sB: drawLine(segWidth + 1, segHeight * 2 + 1, segWidth + 1, segHeight + 2, segColor); break;
    case sC: drawLine(segWidth + 1, 1, segWidth + 1, segHeight, segColor); break;
    case sD: drawLine(1, 0, segWidth, 0, segColor); break;
    case sE: drawLine(0, 1, 0, segHeight, segColor); break;
    case sF: drawLine(0, segHeight * 2 + 1, 0, segHeight + 2, segColor); break;
    case sG: drawLine(1, segHeight + 1, segWidth, segHeight + 1, segColor); break;
  }

  // switch (seg) {
  //   case sA: drawLine(1, segHeight * 2 + 2, segWidth, segHeight * 2 + 2, _color); break;
  //   case sB: drawLine(segWidth + 1, segHeight * 2 + 1, segWidth + 1, segHeight + 2, _color); break;
  //   case sC: drawLine(segWidth + 1, 1, segWidth + 1, segHeight, _color); break;
  //   case sD: drawLine(1, 0, segWidth, 0, _color); break;
  //   case sE: drawLine(0, 1, 0, segHeight, _color); break;
  //   case sF: drawLine(0, segHeight * 2 + 1, 0, segHeight + 2, _color); break;
  //   case sG: drawLine(1, segHeight + 1, segWidth, segHeight + 1, _color); break;
  // }
}

void Digit::Draw(byte value) {
  // Erase all segments first, then light up the ones for the desired digit
  drawFillRect(0,0, segWidth+2, 2*segHeight+3, _black);

  byte pattern = digitBits[value];
  if (bitRead(pattern, 7)) drawSeg(sA);
  if (bitRead(pattern, 6)) drawSeg(sB);
  if (bitRead(pattern, 5)) drawSeg(sC);
  if (bitRead(pattern, 4)) drawSeg(sD);
  if (bitRead(pattern, 3)) drawSeg(sE);
  if (bitRead(pattern, 2)) drawSeg(sF);
  if (bitRead(pattern, 1)) drawSeg(sG);
  _display->show();
  _value = value;
}

void Digit::Morph2() {
  // TWO
  for (int i = 0; i <= segWidth; i++) {
    if (i < segWidth) {
      drawPixel(segWidth - i, segHeight * 2 + 2, _color);  // Grow seg A from right to left
      drawPixel(segWidth - i, segHeight + 1, _color);      // Grow seg G from right to left as well
      drawPixel(segWidth - i, 0, _color);                  // Grow seg D from right to left too
    }

    drawLine(segWidth + 1 - i, 1, segWidth + 1 - i, segHeight, _black);  // Move Seg C to Seg E
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);          // Move Seg C to Seg E
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph3() {
  // THREE
  for (int i = 0; i <= segWidth; i++) {
    drawLine(0 + i, 1, 0 + i, segHeight, _black);
    drawLine(1 + i, 1, 1 + i, segHeight, _color);
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph4() {
  // FOUR
  for (int i = 0; i < segWidth; i++) {
    drawPixel(segWidth - i, segHeight * 2 + 2, _black);  // Erase A
    drawPixel(0, segHeight * 2 + 1 - i, _color);         // Draw as F
    drawPixel(1 + i, 0, _black);                         // Erase D
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph5() {
  // FIVE
  for (int i = 0; i < segWidth; i++) {
    drawPixel(segWidth + 1, segHeight + 2 + i, _black);  // Erase B
    drawPixel(segWidth - i, segHeight * 2 + 2, _color);  // Draw as A
    drawPixel(segWidth - i, 0, _color);                  // Draw D
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph6() {
  // SIX
  for (int i = 0; i <= segWidth; i++) {
    // Move C right to left
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
    if (i > 0) drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, _black);
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph7() {
  // SEVEN
  for (int i = 0; i <= (segWidth + 1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, _black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);

    // Move F left to right
    drawLine(0 + i - 1, segHeight * 2 + 1, 0 + i - 1, segHeight + 2, _black);
    drawLine(0 + i, segHeight * 2 + 1, 0 + i, segHeight + 2, _color);

    // Erase D and G gradually
    drawPixel(1 + i, 0, _black);              // D
    drawPixel(1 + i, segHeight + 1, _black);  // G
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph8() {
  // EIGHT
  for (int i = 0; i <= segWidth; i++) {
    // Move B right to left
    drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2, _color);
    if (i > 0) drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1, segHeight + 2, _black);

    // Move C right to left
    drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
    if (i > 0) drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, _black);

    // Gradually draw D and G
    if (i < segWidth) {
      drawPixel(segWidth - i, 0, _color);              // D
      drawPixel(segWidth - i, segHeight + 1, _color);  // G
    }
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph9() {
  // NINE
  for (int i = 0; i <= (segWidth + 1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, _black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph0() {
  // ZERO
  for (int i = 0; i <= segWidth; i++) {
    if (_value == 1) {  // If 1 to 0, slide B to F and E to C
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2, _color);
      if (i > 0) drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1, segHeight + 2, _black);

      // slide E to C
      drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
      if (i > 0) drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, _black);

      if (i < segWidth) drawPixel(segWidth - i, segHeight * 2 + 2, _color);  // Draw A
      if (i < segWidth) drawPixel(segWidth - i, 0, _color);                  // Draw D
    }

    if (_value == 2) {  // If 2 to 0, slide B to F and Flow G to C
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2, _color);
      if (i > 0) drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1, segHeight + 2, _black);

      drawPixel(1 + i, segHeight + 1, _black);                               // Erase G left to right
      if (i < segWidth) drawPixel(segWidth + 1, segHeight + 1 - i, _color);  // Draw C
    }

    if (_value == 3) {  // 3 to 0, B to F, C to E
      // slide B to F
      drawLine(segWidth - i, segHeight * 2 + 1, segWidth - i, segHeight + 2, _color);
      if (i > 0) drawLine(segWidth - i + 1, segHeight * 2 + 1, segWidth - i + 1, segHeight + 2, _black);

      // Move C to E
      drawLine(segWidth - i, 1, segWidth - i, segHeight, _color);
      if (i > 0) drawLine(segWidth - i + 1, 1, segWidth - i + 1, segHeight, _black);

      // Erase G from right to left
      drawPixel(segWidth - i, segHeight + 1, _black);  // G
    }

    if (_value == 5) {  // If 5 to 0, we also need to slide F to B
      if (i < segWidth) {
        if (i > 0) drawLine(1 + i, segHeight * 2 + 1, 1 + i, segHeight + 2, _black);
        drawLine(2 + i, segHeight * 2 + 1, 2 + i, segHeight + 2, _color);
      }
    }

    if (_value == 5 || _value == 9) {  // If 9 or 5 to 0, Flow G into E
      if (i < segWidth) drawPixel(segWidth - i, segHeight + 1, _black);
      if (i < segWidth) drawPixel(0, segHeight - i, _color);
    }
    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph1() {
  // Zero or two to One
  for (int i = 0; i <= (segWidth+1); i++) {
    // Move E left to right
    drawLine(0 + i - 1, 1, 0 + i - 1, segHeight, _black);
    drawLine(0 + i, 1, 0 + i, segHeight, _color);

    // Move F left to right
    drawLine(0 + i - 1, segHeight * 2 + 1, 0 + i - 1, segHeight + 2, _black);
    drawLine(0 + i, segHeight * 2 + 1, 0 + i, segHeight + 2, _color);

    // Gradually Erase A, G, D
    drawPixel(1 + i, segHeight * 2 + 2, _black);  // A
    drawPixel(1 + i, 0, _black);                  // D
    drawPixel(1 + i, segHeight + 1, _black);      // G

    _display->show();
    delay(animSpeed);
  }
}

void Digit::Morph(byte newValue) {
  int d0= 63 - 1 - 9 * 1;
  int d1= 63 - 1 - 9 * 2;
  int d2= 63 - 4 - 9 * 3;
  int d3= 63 - 4 - 9 * 4;
  int d4= 63 - 7 - 9 * 5;
  int d5= 63 - 7 - 9 * 6;

  int digitIndex=0;
  if (xOffset==d1) digitIndex=1;
  if (xOffset==d2) digitIndex=2;
  if (xOffset==d3) digitIndex=3;
  if (xOffset==d4) digitIndex=4;
  if (xOffset==d5) digitIndex=5;
      
  Serial.println();
  Serial.print("Digit ");
  Serial.print(digitIndex);
  Serial.print(" is morphing ");
  Serial.print(_value);
  Serial.print(" into ");
  Serial.println(newValue);

  bool isValidMorph = false;
  if (newValue == 0) {
    isValidMorph = _value == 1 || _value == 2 || _value == 3 || _value == 5 || _value == 9;
  } else {
    isValidMorph = newValue == _value + 1;
  }

  if (isValidMorph) {
    switch (newValue) {
      case 2: Morph2(); break;
      case 3: Morph3(); break;
      case 4: Morph4(); break;
      case 5: Morph5(); break;
      case 6: Morph6(); break;
      case 7: Morph7(); break;
      case 8: Morph8(); break;
      case 9: Morph9(); break;
      case 0: Morph0(); break;
      case 1: Morph1(); break;
    }
    _value = newValue;
  }
  else {
   Serial.println("OOPS! Invalid morph. Just draw it!");
   Draw(newValue);
  }
}
