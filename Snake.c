#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LCD.h"

#define LCD_WIDTH 14
#define LCD_HEIGHT 6
#define INITIAL_SNAKE_LENGTH 3
#define DELAY_EASY 300
#define DELAY_MEDIUM 225
#define DELAY_HARD 150
#define EASY_SCORE 10
#define MEDIUM_SCORE 15
#define HARD_SCORE 20

typedef int bool;
#define true 1
#define false 0

typedef struct coords {
	int x;
	int y;
} Coords;

typedef struct direction {
	bool UP;
	bool DOWN;
	bool RIGHT;
	bool LEFT;
} Direction;

typedef struct difficulty {
	bool EASY;
	bool MEDIUM;
	bool HARD;
} Difficulty;

Coords snake[LCD_WIDTH * LCD_HEIGHT], fruit;
Direction direction;
Difficulty difficulty;
int snakeLength = INITIAL_SNAKE_LENGTH;
int score = 0;
int points = EASY_SCORE;
int delay_time = DELAY_EASY;
bool shouldPlaceFruit = false;
bool passedWelcomeScreen = false;
bool difficultyChosen = false;
bool gameOver = false;

void init_buttons(void) {
    DDRA = 0xff;
    PORTA = 0x00;
	// UP Button initialization.
    DDRA &= ~(1 << PA0);
    PORTA |= (1 << PA0);
    // Right Button initialization.
    DDRA &= ~(1 << PA1);
    PORTA |= (1 << PA1);
    // Left Button initialization.
    DDRA &= ~(1 << PA2);
    PORTA |= (1 << PA2);
    // Down Button initialization.
    DDRA &= ~(1 << PA3);
    PORTA |= (1 << PA3);  
}

void set_direction(char where[10]){
	if ((strcmp(where, "UP") == 0) && (!direction.DOWN)){
		direction.UP = true;
		direction.LEFT = false;
		direction.RIGHT = false;
		direction.DOWN = false;
	}
	else if ((strcmp(where, "DOWN") == 0) && (!direction.UP)){
		direction.UP = false;
		direction.LEFT = false;
		direction.RIGHT = false;
		direction.DOWN = true;
	}
	else if ((strcmp(where, "LEFT") == 0) && (!direction.RIGHT)){
		direction.UP = false;
		direction.LEFT = true;
		direction.RIGHT = false;
		direction.DOWN = false;
	}
	else if ((strcmp(where, "RIGHT") == 0) && (!direction.LEFT)){
		direction.UP = false;
		direction.LEFT = false;
		direction.RIGHT = true;
		direction.DOWN = false;
	}
}

void button_pressed(void) {
	if (!passedWelcomeScreen){
		// UP Button Pressed, leaving welcome screen
	    if (!(PINA & (1 << PA0)) || !(PINA & (1 << PA1)) || !(PINA & (1 << PA2)) || !(PINA & (1 << PA3)) ) {
			passedWelcomeScreen = true;
	    }
	}
	else if (passedWelcomeScreen && !difficultyChosen){
		// UP Button Pressed, choosing difficulty
		if (!(PINA & (1 << PA0))){
			if (difficulty.EASY){
				difficulty.EASY = false;
				difficulty.HARD = true;
				points = HARD_SCORE;
				delay_time = DELAY_HARD;
			}
			else if (difficulty.MEDIUM){
				difficulty.MEDIUM = false;
				difficulty.EASY = true;
				points = EASY_SCORE;
				delay_time = DELAY_EASY;
			}
			else if (difficulty.HARD){
				difficulty.HARD = false;
				difficulty.MEDIUM = true;
				points = MEDIUM_SCORE;
				delay_time = DELAY_MEDIUM;
			}
		}
		// DOWN Button Pressed, choosing difficulty
		else if (!(PINA & (1 << PA3))){
			if (difficulty.EASY){
				difficulty.EASY = false;
				difficulty.MEDIUM = true;
				points = MEDIUM_SCORE;
				delay_time = DELAY_MEDIUM;
			}
			else if (difficulty.MEDIUM){
				difficulty.MEDIUM = false;
				difficulty.HARD = true;
				points = HARD_SCORE;
				delay_time = DELAY_HARD;
			}
			else if (difficulty.HARD){
				difficulty.HARD = false;
				difficulty.EASY = true;
				points = EASY_SCORE;
				delay_time = DELAY_EASY;
			}
		}
		// LEFT or RIGHT Buttons Pressed, leaving difficulty
		else if (!(PINA & (1 << PA1)) || (!(PINA & (1 << PA2)))){
			difficultyChosen = true;
		} 
	}
	else {
		// UP Button Pressed
	    if (!(PINA & (1 << PA0))) {
			set_direction("UP");
	    }
		// RIGHT Button Pressed
	    else if (!(PINA & (1 << PA1))) {
			set_direction("RIGHT");
	    }
		// LEFT Button Pressed
	    else if (!(PINA & (1 << PA2))) {
			set_direction("LEFT");
	    }
		// DOWN Button Pressed
	    else if (!(PINA & (1 << PA3))) {
			set_direction("DOWN");
	    }
	}
}

