#include <FastLED.h>

// Used for graphics
#define WIDTH 6
#define HEIGHT 6

// Range for REFRESH_RATE: [0, 65535] μs; Range for ANIMATION_RATE: [0, 65535] ms
#define REFRESH_RATE 600 // Microseconds (0 to 0.65535 seconds)
#define ANIMATION_RATE 60 // Milliseconds (0 to 65.535 seconds)

#define SCROLL_TEXT_RATE 225 // Time in milliseconds between text scroll steps

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
long lastStepTime = 0, lastLoopTime = 0; // For animations to know when to take the next step/loop
int frameCounter = 0; // Frame counter (once it gets to 65535 it resets back to 0 on the next count)

CRGB Display[WIDTH][HEIGHT] = {};

void setup() {
  for(int i = 0; i < 3; i++) {
    pinMode(RED_REG[i], OUTPUT);
    pinMode(GREEN_REG[i], OUTPUT);
    pinMode(BLUE_REG[i], OUTPUT);
    pinMode(ROW_REG[i], OUTPUT);
  }
}

void loop() {
  scrollTextColor("HAPPY 4TH OF JULY!", RED, OFF);
//  scrollTextColor("AND", WHITE, OFF);
//  scrollTextColor("BLUE", BLUE, OFF);
  
  lastLoopTime = millis();
  while(millis() - lastLoopTime < 2500)
  {
    drawUSAFlag();
    refreshDisplay();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 4000)
  {
    animate();
    refreshDisplay();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 3000)
  {
    if(millis() - lastStepTime >= 300) {
      testColorsAnimNextStep();
      lastStepTime = millis();
    }
    refreshDisplay();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 4000)
  {
    if(millis() - lastStepTime >= 20) {
      snakeAnimNextStep();
      lastStepTime = millis();
    }
    refreshDisplay();
  }

  scrollTextColor("ALEJANDRO MUNOZ-MCDONALD", CYAN, OFF);
}

void animate() {
  // If it's been at least <ANIMATION_RATE> ms since we last took a step, take the next step and update <lastStepTime>
  if(millis() - lastStepTime >= ANIMATION_RATE) {
    //snakeAnimNextStep();
    //rowAnimNextStep();
    //singleAnimNextStep();
    fireworkNextStep();
    //radialGrowNextStep();
    //linesNextStep();
    //PongNextStep();
    //rainbowAnimNextStep();
    //testColorsAnimNextStep();
    lastStepTime = millis();
  }
}

void refreshDisplay() {
  // Redraw every row with a total <REFRESH_RATE> μs delay
  int delayBetweenEachRow = REFRESH_RATE / HEIGHT;
  for(int i = 0; i < HEIGHT; i++) {
    displayRow(i);
    delayMicroseconds(delayBetweenEachRow);
  }
  frameCounter++;
}

