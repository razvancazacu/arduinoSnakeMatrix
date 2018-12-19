#include "LedControl.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

#define JOY_X A0
#define JOY_Y A1
#define LCD_PIN 9
#define JOY_BUTTON A3
#define BASE_SCORE 100
#define TOP 0
#define RIGHT 1
#define BOTTOM 2
#define LEFT 3
#define MAX_SNAKE_LENGTH 16
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_BREAK 0
#define MATRIX_ROW_COL 8
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
LedControl lc = LedControl(12, 11, 10, 1);

// Variables
int scoreMultiplier = 1;
int snakeDirection = RIGHT;   // snakeDirection of movement
int snakeX[MAX_SNAKE_LENGTH]; // X-coordinates of snake
int snakeY[MAX_SNAKE_LENGTH]; // Y-coordinates of snake
int snakeLength = 3;
int scoreCurrent = 3;
int scoreTime = 0;
int scoreHigh;
int fruitX, fruitY;
unsigned long prevTime = 0;    // for gamedelay (ms)
unsigned long delayTime = 400 ; // Game step in ms
unsigned long fruitPrevTime = 0;
unsigned long fruitBlinkTime = 100;
unsigned long buttonCheckTime = 0;
unsigned long delayButtonCheck = 0;
int fruitLed = true;
bool flagGameOn = false;
const int joyStickValueCheck = 52;

int melody[] = {
  NOTE_E7, NOTE_E7, NOTE_BREAK, NOTE_E7,
  NOTE_BREAK, NOTE_C7, NOTE_E7, NOTE_BREAK,
  NOTE_G7, NOTE_BREAK, NOTE_BREAK,  NOTE_BREAK,
  NOTE_G6, NOTE_BREAK, NOTE_BREAK, NOTE_BREAK,

  NOTE_C7, NOTE_BREAK, NOTE_BREAK, NOTE_G6,
  NOTE_BREAK, NOTE_BREAK, NOTE_E6, NOTE_BREAK,
  NOTE_BREAK, NOTE_A6, NOTE_BREAK, NOTE_B6,
  NOTE_BREAK, NOTE_AS6, NOTE_A6, NOTE_BREAK,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, NOTE_BREAK, NOTE_F7, NOTE_G7,
  NOTE_BREAK, NOTE_E7, NOTE_BREAK, NOTE_C7,
  NOTE_D7, NOTE_B6, NOTE_BREAK, NOTE_BREAK
};

int noteDurations[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 15);
  lc.clearDisplay(0);
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);

  pinMode(LCD_PIN, OUTPUT);
  analogWrite(LCD_PIN, 90);

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BUTTON, INPUT);

  gameStartIntro();
}

void gameStartIntro() { //Reset game data for menu and a new game
  scoreMultiplier = 1;
  scoreTime = 0;
  flagGameOn = false;
  scoreCurrent = 3;
  snakeLength = 3;
  snakeDirection = RIGHT;
  delayTime = 500;
  snakeX[0] = 0;
  snakeY[0] = 4;
  for (int i = 1; i < MAX_SNAKE_LENGTH; i++) {
    snakeX[i] = snakeY[i] = -1;
  }
  makeFruit();
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("~SSNAKE~");
  lcd.setCursor(1, 1);
  lcd.print("~Move to start");
}

void initializeGame() { //Reset game data for a new game
  lcd.clear();
  flagGameOn = true;
  lcd.setCursor(0, 0);
  lcd.print("SCORE 0");
  lcd.setCursor(0, 1);
  lcd.print("LEVEL 1");
  lcd.setCursor(11, 0);
  lcd.print("HIGH");
  lcd.setCursor(10, 1);
  EEPROM_readAnything(0, scoreHigh);
  lcd.print(scoreHigh);
}

void loop() {
  if (flagGameOn == false) {
    playSong();
    if ((treatValue(analogRead(JOY_X)) != joyStickValueCheck) || (treatValue(analogRead(JOY_X)) != joyStickValueCheck)) { // Start a new game on joystick button press
      initializeGame();
    }
  }
  if (flagGameOn == true) { // Condition for startin a new game

    unsigned long currentTimeButtonCheck = millis();
    if ((currentTimeButtonCheck - buttonCheckTime) >= delayButtonCheck) {
      checkButtons(); // First checking if there is a button pressed for changing the snakeDirection
      buttonCheckTime = currentTimeButtonCheck;
    }
    unsigned long currentTime = millis();
    if ((currentTime - prevTime) >= delayTime) { // After an amount of time the snake is moving
      nextStep();
      prevTime = currentTime;
    }
    draw(); // After the checks are done in "nextStep", "draw" is printing the data on the matrix;
  }
}