//Message to display on welcome screen
void welcome_screen(){
	lcd_clear();
	lcd_goto_xy(3, 3);
	lcd_str("Snake LCD!");
	lcd_goto_xy(2, 4);
	lcd_str("Any to Start!");
}

//Choosing difficulty(speed) of game
void difficulty_screen(){
	// EASY selected initially
	difficulty.EASY = true;
	difficulty.MEDIUM = false;
	difficulty.HARD = false;

	while (!difficultyChosen){
		lcd_clear();
		lcd_goto_xy(1, 1);
		lcd_str("Difficulty!");
		lcd_goto_xy(1, 2);
		lcd_str("U/D to choose");
		lcd_goto_xy(1, 3);
		lcd_str("L/R to start");
		lcd_goto_xy(2, 4);
		if (difficulty.EASY){
			lcd_str("EASY *");
			lcd_goto_xy(2, 5);
			lcd_str("MEDIUM");
			lcd_goto_xy(2, 6);
			lcd_str("HARD");
		}
		else if (difficulty.MEDIUM){
			lcd_str("EASY");
			lcd_goto_xy(2, 5);
			lcd_str("MEDIUM *");
			lcd_goto_xy(2, 6);
			lcd_str("HARD");
		}
		else if (difficulty.HARD){
			lcd_str("EASY");
			lcd_goto_xy(2, 5);
			lcd_str("MEDIUM");
			lcd_goto_xy(2, 6);
			lcd_str("HARD *");
		}
		button_pressed();
		_delay_ms(200);
	}
}

//drawing snake ooooO , where O is head.
void draw_snake(){
	int i;

	for (i = 0 ; i < snakeLength ; i ++) {
		lcd_goto_xy(snake[i].x,snake[i].y);
		if (i == snakeLength-1){
			lcd_chr('O');
		}
		else {
			 lcd_chr('o');
		}
	}
}

//initializing snake on random position.
void init_snake(){
	//LCD starts from 1, rand gives values from 0.
	int onY =  (rand() % LCD_HEIGHT) + 1;
	//needs to be displayed as a whole (not through wall) and we don't want it too
	//much on the right side (because initial direction is right so we want it in the first 7 pixels).
	int onX = (rand() % (LCD_WIDTH - LCD_HEIGHT)) + 1;
	int i;

	for (i = 0 ; i < 3; i ++){
		snake[i].x = onX + i;
		snake[i].y = onY;
	}
	//initial direction is right
	set_direction("RIGHT");
	draw_snake();
}

//draw the fruit as '*'
void draw_fruit(){
	lcd_goto_xy(fruit.x, fruit.y);
	lcd_chr('*');
}

//place the fruit when snake eats it(at a random position without conflict with snake)
void place_fruit(){
	bool hitSnake = true;
	int fruitX, fruitY, i;
	while (hitSnake) {
		hitSnake = false;
		fruitX = (rand() % LCD_WIDTH) + 1;
		fruitY = (rand() % LCD_HEIGHT) + 1;
		for (i = 0 ; i < snakeLength ; i ++){
			if ((fruitX == snake[i].x) && (fruitY == snake[i].y)){
				hitSnake = true;
			}
		}
	}
	fruit.x = fruitX;
	fruit.y = fruitY;
}

