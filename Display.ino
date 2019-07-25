#include <FastLED.h> // FastLED library for converting HSV to RGB and CRGB/CHSV types

// LED matrix width and height definitions--used for graphics
#define WIDTH 6
#define HEIGHT 6

// Convenience definitions for colors
#define OFF 0
#define RED 1
#define ORANGE 2
#define YELLOW 3
#define GREEN 4
#define BLUE 5
#define PURPLE 6
#define CYAN 7
#define PINK 8
#define WHITE 9

class Display {
  public:
    Display(int refreshRate);
    // User methods
    int getFrameCount();
    void init(); // For setting pins to output mode (since this shouldn't be done in constructor)
    void refresh(); // User method for refreshing display (calls _refreshDisplay())
    void setRGB(int x, int y, byte r, byte g, byte b); // Set individual LED to a RGB value
    void setHSV(int x, int y, byte h, byte s, byte v); // Set individual LED to a HSV value
    void setColor(int x, int y, int color); // Set individual LED to a pre-defined color
    void setAllColor(int color); // Set entire display to pre-defined color
    void scrollTextColor(char text[], int color, int scrollRate, int backColor); // Scroll text across display

  private:
    // 74HC595 shift registers control GND for each row and RGB values for each column of LED matrix
    static const int _ROW_REG[3] = {2,3,4}; // {CLK, Latch, Data} pins
    static const int _RED_REG[3] = {5,6,7}; // {CLK, Latch, Data} pins
    static const int _GREEN_REG[3] = {8,9,10}; // {CLK, Latch, Data} pins
    static const int _BLUE_REG[3] = {11,12,13}; // {CLK, Latch, Data} pins
    // Internal variables
    CRGB _ledMatrix[WIDTH][HEIGHT]; // 2D CRGB array representing LED matrix
    int _frameCounter; // Frame counter (once it gets to 65535 it resets back to 0 on the next count)
    int _refreshRate; // Display refresh rate in microseconds (μs)
    // Internal class methods
    void _setScrollFrame(int currMap[], int nextMap[], int scrollRate, int color, int backColor); // Utility method for text scroll methods
    void _setDisplayMapColor(int dispMap[], int color, int backColor); // Utility method for text scroll methods
    void _refreshDisplay(); // Utility method for refreshing display
    void _displayRow(int y); // Utility method for _refreshDisplay() method
};

// Display constructor
Display::Display(int refreshRate)
{
  _frameCounter = 0;
  _refreshRate = refreshRate;
}

void Display::init() {
  // Set all register pins to output mode
  for(int i = 0; i < 3; i++) {
    pinMode(_RED_REG[i], OUTPUT);
    pinMode(_GREEN_REG[i], OUTPUT);
    pinMode(_BLUE_REG[i], OUTPUT);
    pinMode(_ROW_REG[i], OUTPUT);
  }
}

void Display::refresh() {
  _refreshDisplay();
}

void Display::setRGB(int x, int y, byte r, byte g, byte b) { // RGB Ranges: [0, 255]
  _ledMatrix[x][y].setRGB(r,g,b);
}

void Display::setHSV(int x, int y, byte h, byte s, byte v) { // HSV ranges: [0, 255]
  CHSV hsv(h, s, v);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  setRGB(x, y, rgb.r, rgb.g, rgb.b);
}

void Display::setColor(int x, int y, int color) {
  switch(color)
  {
    case OFF:
      setRGB(x, y, 0, 0, 0);
      break;
    case RED:
      setRGB(x, y, 255, 0, 0);
      break;
    case ORANGE:
      setRGB(x, y, 255, 44, 0);
      break;
    case YELLOW:
      setRGB(x, y, 255, 125, 0);
      break;
    case GREEN:
      setRGB(x, y, 0, 255, 0);
      break;
    case BLUE:
      setRGB(x, y, 0, 0, 255);
      break;
    case PURPLE:
      setRGB(x, y, 255, 0, 185);
      break;
    case CYAN:
      setRGB(x, y, 0, 255, 255);
      break;
    case PINK:
      setRGB(x, y, 255, 0, 55);
      break;
    case WHITE:
      setRGB(x, y, 255, 135, 135);
      break;
  }
}

void Display::setAllColor(int color) {
  for(int y = 0; y < HEIGHT; y++) {
    for(int x = 0; x < WIDTH; x++) {
      setColor(x, y, color);
    }
  }
}

void Display::scrollTextColor(char text[], int scrollRate, int color, int backColor) {
  text = strupr(text); // Ensure all letters are uppercase
  int *currChar = MAP_SPC; // <currChar> initialized to "space" character
  int *nextChar = getCharMap(text[0]); // <nextChar> initialized to first character in <text> string
  int i = 0;
  do {
    _setScrollFrame(currChar, nextChar, scrollRate, color, backColor);
    currChar = getCharMap(text[i]);
    nextChar = getCharMap(text[i+1]);
    i++;
  } while(i < strlen(text));
  _setScrollFrame(currChar, MAP_SPC, scrollRate, color, backColor);  
}

int Display::getFrameCount() {
  return _frameCounter;
}

