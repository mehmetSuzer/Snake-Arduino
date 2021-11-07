

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1 // Screen will be refreshed when Arduino is reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Analog Pins where we read data coming from the joystick
#define xAxis A0
#define yAxis A1

// Directions
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

// A structure for the snake
typedef struct {
  byte *xValues;
  byte *yValues;
  short int len;
  byte width;
  byte dir;
  bool alive;
} Snake;

// A function to initialize snake
// Head of the snake is represented by LAST elements of xValues and yValues
Snake *init_snake(void) {
  Snake *snake = malloc(sizeof(Snake));
  snake->xValues = malloc(1);
  snake->yValues = malloc(1);
  snake->xValues[0] = 64;
  snake->yValues[0] = 32;
  snake->len = 1;
  snake->width = 4;
  snake->dir = LEFT;
  snake->alive = true;
  return snake;
}

// A structure for food
typedef struct {
  byte x;
  byte y;
  byte width;
  bool eaten;
} Food;

// Global Variables
Snake *snake = init_snake();
short int xValue, yValue, score = 0;
byte xHead, yHead;
Food food;

void setup() {
  // In case Arduino doesn't start the OLED display, we check whether the screen is initialized or not
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    for(;;); // Loop forever

  // Clear display and set the text size and color
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X scale text
  display.setTextColor(WHITE);

  // Write SNAKE on the screen
  display.setCursor(35, 20);
  display.print(F("SNAKE"));
  display.display();
  delay(1500);
  display.clearDisplay();

  // set the seed and initialize the first food
  randomSeed(analogRead(A0));
  food = {random(2, 30)*4, random(2, 14)*4, 4, false};
}

void loop() {
 // If snake hits, it dies and the game ends
  // Show the score which is the number of food eaten, then wait
  // To play the game again, reset your Arduino
  if (!snake->alive) {
    display.clearDisplay();
    display.setCursor(10, 20);
    display.print(F("SCORE: "));
    display.print(score);
    display.display();
    while (1);
  }

  // Recreate the food if it is eaten
  if (food.eaten) {
    food.x = random(2, 30)*4;
    food.y = random(2, 14)*4;
    food.eaten = false;
  }

  // Read the analog data for x and y coordinates and turn the snake if necessary
  xValue = analogRead(xAxis);
  yValue = analogRead(yAxis);
  if (xValue < 400) turn_left();
  else if (xValue > 600) turn_right();
  else if (yValue < 400) turn_up();
  else if (yValue > 600) turn_down();

  // If there is food in front of the snake, eat and set the food as eaten to recreate it 
  // Else, move and check for collusions
  xHead = snake->xValues[snake->len-1];
  yHead = snake->yValues[snake->len-1];
  if ((snake->dir == LEFT && food.x == xHead-4 && food.y == yHead) || 
      (snake->dir == RIGHT && food.x == xHead+4 && food.y == yHead) || 
      (snake->dir == UP && food.x == xHead && food.y == yHead-4) ||
      (snake->dir == DOWN && food.x == xHead && food.y == yHead+4)) {
    eat();
    food.eaten = true;
    score++;
  }
  else {
    move_snake();
    hit();
  }
   // Draw the table and wait 100 ms (~10 FPS)
  draw_table();
  delay(100);
}

void move_snake(void) {
  // Every segment of the snake comes at the position of its predecessor except the head of the snake
  for (int i = 0; i < snake->len-1; i++) {
    snake->xValues[i] = snake->xValues[i+1];
    snake->yValues[i] = snake->yValues[i+1];
  }

  // move the head of the snake in the direction it looks
  switch (snake->dir) {
    case LEFT:
      snake->xValues[snake->len-1] -= snake->width;
      break;
    case RIGHT:
      snake->xValues[snake->len-1] += snake->width;
      break;
    case UP:
      snake->yValues[snake->len-1] -= snake->width;
      break;
    case DOWN:
      snake->yValues[snake->len-1] += snake->width;
      break;
  }
}

// Functions to turn the snake
void turn_left(void) {
  // Turn left if the direction is up or down
  if (snake->dir == UP || snake->dir == DOWN)
    snake->dir = LEFT;
}

void turn_right(void) {
  // Turn right if the direction is up or down
  if (snake->dir == UP || snake->dir == DOWN)
    snake->dir = RIGHT;
}

void turn_up(void) {
  // Turn up if the direction is left or right
  if (snake->dir == LEFT || snake->dir == RIGHT)
    snake->dir = UP;
}

void turn_down(void) {
  // Turn down if the direction is left or right
  if (snake->dir == LEFT || snake->dir == RIGHT)
    snake->dir = DOWN;
}

void draw_table(void) {
  display.clearDisplay();

 // Draw the snake
 for (int i = 0; i < snake->len; i++) 
   display.drawRect(snake->xValues[i], snake->yValues[i], snake->width, snake->width, WHITE);
 
  // Draw food
  display.fillRect(food.x, food.y, food.width, food.width, WHITE);

  //Draw walls
  for (int i = 0; i < SCREEN_HEIGHT/4; i++) {
    display.fillRect(0, i*4, 4, 4, WHITE);
    display.fillRect(SCREEN_WIDTH-4, i*4, 4, 4, WHITE);
  }
  for (int i = 0; i < SCREEN_WIDTH/4; i++) {
    display.fillRect(i*4, 0, 4, 4, WHITE);
    display.fillRect(i*4, SCREEN_HEIGHT-4, 4, 4, WHITE);
  }
  
  display.display();
}

void eat(void) {
  // Determine the next position of the head of the snake
  byte x, y;
  byte xHead = snake->xValues[snake->len-1];
  byte yHead = snake->yValues[snake->len-1];
  switch (snake->dir) {
  case LEFT:
    x = xHead-snake->width;
    y = yHead;
    break;
  case RIGHT: 
    x = xHead+snake->width;
    y = yHead;
    break;
  case UP:
    x = xHead;
    y = yHead-snake->width;
    break;
  case DOWN:
    x = xHead;
    y = yHead+snake->width;
    break;
  }

  snake->xValues = realloc(snake->xValues, snake->len+1);
  snake->xValues[snake->len] = x;
  snake->yValues = realloc(snake->yValues, snake->len+1);
  snake->yValues[snake->len] = y;
  snake->len++;
}

void hit(void) {
  // Get coordinates of the head
  byte xHead = snake->xValues[snake->len-1];
  byte yHead = snake->yValues[snake->len-1];
  
  // Check collusions with walls
  if (xHead < 4 || xHead > SCREEN_WIDTH-8 || yHead < 4 || yHead > SCREEN_HEIGHT-8) {
    snake->alive = false;
    return;
  }

  // Check collusions with itself
  for (int i = 0; i < snake->len-1; i++) {
    if (snake->xValues[i] == xHead && snake->yValues[i] == yHead) {
      snake->alive = false;
      return;
    }
  }
}