//function that makes the snake move.
void move(){
	int i;
	bool canMove = true;
	Coords snakeCopy[LCD_WIDTH * LCD_HEIGHT];

	//Copy original snake to a fake one to check if it's possible to move.
	for (i = 0 ; i < snakeLength ; i ++){
		snakeCopy[i].x = snake[i].x;
		snakeCopy[i].y = snake[i].y;
	}

	//move snake from tail to head 
	for (i = 0; i < snakeLength - 1; i ++){
		snakeCopy[i].x = snakeCopy[i+1].x;
		snakeCopy[i].y = snakeCopy[i+1].y;
	}

	//move the snake head in the requested direction.
	if (direction.RIGHT){
		if (snakeCopy[snakeLength-1].x + 1 == LCD_WIDTH + 1){
			snakeCopy[snakeLength-1].x = 0;
		}
		snakeCopy[snakeLength-1].x ++;
	}
	else if (direction.LEFT){
		if (snakeCopy[snakeLength-1].x - 1 == 0){
			snakeCopy[snakeLength-1].x = LCD_WIDTH + 1;
		}
		snakeCopy[snakeLength-1].x --;
	}
	else if (direction.UP){
		if (snakeCopy[snakeLength-1].y - 1 == 0){
			snakeCopy[snakeLength-1].y = LCD_HEIGHT + 1;
		}
		snakeCopy[snakeLength-1].y --;
	}
	else if (direction.DOWN){
		if (snakeCopy[snakeLength-1].y + 1 == LCD_HEIGHT + 1){
			snakeCopy[snakeLength-1].y = 0;
		}
		snakeCopy[snakeLength-1].y ++;
	}

	//if head hits body, snake cannot move anymore and we'll display the ending screen.
	for (i = 0 ; i < snakeLength - 1; i++){
		if ((snakeCopy[i].x == snakeCopy[snakeLength-1].x) && (snakeCopy[i].y == snakeCopy[snakeLength-1].y)){
			canMove = false;
		}
	}

	//if not, copy the temporary snake into the original one and draw it.
	if (canMove){
		for (i = 0 ; i < snakeLength ; i ++){
			snake[i].x = snakeCopy[i].x;
			snake[i].y = snakeCopy[i].y;
		}
		draw_snake();
	}
	//display loosing screen if snake hit himself.
	else {
		lcd_clear();
		lcd_goto_xy(4, 3);
		lcd_str("YOU LOST!");

		char displayScore[15] = "Score: ";
		char scoreAsString[10];
		itoa(score, scoreAsString, 10);

		strcat(displayScore, scoreAsString);
		lcd_goto_xy(3, 4);
		lcd_str(displayScore);

		gameOver = true;
	}
}

//checks if snake's head colides with fruit and eats it (growing with one block)
void check_eating_fruit(){
	shouldPlaceFruit = false;
	//If snake's head hits fruit, replace the fruit with the snake's new head
	//making the snake bigger and replace the fruit at another random location.
	if (direction.UP){
		if ( (((fruit.y == LCD_HEIGHT) && (snake[snakeLength-1].y - 1 == 0))
			|| (snake[snakeLength-1].y - 1 == fruit.y)) && (snake[snakeLength-1].x == fruit.x) ){
			snakeLength ++;
			snake[snakeLength-1].y = fruit.y;
			snake[snakeLength-1].x = fruit.x;
			shouldPlaceFruit = true;
			score += points;
		}
	}
	if (direction.LEFT){
		if ( (((fruit.x == LCD_WIDTH) && (snake[snakeLength-1].x -1 == 0))
			|| (snake[snakeLength-1].x - 1 == fruit.x )) && (snake[snakeLength-1].y == fruit.y) ){
			snakeLength ++;
			snake[snakeLength-1].y = fruit.y;
			snake[snakeLength-1].x = fruit.x;
			shouldPlaceFruit = true;
			score += points;
		}
	}
	if (direction.RIGHT){
			if ( (((fruit.x == 1) && (snake[snakeLength-1].x + 1 == LCD_WIDTH + 1))
			|| (snake[snakeLength-1].x + 1 == fruit.x )) && (snake[snakeLength-1].y == fruit.y) ){
			snakeLength ++;
			snake[snakeLength-1].y = fruit.y;
			snake[snakeLength-1].x = fruit.x;
			shouldPlaceFruit = true;
			score += points;
		}
	}
	if (direction.DOWN){
		if ( (((fruit.y == 1) && (snake[snakeLength-1].y + 1 == LCD_HEIGHT + 1))
			|| (snake[snakeLength-1].y + 1 == fruit.y )) && (snake[snakeLength-1].x == fruit.x) ){
			snakeLength ++;
			snake[snakeLength-1].y = fruit.y;
			snake[snakeLength-1].x = fruit.x;
			shouldPlaceFruit = true;
			score += points;
		}
	}
}

int main(void){
	lcd_init();
	lcd_contrast(0x40);
	lcd_goto_xy(1,1);
	init_buttons();
	//display welcome screen
	welcome_screen();
	//expect for a button to be pressed while on welcome screen
	while (!passedWelcomeScreen) {
		button_pressed();
	}
	//delay for switching to the next screen
	_delay_ms(300);
	//display difficulty screen(up/down for adjustment and left/right to start)
	difficulty_screen();
	//after left or right is pressed, start game.
	lcd_clear();
	//initialize snake and fruit at random location.
	init_snake();
	place_fruit();
	draw_fruit();
	_delay_ms(delay_time);
	//while the snake didn't collide with itself
	while(!gameOver){
		lcd_clear();
		draw_fruit();
		//verify is snake's head is on fruit
		check_eating_fruit();
		//if it is, place and draw the new fruit, and draw
		//the snake which is bigger(the head took the fruit's place)
		if(shouldPlaceFruit){
			place_fruit();
			draw_fruit();
			draw_snake();
		}
		//if it isn't , move the snake normally.
		else {
			move();
		}
		//check input and delay using the dificuly chosen.
		button_pressed();
		_delay_ms(delay_time);
	}
}
