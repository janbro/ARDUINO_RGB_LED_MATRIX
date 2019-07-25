/*  Predefined colors supported:
||  OFF, RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, CYAN, PINK, WHITE
*/

// Range for REFRESH_RATE: [0, 65535] μs; Range for ANIMATION_RATE and SCROLL_TEXT_RATE: [0, 65535] ms
#define REFRESH_RATE 600 // Microseconds (0 to 0.65535 seconds)
#define ANIMATION_RATE 60 // Milliseconds (0 to 65.535 seconds)
#define SCROLL_TEXT_RATE 225 // Time in milliseconds between text scroll steps

Display disp(REFRESH_RATE); // Initialize custom display object

int currX = 0, currY = 0, currR = 255, currG = 0, currB = 0, currH = 0, currS = 255, currV = 255; // For animations--color starts at red
long lastStepTime = 0, lastLoopTime = 0; // For animations to know when to take the next step/loop

void setup() {
  Serial.begin(9600);
  disp.init();
}

void loop() {
  disp.scrollTextColor("HAPPY 4TH OF JULY!", SCROLL_TEXT_RATE, RED, OFF);
//  disp.scrollTextColor("AND", WHITE, OFF);
//  disp.scrollTextColor("BLUE", BLUE, OFF);
  
  lastLoopTime = millis();
  while(millis() - lastLoopTime < 2500)
  {
    drawUSAFlag();
    disp.refresh();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 4000)
  {
    animate();
    disp.refresh();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 3000)
  {
    if(millis() - lastStepTime >= 300) {
      testColorsAnimNextStep();
      lastStepTime = millis();
    }
    disp.refresh();
  }

  lastLoopTime = millis();
  while(millis() - lastLoopTime < 4000)
  {
    if(millis() - lastStepTime >= 20) {
      snakeAnimNextStep();
      lastStepTime = millis();
    }
    disp.refresh();
  }

  disp.scrollTextColor("ALEJANDRO MUNOZ-MCDONALD", SCROLL_TEXT_RATE, CYAN, OFF);
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
      disp.setRGB(currX,currY,currR,currG,currB);
      //disp.setRGB(currY,currX,currR,currG,currB);
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
      disp.setRGB(WIDTH-currX-1,currY,currR,currG,currB);
      //disp.setRGB(currY,WIDTH-currX-1,currR,currG,currB);
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
    disp.setRGB(i, currY, currR, currG, currB);
    //disp.setRGB(currY, i, currR, currG, currB);
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
    if(i == currY) disp.setRGB(0,i,currR,0,currB);
    else disp.setRGB(0,i,0,0,0);
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
          disp.setRGB(i, j, 0, 0, 0);
        }
      }
      disp.setHSV(f.x, HEIGHT - 1 - f.tick, f.hue, 255, 255);
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
            disp.setHSV(i, j, f.hue, 255, 255);
          } 
          else {
            disp.setRGB(i, j, 0, 0, 0);
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
  int frameCounter = disp.getFrameCount();
  for(int i = 0; i < WIDTH; i++) {
    for(int j = 0; j < HEIGHT; j++) {
      if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= (frameCounter % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > (frameCounter % (HEIGHT+2)) - 2) {
        disp.setRGB(i, j, 255, 0, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frameCounter - 2) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frameCounter - 2) % (HEIGHT+2)) - 2) {
        disp.setRGB(i, j, 0, 255, 0);
      }
      else if(int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) <= ((frameCounter - 4) % (HEIGHT+2)) && int(sqrt(pow(i-center[0],2)+pow(j-center[1],2))) > ((frameCounter - 4) % (HEIGHT+2)) - 2) {
        disp.setRGB(i, j, 0, 0, 255);
      }
      else {
        disp.setRGB(i, j, 0, 0, 0);
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
          disp.setHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i <= (5-counter % 6)) {
          disp.setHSV(i, j, h2, 255, 255);
        }
        else {
          disp.setRGB(i, j, 0, 0, 0);
        }
      }
      else {
        if(j % 2 == 0 && i <= (counter % 6)) {
          disp.setHSV(i, j, h1, 255, 255);
        }
        else if(j % 2 == 1 && i >= (5-counter % 6)) {
          disp.setHSV(i, j, h2, 255, 255);
        }
        else {
          disp.setRGB(i, j, 0, 0, 0);
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
      disp.setRGB(i, j, 0, 0, 0);
    }
  }
  disp.setRGB(pb.x, pb.y, 90, 0, 0);
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
      disp.setHSV(i, j, currH, 255, 255);
    }
  }
  currH += 8; // Allows for 31 unique hues
}

int color[10] = {RED, ORANGE, YELLOW, GREEN, BLUE, PURPLE, CYAN, PINK, WHITE, OFF};
int currIndex = 0;
void testColorsAnimNextStep() {
  if(currIndex == 10) currIndex = 0;
  disp.setAllColor(color[currIndex]);
  currIndex++;
}

void drawUSAFlag() {
  // Color even rows red and odd rows white (starting at row 0)
  for(int j = 0; j < HEIGHT; j++) {
    for(int i = 0; i < WIDTH; i++) {
      if(j % 2 == 0) disp.setRGB(i,j,255,160,160); // If even, color row white
      else disp.setRGB(i,j,255,0,0); // Else odd, color row red
    }
  }
  // Set 3x3 square in top right to blue
  for(int j = 0; j < 3; j++) {
    for(int i = 0; i < 3; i++) {
      disp.setRGB(i,j,0,0,255);
    }
  }
}
