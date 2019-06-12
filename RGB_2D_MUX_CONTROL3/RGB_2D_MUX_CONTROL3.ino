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
#include <FastLED.h>
#include <dmx.h>
#include <power_mgt.h>


#include <FastLED.h>

// Used for graphics
#define WIDTH 6
#define HEIGHT 6

#define REFRESH_RATE 500 // Microseconds
#define ANIMATION_RATE 200 //Milliseconds

// Shift registers. Each control parallel r/g/b values for each column
int RED[3] = {5,6,7}; // CLK, Latch, Data
int GREEN[3] = {8,9,10}; // CLK, Latch, Data
int BLUE[3] = {11,12,13}; // CLK, Latch, Data

int ROW[3] = {2,3,4}; // CLK, Latch, Data

int currX = 0, currY = 0, currR = 255, currG = 0, currB = 0; // For animations--color starts at red
long lastStepTime = 0; // For animations to know when to take the next step
int frame_counter=0;

int gnd_ctrl_sr = 0;
String inString = "";

struct ccRGB {
  byte r;
  byte g;
  byte b;
};

ccRGB Display[HEIGHT][WIDTH] = {};

void setup() {
  for(int i=0; i<3; i++) {
    pinMode(RED[i], OUTPUT);
    pinMode(GREEN[i], OUTPUT);
    pinMode(BLUE[i], OUTPUT);
    pinMode(ROW[i], OUTPUT);
  }

  drawUSFlag();

  
//  for(int i=0;i<6;i++) {
//    setLEDRGB(i,0,255,255,255);
//    setLEDRGB(i,1,255,255,255);
//    setLEDRGB(i,2,255,255,255);
//    setLEDRGB(i,3,255,255,255);
//    setLEDRGB(i,4,255,255,255);
//    setLEDRGB(i,5,255,255,255);
//    
//  }
//  setLEDRGB(5,5,0,255,0);
}

void loop() {
  // Calculate how the display should look right now (value passed in is the delay in ms between each animation step)
  animate(ANIMATION_RATE);
  
//  if (Serial.available() > 0) {
//    // read the incoming byte:
//    int inChar = Serial.read();
//    if (isDigit(inChar)) {
//      // convert the incoming byte to a char and add it to the string:
//      inString += (char)inChar;
//    }
//    // if you get a newline, print the string, then the string's value:
//    if (inChar == '\n') {
//      Serial.print("Value:");
//      Serial.println(inString.toInt());
//      gnd_ctrl_sr = inString.toInt();
//      Serial.print("String: ");
//      Serial.println(inString);
//      // clear the string for new input:
//      inString = "";
//
//    }
//  }

  // Redraw the display
  refreshDisplay();
  frame_counter++;
}

void animate(int t)
{
  // If it's been <t> ms since we last took a step, take the next step and update <lastStepTime>
  if(millis() - lastStepTime >= t)
  {
    //snakeAnimNextStep();
    //rowAnimNextStep();
    //singleAnimNextStep();
    fireworkNextStep();
    //radialGrowNextStep();
    //linesNextStep();
    //PongNextStep();
    //rainbowAnimNextStep();
    lastStepTime = millis();
  }
}

void setLEDRGB(int x, int y, byte r, byte g, byte b) {
  ccRGB color = { r, g, b };
  Display[y][x] = color;
}

// @input y: The row to connect to GND
void displayRow(int y) {
  
  //SR GND Control
  int row = 1 << y;
  row = ~row;
  
  // Turn off all LEDs when setting color control signal
  int off = 63;
  // setlatch pin low so the LEDs don't change while sending in bits
  digitalWrite(ROW[1], LOW);
  // shift out the bits of dataToSend to the 74HC595
  shiftOut(ROW[2], ROW[0], LSBFIRST, off);
  //set latch pin high- this sends data to outputs so the LEDs will light up
  digitalWrite(ROW[1], HIGH);

  int redData = 0;
  int greenData = 0;
  int blueData = 0;

  // Calculate data to send to each sr for respective color
  for(int x=0;x<WIDTH;x++) {
    // Shift bits 2 to the left because using MSB of 
    if(Display[y][x].r > 127) redData += Display[y][x].r > 0 && frame_counter % round(255.0/(Display[y][x].r-127)) != 0  ? 1 << x : 0;
    else redData += Display[y][x].r > 0 && frame_counter % round(255.0/Display[y][x].r) == 0  ? 1 << x : 0;
    if(Display[y][x].g > 127) greenData += Display[y][x].g > 0 && frame_counter % round(255.0/(Display[y][x].g-127)) != 0  ? 1 << x : 0;
    else greenData += Display[y][x].g > 0 && frame_counter % round(255.0/Display[y][x].g) == 0  ? 1 << x : 0;
    if(Display[y][x].b > 127) redData += Display[y][x].b > 0 && frame_counter % round(255.0/(Display[y][x].b-127)) != 0  ? 1 << x : 0;
    else blueData += Display[y][x].b > 0 && frame_counter % round(255.0/Display[y][x].b) == 0  ? 1 << x : 0;
    
//    greenData += Display[y][x].g > 0 && frame_counter % (255/Display[y][x].g) == 0 ? 1 << x : 0;
//    blueData += Display[y][x].b > 0 && frame_counter % (255/Display[y][x].b) == 0  ? 1 << x : 0;
  }
  
  // setlatch pin low so the LEDs don't change while sending in bits
  digitalWrite(RED[1], LOW);
  // shift out the bits of dataToSend to the 74HC595
  shiftOut(RED[2], RED[0], MSBFIRST, redData);
  //set latch pin high- this sends data to outputs so the LEDs will light up
  digitalWrite(RED[1], HIGH);
  
  // setlatch pin low so the LEDs don't change while sending in bits
  digitalWrite(GREEN[1], LOW);
  // shift out the bits of dataToSend to the 74HC595
  shiftOut(GREEN[2], GREEN[0], MSBFIRST, greenData);
  //set latch pin high- this sends data to outputs so the LEDs will light up
  digitalWrite(GREEN[1], HIGH);

  // setlatch pin low so the LEDs don't change while sending in bits
  digitalWrite(BLUE[1], LOW);
  // shift out the bits of dataToSend to the 74HC595
  shiftOut(BLUE[2], BLUE[0], MSBFIRST, blueData);
  //set latch pin high- this sends data to outputs so the LEDs will light up
  digitalWrite(BLUE[1], HIGH);

  // Set row on
  // setlatch pin low so the LEDs don't change while sending in bits
  digitalWrite(ROW[1], LOW);
  // shift out the bits of dataToSend to the 74HC595
  shiftOut(ROW[2], ROW[0], LSBFIRST, row);
  //set latch pin high- this sends data to outputs so the LEDs will light up
  digitalWrite(ROW[1], HIGH);
}

