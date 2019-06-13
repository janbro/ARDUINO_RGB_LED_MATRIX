#include <FastLED.h>

// Used for graphics
#define WIDTH 6
#define HEIGHT 6

// Range for REFRESH_RATE: [0, 65535] μs; Range for ANIMATION_RATE: [0, 65535] ms
#define REFRESH_RATE 0 // Microseconds (0 to 0.65535 seconds)
#define ANIMATION_RATE 1000 // Milliseconds (0 to 65.535 seconds)

#define SCROLL_TEXT_RATE 500 // Time in milliseconds between scroll steps

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

// Shift registers. Each control parallel r/g/b values for each column
int RED_REG[3] = {5,6,7}; // CLK, Latch, Data
int GREEN_REG[3] = {8,9,10}; // CLK, Latch, Data
int BLUE_REG[3] = {11,12,13}; // CLK, Latch, Data

int ROW_REG[3] = {2,3,4}; // CLK, Latch, Data

int currX = 0, currY = 0, currR = 255, currG = 0, currB = 0, currH = 0, currS = 255, currV = 255; // For animations--color starts at red
long lastStepTime = 0; // For animations to know when to take the next step
int frameCounter = 0; // Frame counter (once it gets to 65535 it resets back to 0 on the next count)

String inString = "";

CRGB Display[WIDTH][HEIGHT] = {};

//void scrollTextColor(String text, int color, int backColor);

void setup() {
  Serial.begin(9600);
  Serial.println(1 << 0);
  for(int i=0; i<3; i++) {
    pinMode(RED_REG[i], OUTPUT);
    pinMode(GREEN_REG[i], OUTPUT);
    pinMode(BLUE_REG[i], OUTPUT);
    pinMode(ROW_REG[i], OUTPUT);
  }

  drawUSAFlag();
  scrollTextColor("ABBA", CYAN, OFF);
//  int intensity = 255;
//  for(int j = 0; j < HEIGHT-1; j++) {
//    if(intensity < 0) intensity = 0;
//    for(int i = 0; i < WIDTH; i++) {
//      //Serial.println(intensity);
//      setLEDRGB(i,j,0,0,intensity);
//    }
//    intensity -= 51;
//  }
}

void loop() {
  // Calculate how the display should look right now
  //animate();

  // Redraw the display
  refreshDisplay();
  frameCounter++;
}

void animate() {
  // If it's been at least <ANIMATION_RATE> ms since we last took a step, take the next step and update <lastStepTime>
  if(millis() - lastStepTime >= ANIMATION_RATE) {
    //snakeAnimNextStep();
    //rowAnimNextStep();
    //singleAnimNextStep();
    //fireworkNextStep();
    //radialGrowNextStep();
    //linesNextStep();
    //PongNextStep();
    //rainbowAnimNextStep();
    testColorsAnimNextStep();
    lastStepTime = millis();
  }
}

void refreshDisplay() {
  // Redraw every row with a total <REFRESH_RATE> μs delay
  int delayBetweenEachRow = REFRESH_RATE / HEIGHT;
  for(int i = 0; i < HEIGHT; i++) {
    displayRow(i);
    delayMicroseconds(delayBetweenEachRow);
    //delay(1000);
  }
}

