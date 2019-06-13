#include <noise.h>
#include <bitswap.h>
#include <fastspi_types.h>
#include <pixelset.h>
#include <fastled_progmem.h>
#include <led_sysdefs.h>
#include <hsv2rgb.h>
#include <fastled_delay.h>
#include <colorpalettes.h>
#include <color.h>
#include <fastspi_ref.h>
#include <fastspi_bitbang.h>
#include <controller.h>
#include <fastled_config.h>
#include <colorutils.h>
#include <chipsets.h>
#include <pixeltypes.h>
#include <fastspi_dma.h>
#include <fastpin.h>
#include <fastspi_nop.h>
#include <platforms.h>
#include <lib8tion.h>
#include <cpp_compat.h>
#include <fastspi.h>
#include <dmx.h>
#include <power_mgt.h>


#include <FastLED.h>

// Used for graphics
#define WIDTH 6
#define HEIGHT 6

// Range for REFRESH_RATE: [0, 65535] μs; Range for ANIMATION_RATE: [0, 65535] ms
#define REFRESH_RATE 6 // Microseconds (0 to 0.65535 seconds)
#define ANIMATION_RATE 300 // Milliseconds (0 to 65.535 seconds)

// Shift registers. Each control parallel r/g/b values for each column
int RED[3] = {5,6,7}; // CLK, Latch, Data
int GREEN[3] = {8,9,10}; // CLK, Latch, Data
int BLUE[3] = {11,12,13}; // CLK, Latch, Data

int ROW[3] = {2,3,4}; // CLK, Latch, Data

int currX = 0, currY = 0, currR = 255, currG = 0, currB = 0, currH = 0, currS = 100, currV = 100; // For animations--color starts at red
long lastStepTime = 0; // For animations to know when to take the next step
int frame_counter = 0;

String inString = "";

struct ccRGB {
  byte r;
  byte g;
  byte b;
};

ccRGB Display[WIDTH][HEIGHT] = {};

void setup() {
//  Serial.begin(9600);
  for(int i=0; i<3; i++) {
    pinMode(RED[i], OUTPUT);
    pinMode(GREEN[i], OUTPUT);
    pinMode(BLUE[i], OUTPUT);
    pinMode(ROW[i], OUTPUT);
  }

  drawUSAFlag();
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
  animate();

  // Redraw the display
  refreshDisplay();
  frame_counter++;
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
    rainbowAnimNextStep();
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
  // Set the latches to all the 74HC595 chips to LOW since we're about to calculate data
  digitalWrite(RED[1], LOW);
  digitalWrite(GREEN[1], LOW);
  digitalWrite(BLUE[1], LOW);
  digitalWrite(ROW[1], LOW);

  // Calculate which RED, GREEN, and BLUE LEDs should be on in the current row
  int rData = 0, gData = 0, bData = 0;
  for(int x = 0; x < WIDTH; x++) {
    rData += ((Display[x][y].r > 0) && (frame_counter % (255/Display[x][y].r) == 0)) ? (1 << x+1) : 0;
    gData += ((Display[x][y].g > 0) && (frame_counter % (255/Display[x][y].g) == 0)) ? (1 << x+1) : 0;
    bData += ((Display[x][y].b > 0) && (frame_counter % (255/Display[x][y].b) == 0)) ? (1 << x+1) : 0;
  }

  // Calculate which row we want to turn on right now
  int row = 1 << y+1; // Shift a single bit <y>+1 times--where the bit lands corresponds to the row that's about to be lit
  row = ~row; // Negate since GND is active low

  // Shift out the calculated row data to all the 74HC595 chips
  shiftOut(RED[2], RED[0], LSBFIRST, rData);
  shiftOut(GREEN[2], GREEN[0], LSBFIRST, gData);
  shiftOut(BLUE[2], BLUE[0], LSBFIRST, bData);
  shiftOut(ROW[2], ROW[0], LSBFIRST, row);

  // Set the latches to all the 74HC595 chips to HIGH since we're done calculating data and want the changes to take effect
  digitalWrite(RED[1], HIGH);
  digitalWrite(GREEN[1], HIGH);
  digitalWrite(BLUE[1], HIGH);
  digitalWrite(ROW[1], HIGH);
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
      fastLEDHSV(f.x, HEIGHT - 1 - f.tick, f.hue, 255, 255);
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
            fastLEDHSV(i, j, f.hue, 255, 255);
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
      if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= (frame_counter % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > (frame_counter % (HEIGHT+2)) - 2) {
        setLEDRGB(i, j, 255, 0, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frame_counter - 2) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frame_counter - 2) % (HEIGHT+2)) - 2) {
        setLEDRGB(i, j, 0, 255, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frame_counter - 4) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frame_counter - 4) % (HEIGHT+2)) - 2) {
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
          fastLEDHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i <= (5-counter % 6)) {
          fastLEDHSV(i, j, h2, 255, 255);
        }
        else {
          setLEDRGB(i, j, 0, 0, 0);
        }
      }
      else {
        if(j % 2 == 0 && i <= (counter % 6)) {
          fastLEDHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i >= (5-counter % 6)) {
          fastLEDHSV(i, j, h2, 255, 255);
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

void rainbowAnimNextStep()
{
  if(currH == 360) currH = 0;
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      //fastLEDHSV(i, j, currH, 255, 255);
      setLEDHSV(i, j, currH, 1.0, 1.0);
    }
  }
  currH += 15;
}

void setAllOff() {
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      setLEDRGB(i,j,0,0,0);
    }
  }
}

void setAllWhite() {
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      setLEDRGB(i,j,255,255,255);
    }
  }
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

void setLEDRGB(int x, int y, byte r, byte g, byte b) {
  ccRGB color = { r, g, b };
  Display[x][y] = color;
}

void setLEDHSV(int x, int y, int h, float s, float v) // HSV ranges: [0, 359], [0.00, 1.00], [0.00, 1.00]
{
  /* Formula used (Alternative): https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB */
  
  float r, g, b;

  // Convert HSV to RGB using Alternative Wikipedia formula
  r = v - (v * s * max(min(fmod(5.0+(h/60.0), 6.0), min(4.0 - (fmod(5.0+(h/60.0), 6.0)), 1.0)), 0.0));
  g = v - (v * s * max(min(fmod(3.0+(h/60.0), 6.0), min(4.0 - (fmod(3.0+(h/60.0), 6.0)), 1.0)), 0.0));
  b = v - (v * s * max(min(fmod(1.0+(h/60.0), 6.0), min(4.0 - (fmod(1.0+(h/60.0), 6.0)), 1.0)), 0.0));

  setLEDRGB(x, y, r * 255, g * 255, b * 255); // Multiply RGB by 255 to convert to byte values
}

void fastLEDHSV(int x, int y, byte h, byte s, byte v) {
  CHSV hsv(h, s, v);
  CRGB rgb;
  hsv2rgb_spectrum(hsv, rgb);
//  Serial.print(r);
//  Serial.print(", ");
//  Serial.print(g);
//  Serial.print(", ");
//  Serial.print(b);
//  Serial.println();
  setLEDRGB(x, y, rgb.red, rgb.green, rgb.blue);
}