void setLEDHSV(int x, int y, int h, float s, float v)
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
  CHSV hsv(h, s, v); // pure blue in HSV Rainbow space
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

void singleAnimNextStep() {
  if(currY == HEIGHT)
  {
    currY = 0;
    currR = (currR == 255) ? 0 : 255;
    currB = (currB == 255) ? 0 : 255;
  }
  for(int i = 0; i < HEIGHT; i++)
  {
    if(i == currY) setLEDRGB(0,i,currR,0,currB);
    else setLEDRGB(0,i,0,0,0);
  }
  currY++;
}

void rowAnimNextStep() {
  // If the currY is 6 (the height), then set it to 0 and change colors
  if(currY == HEIGHT)
  {
    currY = 0;
    if(currR > 0)
    {
      currR = 0;
      currG = 255;
    }
    else if(currG > 0)
    {
      currG = 0;
      currB = 255;
    }
    else
    {
      currB = 0;
      currR = 255;
    }
  }
  // Set all LEDs in the current row (<currY>) to the current color before incrementing <currY>
  for(int i = 0; i < WIDTH; i++)
  {
    setLEDRGB(i, currY, currR, currG, currB);
    //setLEDRGB(currY, i, currR, currG, currB);
  }
  currY++;
}

int currH;

void rainbowAnimNextStep()
{
  if(currH == 255) currH = 0;
  for(int i = 0; i < WIDTH; i++)
  {
    for(int j = 0; j < HEIGHT; j++)
    {
      fastLEDHSV(i, j, currH, 255, 255);
    }
  }
  currH ++;
}

void snakeAnimNextStep() {
  /* DISCLAIMER: This is inefficient AF */

  // If the currY is 6 (the height), then set it to 0 and change colors
  if(currY == HEIGHT)
  {
    currY = 0;
    if(currR > 0)
    {
      currR = 0;
      currG = 255;
    }
    else if(currG > 0)
    {
      currG = 0;
      currB = 255;
    }
    else
    {
      currB = 0;
      currR = 255;
    }
  }

  // If <currY> is even, then animate from left to right before incrementing <currY>
  if(currY % 2 == 0)
  {
    if(currX < WIDTH) 
    {
      setLEDRGB(currX,currY,currR,currG,currB);
      //setLEDRGB(currY,currX,currR,currG,currB);
      currX++;
    }
    else
    {
      currX = 0;
      currY++;
    }
  }

  // Else <currY> is odd, animate from right to left before incrementing <currY>
  else
  {
    if(currX < WIDTH) 
    {
      setLEDRGB(WIDTH-currX-1,currY,currR,currG,currB);
      //setLEDRGB(currY,WIDTH-currX-1,currR,currG,currB);
      currX++;
    }
    else
    {
      currX = 0;
      currY++;
    }
  }
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
          } else {
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
  } else if(pb.x <= 0) {
    pb.x_dir = -pb.x_dir;
  }
  if(pb.y >= HEIGHT - 1) {
    pb.y_dir = -pb.y_dir;
  } else if(pb.y <= 0) {
    pb.y_dir = -pb.y_dir;
  }
}

void drawUSFlag()
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      setLEDRGB(i,j,0,0,255);
    }
  }

  for(int i = 0; i < 6; i++)
  {
    for(int j = 0; j < 6; j++)
    {
      if(i < 3 && j < 3)
      {
        continue;
      }
      if(j % 2 == 0)
      {
        setLEDRGB(i,j,255,255,255);
      }
      else
      {
        setLEDRGB(i,j,255,0,0);
      }
    }
  }
}

void refreshDisplay() {
  // Redraw every row with a 1.25 ms delay (compromise between brightness and jitteryness)
  for(int i = 0; i < HEIGHT; i++) {
    displayRow(i);
    delayMicroseconds(REFRESH_RATE);
  }
}
