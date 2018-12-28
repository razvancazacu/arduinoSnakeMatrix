# SSnake

Arduino parts:

 * Arduino UNO
 * 16x2 LCD
 * 8x8 LED Matrix Display
 * Joysyick
 * Wires
 * BreadBoard x2
 * Resistor 220 Ohm, 10k Ohm
 
# How to play?

  The game starts when receiving an input from the joystick and starts the movement of the snake in the choosen direction. Goal is to eat fruits and get a high score. The game ends when the snake eats himself.


Score and Difficulty:
  - The score is calculated by basescore(100)*multiplier(x1 at lvl 1) + old score.
  - The multiplier is increased by 1 every 8 fruits eaten.
  - The difficulty is increased by making the snake move faster using the same rule as the multiplier
  - In case of a new HIGHSCORE beeing reached, the score is written into the EEPROM memory of the arduino.
 
# Quick rundown:
 * At the start of the game we are greeted by the LCD screen with a message and a music played ( in this case is mario theme song).
 * After the joystick recived a "direction", the game starts by setting the value of a variable "flagGameOn" to true, and so the music stops and the loop for snake movement and fruit spawning begins. 
 * The snake is keept into 2 vectors that represent the x and y coordinate for every snake part, and the coordinate for the fruit are keept into 2 variables.
 * When the head of the snake reached the same coordinates as the fruit, the snake lenght is increased, the fruit respawns and in case of 8 fruits collected, difficulty is increased.
 * At every step the program checks if the snake coordinates intersect and in that case it is a GAME OVER.
 Here is a quick example of the game ( sorry for the upside video ) : 
https://youtu.be/WhzQ61-wnv4