int playSong() {
  int size = sizeof(melody) / sizeof(int);
  for (int thisNote = 0; thisNote < size; thisNote++) {
    if ((treatValue(analogRead(JOY_X)) != joyStickValueCheck) || (treatValue(analogRead(JOY_X)) != joyStickValueCheck)) { // Start a new game on joystick button press
      initializeGame();
      return 0;
    }
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(13, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(13);
  }
}

int treatValue(int data) { // A mapping for the values of the joystick
  return (data * 9 / 1024) + 48;
}

void checkButtons() { // Direction given by the player with the joystick
  if (((snakeX[0] >= 0) && (snakeX[0] <= 7))) {
    if (((snakeY[0] >= 0) && (snakeY[0] <= 7))) {
      int tempValX = treatValue(analogRead(JOY_X)); // Possible game bug from sending commands before the snake is fully drawn.
      int tempValY = treatValue(analogRead(JOY_Y));
      if ((tempValX != joyStickValueCheck) || (tempValY != joyStickValueCheck)) {
        if (tempValY < joyStickValueCheck) {
          if (snakeDirection != BOTTOM) {
            snakeDirection = TOP;
          }
        } else if (tempValY > joyStickValueCheck) {
          if (snakeDirection != TOP) {
            snakeDirection = BOTTOM;
          }
        } else if (tempValX < joyStickValueCheck) {
          if (snakeDirection != RIGHT) {
            snakeDirection = LEFT;
          }
        } else if (tempValX > joyStickValueCheck) {
          if (snakeDirection != LEFT) {
            snakeDirection = RIGHT;
          }
        }
      }
    }
  }
}
void draw() {
  lc.clearDisplay(0);
  drawSnake();
  drawFruit();
}

void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    lc.setLed(0, snakeX[i], snakeY[i], true);
  }
}

void drawFruit() {
  if (fruitInTable(fruitX, fruitY)) {
    unsigned long currentTime = millis();
    if ((currentTime - fruitPrevTime) >= fruitBlinkTime) { // Blinking fruit
      fruitLed = (fruitLed == true) ? false : true;
      fruitPrevTime = currentTime;
    }
    lc.setLed(0, fruitX, fruitY, fruitLed);
  }
}

bool fruitInTable(int x, int y) {
  return (x >= 0) && (x <= 7) && (y >= 0) && (y <= 7);
}

void nextStep() {
  for (int i = snakeLength - 1; i > 0; i--) { //Checking for cases in which snake it's passing from side to side
    if ((snakeDirection == RIGHT) && (snakeX[0] - snakeLength == 7)) {
      snakeX[0] = -1;
    } else if ((snakeDirection == LEFT) && (snakeX[0] + snakeLength == 0)) {
      snakeX[0] = 8;
    } else {
      snakeX[i] = snakeX[i - 1];
    }
    if ((snakeDirection == TOP) && (snakeY[0] + snakeLength == 0)) {
      snakeY[0] = 8;
    } else if ((snakeDirection == BOTTOM) && (snakeY[0] - snakeLength == 7)) {
      snakeY[0] = -1;
    } else {
      snakeY[i] = snakeY[i - 1];
    }
  }
  if (snakeDirection == TOP) {  // "head" of the snake is advancing"
    snakeY[0] = snakeY[0] - 1;
  } else if (snakeDirection == RIGHT) {
    snakeX[0] = snakeX[0] + 1;
  } else if (snakeDirection == BOTTOM) {
    snakeY[0] = snakeY[0] + 1;
  } else if (snakeDirection == LEFT) {
    snakeX[0] = snakeX[0] - 1;
  }
  if ((snakeX[0] == fruitX) && (snakeY[0] == fruitY)) { // "head" it's on the same tile as the fruit
    snakeLength++;
    scoreCurrent++;
    makeFruit();
    scoreTime = scoreTime + BASE_SCORE * scoreMultiplier;
    lcd.setCursor(0, 0);
    lcd.print("SCORE");
    lcd.setCursor(6, 0);
    lcd.print(scoreTime);
    lcd.setCursor(0, 1);
    lcd.print("LEVEL");
    if (scoreCurrent % 5 == 0) { // After reaching length 8, reset to 3, increase difficulty, bigger score multiplier
      snakeLength = 3;
      delayTime = delayTime - 30;
      scoreMultiplier++;
      lcd.setCursor(6, 1);
      lcd.print(scoreMultiplier);
    }
  }
  checkSnakeSuicide(); // After the movement, checking if snake ate himself ( GAMEOVER )
}

void makeFruit() {
  int x, y;
  x = random(0, MATRIX_ROW_COL);
  y = random(0, MATRIX_ROW_COL);
  while (isPartOfSnake(x, y)) { // Check to not spawn on the snake
    x = random(0, MATRIX_ROW_COL);
    y = random(0, MATRIX_ROW_COL);
  }
  fruitX = x;
  fruitY = y;
}

boolean isPartOfSnake(int x, int y) {
  for (int i = 0; i < snakeLength - 1; i++) {
    if ((x == snakeX[i]) && (y == snakeY[i])) {
      return true;
    }
  }
  return false;
}

void checkSnakeSuicide() {
  for (int i = 1; i < snakeLength; i++) {
    if ((snakeX[0] == snakeX[i]) && (snakeY[0] == snakeY[i])) {
      gameOver();
    }
  }
}

const PROGMEM bool gameOverMessage[MATRIX_ROW_COL][90] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void gameOver() { // Display "GAME OVER" message.
  lc.clearDisplay(0);
  for (int d = 0; d < sizeof(gameOverMessage[0]) - 7; d++) {
    for (int col = 0; col < MATRIX_ROW_COL; col++) {
      delay(3);
      for (int row = 0; row < MATRIX_ROW_COL; row++) {
        // this reads the byte from the PROGMEM and displays it on the screen
        lc.setLed(0, col, row, pgm_read_byte(&(gameOverMessage[row][col + d])));
      }
    }
  }
  if (scoreTime > scoreHigh) {  // Save if HighScore was achived
    EEPROM_writeAnything(0, scoreTime);
  }
  gameStartIntro(); // Reset to INTRO
}