// @input y: The row to connect to GND
void displayRow(int y) {
  /* Q6:Q1 on the the 74HC595 chip corresponds to ROW0:ROW5, respectively (or for columns: R0:R5, G0:G5, B0:B5) */
  // Set the latches to all the 74HC595 chips to LOW since we're about to calculate data
  digitalWrite(RED_REG[1], LOW);
  digitalWrite(GREEN_REG[1], LOW);
  digitalWrite(BLUE_REG[1], LOW);
  digitalWrite(ROW_REG[1], LOW);

  // Calculate which RED, GREEN, and BLUE LEDs should be on in the current row
  int rData = 0, gData = 0, bData = 0;
  for(int x = 0; x < WIDTH; x++) {
    rData += ((Display[x][y].r > 0) && (frameCounter % (255/Display[x][y].r) == 0)) ? (1 << x+1) : 0;
    gData += ((Display[x][y].g > 0) && (frameCounter % (255/Display[x][y].g) == 0)) ? (1 << x+1) : 0;
    bData += ((Display[x][y].b > 0) && (frameCounter % (255/Display[x][y].b) == 0)) ? (1 << x+1) : 0;
//    rData += ((Display[x][y].r > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].r)) < 0.5)) ? (1 << x+1) : 0;
//    gData += ((Display[x][y].g > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].g)) < 0.5)) ? (1 << x+1) : 0;
//    bData += ((Display[x][y].b > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].b)) < 0.5)) ? (1 << x+1) : 0;
  }

  // Calculate which row we want to turn on right now
  int row = 1 << y+1; // Shift a single bit <y>+1 times--where the bit lands corresponds to the row that's about to be lit
  row = ~row; // Negate since GND is active low

  // Shift out the calculated row data to all the 74HC595 chips
  shiftOut(RED_REG[2], RED_REG[0], LSBFIRST, rData);
  shiftOut(GREEN_REG[2], GREEN_REG[0], LSBFIRST, gData);
  shiftOut(BLUE_REG[2], BLUE_REG[0], LSBFIRST, bData);
  shiftOut(ROW_REG[2], ROW_REG[0], LSBFIRST, row);

  // Set the latches to all the 74HC595 chips to HIGH since we're done calculating data and want the changes to take effect
  digitalWrite(RED_REG[1], HIGH);
  digitalWrite(GREEN_REG[1], HIGH);
  digitalWrite(ROW_REG[1], HIGH);
  digitalWrite(BLUE_REG[1], HIGH);
}

void snakeAnimNextStep() {
  /* DISCLAIMER: This is inefficient AF */

  // If the currY is <HEIGHT>, then set it to 0 and change colors
  if(currY == HEIGHT) {
    currY = 0;
    if(currR > 0) {
      currR = 0;
      currG = 255;
    }
    else if(currG > 0) {
      currG = 0;
      currB = 255;
    }
    else {
      currB = 0;
      currR = 255;
    }
  }

  // If <currY> is even, then animate from left to right before incrementing <currY>
  if(currY % 2 == 0) {
    if(currX < WIDTH) {
      setLEDRGB(currX,currY,currR,currG,currB);
      //setLEDRGB(currY,currX,currR,currG,currB);
      currX++;
    }
    else {
      currX = 0;
      currY++;
    }
  }

  // Else <currY> is odd, animate from right to left before incrementing <currY>
  else {
    if(currX < WIDTH) {
      setLEDRGB(WIDTH-currX-1,currY,currR,currG,currB);
      //setLEDRGB(currY,WIDTH-currX-1,currR,currG,currB);
      currX++;
    }
    else {
      currX = 0;
      currY++;
    }
  }
}

void rowAnimNextStep() {
  // If the currY is 6 (the height), then set it to 0 and change colors
  if(currY == HEIGHT) {
    currY = 0;
    if(currR > 0) {
      currR = 0;
      currG = 255;
    }
    else if(currG > 0) {
      currG = 0;
      currB = 255;
    }
    else {
      currB = 0;
      currR = 255;
    }
  }
  // Set all LEDs in the current row (<currY>) to the current color before incrementing <currY>
  for(int i = 0; i < WIDTH; i++) {
    setLEDRGB(i, currY, currR, currG, currB);
    //setLEDRGB(currY, i, currR, currG, currB);
  }
  currY++;
}

void singleAnimNextStep() {
  if(currY == HEIGHT) {
    currY = 0;
    currR = (currR == 255) ? 0 : 255;
    currB = (currB == 255) ? 0 : 255;
  }
  for(int i = 0; i < HEIGHT; i++) {
    if(i == currY) setLEDRGB(0,i,currR,0,currB);
    else setLEDRGB(0,i,0,0,0);
  }
  currY++;
}

struct Firework {
  int hue;
  int x, y;
  bool fired;
  bool exploded;
  int life;
  int tick;
};