void Display::_setScrollFrame(int currMap[], int nextMap[], int scrollRate, int color, int backColor) {
  int dispMap[HEIGHT];
  long lastStepTime;

  // Start with the <currMap> char shifted to the left 2 times,
  // and the <nextMap> char shifted to the right 2 times.
  // Every iteration of <n>, move the <currMap> char and the <nextMap> char
  // to the left once until the <nextMap> char is right next to the place
  // where the <currMap> char started (i.e., the loop is complete). 
  for(int n = 0; n < 4; n++) {
    for(int i = 0; i < HEIGHT; i++) {
      dispMap[i] = (n < 2) ? ((currMap[i] << n+2) | (nextMap[i] >> 2-n)) : ((currMap[i] << n+2) | (nextMap[i] << n-2));
    }
    _setDisplayMapColor(dispMap, color, backColor);
    
    // Use the <scrollRate> delay time to refresh the display after each <dispMap> is set
    lastStepTime = millis();
    while(millis() - lastStepTime < scrollRate) {
      _refreshDisplay();
    }
  }
}

void Display::_setDisplayMapColor(int dispMap[], int color, int backColor) {
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      if(((dispMap[j] >> i) & 1) == 1) setColor(WIDTH-1-i, j, color);
      else setColor(WIDTH-1-i, j, backColor);
    }
  }
}

void Display::_refreshDisplay() {
  // Redraw every row with a total <_refreshRate> μs delay
  int delayBetweenEachRow = _refreshRate / HEIGHT;
  for(int i = 0; i < HEIGHT; i++) {
    _displayRow(i);
    delayMicroseconds(delayBetweenEachRow);
  }
  _frameCounter++;
}

// @input y: The row to connect to GND
void Display::_displayRow(int y) {
  /* Q6:Q1 (QG:QB) on the the 74HC595 chip corresponds to ROW0:ROW5, respectively (or for columns: R0:R5, G0:G5, B0:B5) */
  
  // Turn off all the LEDs by cutting off GND (fixes ghosting issue)
  digitalWrite(_ROW_REG[1], LOW);
  shiftOut(_ROW_REG[2], _ROW_REG[0], LSBFIRST, 0b11111111); // Setting all GND pins to HIGH
  digitalWrite(_ROW_REG[1], HIGH);

  // Set the latches to all the 74HC595 chips to LOW since we're about to calculate data
  digitalWrite(_RED_REG[1], LOW);
  digitalWrite(_GREEN_REG[1], LOW);
  digitalWrite(_BLUE_REG[1], LOW);
  digitalWrite(_ROW_REG[1], LOW);

  // Calculate which row we want to turn on right now
  int row = 1 << y+1; // Shift a single bit <y>+1 times--where the bit lands corresponds to the row that's about to be lit
  row = ~row; // Negate since GND is active low

  // Calculate which RED, GREEN, and BLUE LEDs should be on in the current row
  int rData = 0, gData = 0, bData = 0;
  for(int x = 0; x < WIDTH; x++) {
    rData |= ((_ledMatrix[x][y].r > 0) && (_frameCounter % (255/_ledMatrix[x][y].r) == 0)) ? (1 << x+1) : 0;
    gData |= ((_ledMatrix[x][y].g > 0) && (_frameCounter % (255/_ledMatrix[x][y].g) == 0)) ? (1 << x+1) : 0;
    bData |= ((_ledMatrix[x][y].b > 0) && (_frameCounter % (255/_ledMatrix[x][y].b) == 0)) ? (1 << x+1) : 0;
//    rData |= ((_ledMatrix[x][y].r > 0) && (fmod(_frameCounter/1.0, (255/_ledMatrix[x][y].r)) < 0.5)) ? (1 << x+1) : 0;
//    gData |= ((_ledMatrix[x][y].g > 0) && (fmod(_frameCounter/1.0, (255/_ledMatrix[x][y].g)) < 0.5)) ? (1 << x+1) : 0;
//    bData |= ((_ledMatrix[x][y].b > 0) && (fmod(_frameCounter/1.0, (255/_ledMatrix[x][y].b)) < 0.5)) ? (1 << x+1) : 0;
  }

  // Shift out the calculated row data to all the 74HC595 chips
  shiftOut(_RED_REG[2], _RED_REG[0], LSBFIRST, rData);
  shiftOut(_GREEN_REG[2], _GREEN_REG[0], LSBFIRST, gData);
  shiftOut(_BLUE_REG[2], _BLUE_REG[0], LSBFIRST, bData);
  shiftOut(_ROW_REG[2], _ROW_REG[0], LSBFIRST, row);

  // Set the latches to all the 74HC595 chips to HIGH since we're done calculating data and want the changes to take effect
  digitalWrite(_RED_REG[1], HIGH);  
  digitalWrite(_GREEN_REG[1], HIGH);  
  digitalWrite(_BLUE_REG[1], HIGH);
  digitalWrite(_ROW_REG[1], HIGH); // Make sure ROW is last to be set since we want all LEDs to stay off until data is ready in RGB chips
}