// @input y: The row to connect to GND
void displayRow(int y) {
  /* Q6:Q1 (QG:QB) on the the 74HC595 chip corresponds to ROW0:ROW5, respectively (or for columns: R0:R5, G0:G5, B0:B5) */
  
  // Turn off all the LEDs by cutting off GND (fixes ghosting issue)
  digitalWrite(ROW_REG[1], LOW);
  shiftOut(ROW_REG[2], ROW_REG[0], LSBFIRST, 0b11111111); // Setting all GND pins to HIGH
  digitalWrite(ROW_REG[1], HIGH);

  // Set the latches to all the 74HC595 chips to LOW since we're about to calculate data
  digitalWrite(RED_REG[1], LOW);
  digitalWrite(GREEN_REG[1], LOW);
  digitalWrite(BLUE_REG[1], LOW);
  digitalWrite(ROW_REG[1], LOW);

  // Calculate which row we want to turn on right now
  int row = 1 << y+1; // Shift a single bit <y>+1 times--where the bit lands corresponds to the row that's about to be lit
  row = ~row; // Negate since GND is active low

  // Calculate which RED, GREEN, and BLUE LEDs should be on in the current row
  int rData = 0, gData = 0, bData = 0;
  for(int x = 0; x < WIDTH; x++) {
    rData |= ((Display[x][y].r > 0) && (frameCounter % (255/Display[x][y].r) == 0)) ? (1 << x+1) : 0;
    gData |= ((Display[x][y].g > 0) && (frameCounter % (255/Display[x][y].g) == 0)) ? (1 << x+1) : 0;
    bData |= ((Display[x][y].b > 0) && (frameCounter % (255/Display[x][y].b) == 0)) ? (1 << x+1) : 0;
//    rData |= ((Display[x][y].r > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].r)) < 0.5)) ? (1 << x+1) : 0;
//    gData |= ((Display[x][y].g > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].g)) < 0.5)) ? (1 << x+1) : 0;
//    bData |= ((Display[x][y].b > 0) && (fmod(frameCounter/1.0, (255/Display[x][y].b)) < 0.5)) ? (1 << x+1) : 0;
  }

  // Shift out the calculated row data to all the 74HC595 chips
  shiftOut(RED_REG[2], RED_REG[0], LSBFIRST, rData);
  shiftOut(GREEN_REG[2], GREEN_REG[0], LSBFIRST, gData);
  shiftOut(BLUE_REG[2], BLUE_REG[0], LSBFIRST, bData);
  shiftOut(ROW_REG[2], ROW_REG[0], LSBFIRST, row);

  // Set the latches to all the 74HC595 chips to HIGH since we're done calculating data and want the changes to take effect
  digitalWrite(RED_REG[1], HIGH);  
  digitalWrite(GREEN_REG[1], HIGH);  
  digitalWrite(BLUE_REG[1], HIGH);
  digitalWrite(ROW_REG[1], HIGH); // Make sure ROW is last to be set since we want all LEDs to stay off until data is ready in RGB chips
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
      if(j % 2 == 0) setLEDRGB(i,j,255,160,160); // If even, color row white
      else setLEDRGB(i,j,255,0,0); // Else odd, color row red
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
      setLEDRGB(x, y, 255, 135, 135);
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

int MAP_0[] = {
  0b000010,
  0b000101,
  0b000101,
  0b000101,
  0b000010,
  0b000000
};
int MAP_1[] = {
  0b000010,
  0b000110,
  0b000010,
  0b000010,
  0b000111,
  0b000000
};
int MAP_2[] = {
  0b000111,
  0b000001,
  0b000111,
  0b000100,
  0b000111,
  0b000000
};
int MAP_3[] = {
  0b000111,
  0b000001,
  0b000111,
  0b000001,
  0b000111,
  0b000000
};
int MAP_4[] = {
  0b000101,
  0b000101,
  0b000111,
  0b000001,
  0b000001,
  0b000000
};
int MAP_5[] = {
  0b000111,
  0b000100,
  0b000111,
  0b000001,
  0b000111,
  0b000000
};
int MAP_6[] = {
  0b000111,
  0b000100,
  0b000111,
  0b000101,
  0b000111,
  0b000000
};
int MAP_7[] = {
  0b000111,
  0b000001,
  0b000010,
  0b000010,
  0b000010,
  0b000000
};
int MAP_8[] = {
  0b000111,
  0b000101,
  0b000111,
  0b000101,
  0b000111,
  0b000000
};
int MAP_9[] = {
  0b000111,
  0b000101,
  0b000111,
  0b000001,
  0b000001,
  0b000000
};
int MAP_A[] = {
  0b000010,
  0b000101,
  0b000111,
  0b000101,
  0b000101,
  0b000000
};
int MAP_B[] = {
  0b000110,
  0b000101,
  0b000110,
  0b000101,
  0b000110,
  0b000000
};
int MAP_C[] = {
  0b000111,
  0b000100,
  0b000100,
  0b000100,
  0b000111,
  0b000000
};
int MAP_D[] = {
  0b000110,
  0b000101,
  0b000101,
  0b000101,
  0b000110,
  0b000000
};
int MAP_E[] = {
  0b000111,
  0b000100,
  0b000111,
  0b000100,
  0b000111,
  0b000000
};
int MAP_F[] = {
  0b000111,
  0b000100,
  0b000111,
  0b000100,
  0b000100,
  0b000000
};
int MAP_G[] = {
  0b000011,
  0b000100,
  0b000111,
  0b000101,
  0b000011,
  0b000000
};
int MAP_H[] = {
  0b000101,
  0b000101,
  0b000111,
  0b000101,
  0b000101,
  0b000000
};
int MAP_I[] = {
  0b000111,
  0b000010,
  0b000010,
  0b000010,
  0b000111,
  0b000000
};
int MAP_J[] = {
  0b000111,
  0b000001,
  0b000001,
  0b000101,
  0b000110,
  0b000000
};
int MAP_K[] = {
  0b000101,
  0b000101,
  0b000110,
  0b000101,
  0b000101,
  0b000000
};
int MAP_L[] = {
  0b000100,
  0b000100,
  0b000100,
  0b000100,
  0b000111,
  0b000000
};
int MAP_M[] = {
  0b000101,
  0b000111,
  0b000111,
  0b000101,
  0b000101,
  0b000000
};
int MAP_N[] = {
  0b000111,
  0b000101,
  0b000101,
  0b000101,
  0b000101,
  0b000000
};
int MAP_O[] = {
  0b000111,
  0b000101,
  0b000101,
  0b000101,
  0b000111,
  0b000000
};
int MAP_P[] = {
  0b000111,
  0b000101,
  0b000111,
  0b000100,
  0b000100,
  0b000000
};
int MAP_Q[] = {
  0b000111,
  0b000101,
  0b000101,
  0b000111,
  0b000001,
  0b000000
};
int MAP_R[] = {
  0b000110,
  0b000101,
  0b000110,
  0b000101,
  0b000101,
  0b000000
};
int MAP_S[] = {
  0b000011,
  0b000100,
  0b000111,
  0b000001,
  0b000110,
  0b000000
};
int MAP_T[] = {
  0b000111,
  0b000010,
  0b000010,
  0b000010,
  0b000010,
  0b000000
};
int MAP_U[] = {
  0b000101,
  0b000101,
  0b000101,
  0b000101,
  0b000111,
  0b000000
};
int MAP_V[] = {
  0b000101,
  0b000101,
  0b000101,
  0b000101,
  0b000010,
  0b000000
};
int MAP_W[] = {
  0b000101,
  0b000101,
  0b000101,
  0b000111,
  0b000101,
  0b000000
};
int MAP_X[] = {
  0b000101,
  0b000101,
  0b000010,
  0b000101,
  0b000101,
  0b000000
};
int MAP_Y[] = {
  0b000101,
  0b000101,
  0b000010,
  0b000010,
  0b000010,
  0b000000
};
int MAP_Z[] = {
  0b000111,
  0b000001,
  0b000010,
  0b000100,
  0b000111,
  0b000000
};
int MAP_SPC[] = {
  0b000000,
  0b000000,
  0b000000,
  0b000000,
  0b000000,
  0b000000
};
int MAP_CMA[] = {
  0b000000,
  0b000000,
  0b000000,
  0b000010,
  0b000110,
  0b000000
};
int MAP_PRD[] = {
  0b000000,
  0b000000,
  0b000000,
  0b000000,
  0b000010,
  0b000000
};
int MAP_FSL[] = {
  0b000000,
  0b000001,
  0b000010,
  0b000100,
  0b000000,
  0b000000
};
int MAP_LCT[] = {
  0b000000,
  0b000010,
  0b000100,
  0b000010,
  0b000000,
  0b000000
};
int MAP_RCT[] = {
  0b000000,
  0b000010,
  0b000001,
  0b000010,
  0b000000,
  0b000000
};
int MAP_QSN[] = {
  0b000110,
  0b000001,
  0b000010,
  0b000000,
  0b000010,
  0b000000
};
int MAP_SEM[] = {
  0b000000,
  0b000010,
  0b000000,
  0b000010,
  0b000110,
  0b000000
};
int MAP_SQT[] = {
  0b000010,
  0b000010,
  0b000000,
  0b000000,
  0b000000,
  0b000000
};
int MAP_CLN[] = {
  0b000000,
  0b000010,
  0b000000,
  0b000000,
  0b000010,
  0b000000
};
int MAP_DQT[] = {
  0b000101,
  0b000101,
  0b000000,
  0b000000,
  0b000000,
  0b000000
};
int MAP_LBT[] = {
  0b000011,
  0b000010,
  0b000010,
  0b000010,
  0b000011,
  0b000000
};
int MAP_RBT[] = {
  0b000110,
  0b000010,
  0b000010,
  0b000010,
  0b000110,
  0b000000
};
int MAP_BSL[] = {
  0b000000,
  0b000100,
  0b000010,
  0b000001,
  0b000000,
  0b000000
};
int MAP_LCB[] = {
  0b000011,
  0b000010,
  0b000100,
  0b000010,
  0b000011,
  0b000000
};
int MAP_RCB[] = {
  0b000110,
  0b000010,
  0b000001,
  0b000010,
  0b000110,
  0b000000
};
int MAP_VLN[] = {
  0b000010,
  0b000010,
  0b000010,
  0b000010,
  0b000010,
  0b000000
};
int MAP_BCT[] = {
  0b000010,
  0b000001,
  0b000000,
  0b000000,
  0b000000,
  0b000000
};
int MAP_TLD[] = {
  0b000000,
  0b000010,
  0b000101,
  0b000000,
  0b000000,
  0b000000
};
int MAP_EXC[] = {
  0b000010,
  0b000010,
  0b000010,
  0b000000,
  0b000010,
  0b000000
};
int MAP_AT[] = {
  0b000110,
  0b000001,
  0b000111,
  0b000101,
  0b000110,
  0b000000
};
int MAP_PND[] = {
  0b000101,
  0b000111,
  0b000101,
  0b000111,
  0b000101,
  0b000000
};
int MAP_PCT[] = {
  0b000010,
  0b000001,
  0b000010,
  0b000100,
  0b000010,
  0b000000
};
int MAP_UCT[] = {
  0b000010,
  0b000101,
  0b000000,
  0b000000,
  0b000000,
  0b000000
};
int MAP_AND[] = {
  0b000010,
  0b000101,
  0b000010,
  0b000101,
  0b000111,
  0b000000
};
int MAP_AST[] = {
  0b000000,
  0b000101,
  0b000010,
  0b000101,
  0b000000,
  0b000000
};
int MAP_LPA[] = {
  0b000011,
  0b000100,
  0b000100,
  0b000100,
  0b000011,
  0b000000
};
int MAP_RPA[] = {
  0b000110,
  0b000001,
  0b000001,
  0b000001,
  0b000110,
  0b000000
};
int MAP_DSH[] = {
  0b000000,
  0b000000,
  0b000111,
  0b000000,
  0b000000,
  0b000000
};
int MAP_EQU[] = {
  0b000000,
  0b000111,
  0b000000,
  0b000111,
  0b000000,
  0b000000
};
int MAP_UND[] = {
  0b000000,
  0b000000,
  0b000000,
  0b000000,
  0b000111,
  0b000000
};
int MAP_PLS[] = {
  0b000000,
  0b000010,
  0b000111,
  0b000010,
  0b000000,
  0b000000
};
int MAP_UKN[] = {
  0b000111,
  0b000111,
  0b000111,
  0b000111,
  0b000111,
  0b000000
};
int* getCharMap(char c)
{
  switch(c)
  {
    case '0':
      return MAP_0;
    case '1':
      return MAP_1;
    case '2':
      return MAP_2;
    case '3':
      return MAP_3;
    case '4':
      return MAP_4;
    case '5':
      return MAP_5;
    case '6':
      return MAP_6;
    case '7':
      return MAP_7;
    case '8':
      return MAP_8;
    case '9':
      return MAP_9;
    case 'A':
      return MAP_A;
    case 'B':
      return MAP_B;
    case 'C':
      return MAP_C;
    case 'D':
      return MAP_D;
    case 'E':
      return MAP_E;
    case 'F':
      return MAP_F;
    case 'G':
      return MAP_G;
    case 'H':
      return MAP_H;
    case 'I':
      return MAP_I;
    case 'J':
      return MAP_J;
    case 'K':
      return MAP_K;
    case 'L':
      return MAP_L;
    case 'M':
      return MAP_M;
    case 'N':
      return MAP_N;
    case 'O':
      return MAP_O;
    case 'P':
      return MAP_P;
    case 'Q':
      return MAP_Q;
    case 'R':
      return MAP_R;
    case 'S':
      return MAP_S;
    case 'T':
      return MAP_T;
    case 'U':
      return MAP_U;
    case 'V':
      return MAP_V;
    case 'W':
      return MAP_W;
    case 'X':
      return MAP_X;
    case 'Y':
      return MAP_Y;
    case 'Z':
      return MAP_Z;
    case ' ':
      return MAP_SPC;
    case ',':
      return MAP_CMA;
    case '.':
      return MAP_PRD;
    case '/':
      return MAP_FSL;
    case '<':
      return MAP_LCT;
    case '>':
      return MAP_RCT;
    case '?':
      return MAP_QSN;
    case ';':
      return MAP_SEM;
    case '\'':
      return MAP_SQT;
    case ':':
      return MAP_CLN;
    case '"':
      return MAP_DQT;
    case '[':
      return MAP_LBT;
    case ']':
      return MAP_RBT;
    case '\\':
      return MAP_BSL;
    case '{':
      return MAP_LCB;
    case '}':
      return MAP_RCB;
    case '|':
      return MAP_VLN;
    case '`':
      return MAP_BCT;
    case '~':
      return MAP_TLD;
    case '!':
      return MAP_EXC;
    case '@':
      return MAP_AT;
    case '#':
      return MAP_PND;
    case '%':
      return MAP_PCT;
    case '^':
      return MAP_UCT;
    case '&':
      return MAP_AND;
    case '*':
      return MAP_AST;
    case '(':
      return MAP_LPA;
    case ')':
      return MAP_RPA;
    case '-':
      return MAP_DSH;
    case '=':
      return MAP_EQU;
    case '_':
      return MAP_UND;
    case '+':
      return MAP_PLS;
    default:
      return MAP_UKN;
  }
}

void scrollTextColor(char text[], int color, int backColor) {
  text = strupr(text); // Ensure all letters are uppercase
  int *currChar = MAP_SPC; // <currChar> initialized to "space" character
  int *nextChar = getCharMap(text[0]); // <nextChar> initialized to first character in <text> string
  int i = 0;
  do {
    setScrollFrame(currChar, nextChar, color, backColor);
    currChar = getCharMap(text[i]);
    nextChar = getCharMap(text[i+1]);
    i++;
  } while(i < strlen(text));
  setScrollFrame(currChar, MAP_SPC, color, backColor);  
  
}

void setScrollFrame(int currMap[], int nextMap[], int color, int backColor) {
  int dispMap[HEIGHT];

  // Start with the <currMap> char shifted to the left 2 times,
  // and the <nextMap> char shifted to the right 2 times.
  // Every iteration of <n>, move the <currMap> char and the <nextMap> char
  // to the left once until the <nextMap> char is right next to the place
  // where the <currMap> char started (i.e., the loop is complete). 
  for(int n = 0; n < 4; n++) {
    for(int i = 0; i < HEIGHT; i++) {
      dispMap[i] = (n < 2) ? ((currMap[i] << n+2) | (nextMap[i] >> 2-n)) : ((currMap[i] << n+2) | (nextMap[i] << n-2));
    }
    setDisplayMapColor(dispMap, color, backColor);
    
    // Use the <SCROLL_TEXT_RATE> delay time to refresh the display after each <dispMap> is set
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