Firework f = {0, 0, 0, false, false, 11, 0};
void fireworkNextStep() {
  if(!f.fired) {
    f.hue = int(random(0,6))*42;
    f.x = random(0, WIDTH-1);
    f.y = random(0, HEIGHT-1);
    f.exploded = false;
    f.fired = true;
  }
  if(f.fired) {
    if(!f.exploded) {
      for(int i = 0; i < WIDTH; i++) {
        for(int j = 0; j < HEIGHT; j++) {
          setLEDRGB(i, j, 0, 0, 0);
        }
      }
      setLEDHSV(f.x, HEIGHT - 1 - f.tick, f.hue, 255, 255);
      f.tick++;
      if(HEIGHT - 1 - f.tick < f.y) {
        f.tick = 1;
        f.exploded = true;
        f.hue = int(random(0,6))*42;
      }
    }
    else {
      for(int i = 0; i < WIDTH; i++) {
        for(int j = 0; j < HEIGHT; j++) {
          if(int(sqrt(pow(i-f.x,2)+pow(j-f.y,2))) <= f.tick && int(sqrt(pow(i-f.x,2)+pow(j-f.y,2))) > f.tick - map(f.tick, 0, f.life-6, 4, 2)) {
            setLEDHSV(i, j, f.hue, 255, 255);
          } 
          else {
            setLEDRGB(i, j, 0, 0, 0);
          }
        }
      }
      f.tick++;
      if(f.tick >= f.life) {
        f.fired = false;
        f.tick = 0;
      }
    }
  }
}

void radialGrowNextStep() {
  int center[] = {3, 0};
  for(int i = 0; i < WIDTH; i++) {
    for(int j = 0; j < HEIGHT; j++) {
      if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= (frameCounter % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > (frameCounter % (HEIGHT+2)) - 2) {
        setLEDRGB(i, j, 255, 0, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frameCounter - 2) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frameCounter - 2) % (HEIGHT+2)) - 2) {
        setLEDRGB(i, j, 0, 255, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frameCounter - 4) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frameCounter - 4) % (HEIGHT+2)) - 2) {
        setLEDRGB(i, j, 0, 0, 255);
      }
      else {
        setLEDRGB(i, j, 0, 0, 0);
      }
    }
  }
}

int counter = 0;
int h1, h2;
void linesNextStep() {
  if(counter % 12 == 0) {
    h1 = int(random(0, 6)) * 42;
    h2 = int(random(0, 6)) * 42;
  }
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      if(counter % 12 >= 6) {
        if(j % 2 == 0 && i >= (counter % 6)) {
          setLEDHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i <= (5-counter % 6)) {
          setLEDHSV(i, j, h2, 255, 255);
        }
        else {
          setLEDRGB(i, j, 0, 0, 0);
        }
      }
      else {
        if(j % 2 == 0 && i <= (counter % 6)) {
          setLEDHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i >= (5-counter % 6)) {
          setLEDHSV(i, j, h2, 255, 255);
        }
        else {
          setLEDRGB(i, j, 0, 0, 0);
        }
      }
    }
  }
  counter++;
}

struct PongBall {
  int x, y;
  int x_dir, y_dir;
};

PongBall pb = {0,2,1,1};
void PongNextStep() {
  for(int i = 0; i < WIDTH; i++) {
    for(int j = 0; j < HEIGHT; j++) {
      setLEDRGB(i, j, 0, 0, 0);
    }
  }
  setLEDRGB(pb.x, pb.y, 90, 0, 0);
  pb.x = pb.x + pb.x_dir;
  pb.y = pb.y + pb.y_dir;
  if(pb.x >= WIDTH - 1) {
    pb.x_dir = -pb.x_dir;
  } 
  else if(pb.x <= 0) {
    pb.x_dir = -pb.x_dir;
  }
  if(pb.y >= HEIGHT - 1) {
    pb.y_dir = -pb.y_dir;
  }
  else if(pb.y <= 0) {
    pb.y_dir = -pb.y_dir;
  }
}

void rainbowAnimNextStep() {
  if(currH == 256) currH = 0;
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      setLEDHSV(i, j, currH, 255, 255);
    }
  }
  currH += 8; // Allows for 31 unique hues
}

int color[10] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, CYAN, PINK, WHITE, OFF};
int currIndex = 0;
void testColorsAnimNextStep() {
  if(currIndex == 10) currIndex = 0;
  setAllColor(color[currIndex]);
  currIndex++;
}

void drawUSAFlag() {
  // Color even rows red and odd rows white (starting at row 0)
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      if(j % 2 == 0) setLEDRGB(i,j,255,255,255); // If even, color row red
      else setLEDRGB(i,j,255,0,0); // Else odd, color row white
    }
  }
  // Set 3x3 square in top right to blue
  for(int j = 0; j < 3; j++) {
    for(int i = 0; i < 3; i++) {
      setLEDRGB(i,j,0,0,255);
    }
  }
}

void setLEDRGB(int x, int y, byte r, byte g, byte b) { // RGB Ranges: [0, 255]
  Display[x][y].setRGB(r,g,b);
}

void setLEDHSV(int x, int y, byte h, byte s, byte v) { // HSV ranges: [0, 255]
  CHSV hsv(h, s, v);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
  setLEDRGB(x, y, rgb.r, rgb.g, rgb.b);
}

void setLEDColor(int x, int y, int color) {
  switch(color)
  {
    case OFF:
      setLEDRGB(x, y, 0, 0, 0);
      break;
    case RED:
      setLEDRGB(x, y, 255, 0, 0);
      break;
    case ORANGE:
      setLEDRGB(x, y, 255, 44, 0);
      break;
    case YELLOW:
      setLEDRGB(x, y, 255, 125, 0);
      break;
    case GREEN:
      setLEDRGB(x, y, 0, 255, 0);
      break;
    case BLUE:
      setLEDRGB(x, y, 0, 0, 255);
      break;
    case PURPLE:
      setLEDRGB(x, y, 255, 0, 185);
      break;
    case CYAN:
      setLEDRGB(x, y, 0, 255, 255);
      break;
    case PINK:
      setLEDRGB(x, y, 255, 0, 55);
      break;
    case WHITE:
      setLEDRGB(x, y, 255, 255, 255);
      break;
  }
}

void setAllColor(int color) {
  for(int y = 0; y < HEIGHT; y++) {
    for(int x = 0; x < WIDTH; x++) {
      setLEDColor(x, y, color);
    }
  }
}

int A_MAP[] = {
  0b000010,
  0b000101,
  0b000111,
  0b000101,
  0b000101,
  0b000000
};
int B_MAP[] = {
  0b000110,
  0b000101,
  0b000110,
  0b000101,
  0b000110,
  0b000000
};

int* getCharMap(char c)
{
  switch(c)
  {
    case 'A':
      return A_MAP;
    case 'B':
      return B_MAP;
  }
}

void scrollTextColor(String text, int color, int backColor) {
  int *currChar = NULL;
  int *nextChar = NULL;
  for(int i = 0; i < text.length() - 1; i++) {
    currChar = getCharMap(text.charAt(i));
    nextChar = getCharMap(text.charAt(i+1));
    scrollChars(currChar, nextChar, color, backColor);
  }
}

void scrollChars(int currMap[], int nextMap[], int color, int backColor) {
  int currRow;
  for(int n = 0; n < 6; n++) {
    for(int j = 0; j < HEIGHT; j++) {
      for(int i = 0; i < WIDTH; i++) {
        currRow = currMap[j] << n;
        if(n >= 3)
        {
          currRow += (nextMap[j] >> (5-n));
        }
        if(((currRow >> i) & 1) == 1) setLEDColor(WIDTH-1-i, j, color);
        else setLEDColor(WIDTH-1-i, j, backColor);
      }
    }
    lastStepTime = millis();
    while(millis() - lastStepTime < SCROLL_TEXT_RATE) {
      refreshDisplay();
    }
  }
}

void setDisplayMapColor(int dispMap[], int color, int backColor) {
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      if(((dispMap[j] >> i) & 1) == 1) setLEDColor(WIDTH-1-i, j, color);
      else setLEDColor(WIDTH-1-i, j, backColor);
    }
  }
}
