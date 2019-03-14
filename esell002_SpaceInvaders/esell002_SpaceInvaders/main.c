/* Elias Sells esell002@ucr.edu
* Lab Section: 23
* Assignment: Custom Lab Project: Space Invaders
* Exercise Description: [optional - include for your own benefit]
*
* I acknowledge all content contained herein, excluding template or example
* code, is my own original work.
*/

#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "nokia5110.c"
#include "timer.h"
#include <stdio.h>
#include "ADC.h"

//--------Find GCD function --------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}
//--------End find GCD function ----------------------------------------------

//Struct for enemy ships
typedef struct _enemy {
	unsigned char EnemyXPos; //Enemy X Position
	unsigned char EnemyYPos; //Enemy Y Position
	bool isDestroyed; //Whether or not enemy is destroyed
	bool direction; // 0 for right, 1 for left
} EnemyShip;


//State enums for state machines
enum LCD_DISPLAY {START_1, WAIT_1} state_LED;
enum PLAYER_INPUT { START_2, READ_INPUTS_2} state_PLAYER;
enum ENEMY_UPDATE {START_3, UPDATE_3} state_ENEMY;
enum GAMESTATE {START_4, WAIT_4, WIN_4, LOSS_4} state_GAMESTATE;
//----------------------------------------------------

//Global Variables
unsigned char XPos;
unsigned char YPos;
bool shootLaser;
unsigned char LaserXPos;
unsigned char LaserYPosStart;
unsigned char LaserYPos;
unsigned short Joystick_Input;
bool LaserHit;
unsigned char tickMovement;
bool GameOver;
unsigned short score;
bool DisableLCD;
unsigned char EndTimer;
bool difficulty;
unsigned char EnemySprite;
unsigned short totElapsed;
unsigned char numDestroyed;
unsigned char TicksNeeded;

EnemyShip E1 = {23, 5, 0, 0};
EnemyShip E2 = {33, 5, 0, 0};
EnemyShip E3 = {43, 5, 0, 0};
EnemyShip E4 = {53, 5, 0, 0};
EnemyShip E5 = {23, 5, 0, 1};
EnemyShip E6 = {33, 5, 0, 1};
EnemyShip E7 = {43, 5, 0, 1};
EnemyShip E8 = {53, 5, 0, 1};

EnemyShip *EnemyShips[] = {&E1, &E2, &E3, &E4, &E5, &E6, &E7, &E8};

//initializes global variables, sets difficulty
void Game_Init() {
	nokia_lcd_clear();
	nokia_lcd_set_cursor(0,3);
	nokia_lcd_write_string("SPACE INVADERS", 1);
	nokia_lcd_set_cursor(22,23);
	nokia_lcd_write_string("Select", 1);
	nokia_lcd_set_cursor(10,32);
	nokia_lcd_write_string("Difficulty", 1);
	nokia_lcd_set_cursor(17,41);
	nokia_lcd_write_string("To Start", 1);
	nokia_lcd_render();
	while(1) {
		if (~PINB & 0x01 && !(~PINB & 0x02)) {
			difficulty = 0;
			EnemySprite = 0x82;
			break;
		}
		else if (~PINB & 0x02 && !(~PINB & 0x01)) {
			difficulty = 1;
			EnemySprite = 0x83;
			break;
		}
	}
	XPos = 38;
	YPos = 40;
	shootLaser = 0;
	LaserXPos = 0;
	LaserYPosStart = 34;
	LaserYPos = 0;
	Joystick_Input = 0;
	LaserHit = 0;
	tickMovement = 0;
	TicksNeeded = 5;
	
	E1.EnemyXPos = 23;
	E1.EnemyYPos = 5;
	E1.direction = 0;
	E1.isDestroyed = 0;
	
	E2.EnemyXPos = 33;
	E2.EnemyYPos = 5;
	E2.direction = 0;
	E2.isDestroyed = 0;
	
	E3.EnemyXPos = 43;
	E3.EnemyYPos = 5;
	E3.direction = 0;
	E3.isDestroyed = 0;

	E4.EnemyXPos = 53;
	E4.EnemyYPos = 5;
	E4.direction = 0;
	E4.isDestroyed = 0;
	
	E5.EnemyXPos = 23;
	E5.EnemyYPos = 12;
	E5.direction = 1;
	E5.isDestroyed = 0;
	
	E6.EnemyXPos = 33;
	E6.EnemyYPos = 12;
	E6.direction = 1;
	E6.isDestroyed = 0;
	
	E7.EnemyXPos = 43;
	E7.EnemyYPos = 12;
	E7.direction = 1;
	E7.isDestroyed = 0;

	E8.EnemyXPos = 53;
	E8.EnemyYPos = 12;
	E8.direction = 1;
	E8.isDestroyed = 0;

	GameOver = 0;
	score = 0;
	DisableLCD = 0;
	EndTimer = 0 ;
	totElapsed = 0;
	numDestroyed = 0;
}


unsigned char DetectLaserCollision(unsigned char EnX, unsigned char EnY) {
	if (((LaserXPos+2 >= (EnX+2) - 3) && (LaserXPos+2 <= (EnX+2) + 3)) && ((LaserYPosStart-LaserYPos-3 >= (EnY-3) - 3) && (LaserYPosStart-LaserYPos-3 <= (EnY-3) + 3)) && shootLaser) {
		LaserHit = 1;
		numDestroyed++;
		if (difficulty == 0) {
			score += 100 * (5000 - totElapsed)/250;
		}
		else {
			score += (100 * (5000 - totElapsed)/250) * 2;
		}
		return 1;
	}
	else {
		return 0;
	}
}

//SM Task Tick Functions

//Controls what outputs to the Nokia 5110 LCD screen and outputs it.
void LCD_DISPLAY_Tick () {
	switch (state_LED) {
		case START_1:
		state_LED = WAIT_1;
		break;
		case WAIT_1:
		break;
		default:
		break;
	}
	//State machine actions
	switch(state_LED) {
		case START_1:
		break;
		case WAIT_1:
		if (DisableLCD) {
			break;
		}
		nokia_lcd_clear();
		//Draw Player
		nokia_lcd_set_cursor(XPos, YPos);
		nokia_lcd_write_char(0x7F, 1);
		nokia_lcd_set_cursor(XPos+5, YPos);
		nokia_lcd_write_char(0x80, 1);
		
		//Draw Laser
		if (shootLaser == 1) {
			nokia_lcd_set_cursor(LaserXPos+2, LaserYPosStart - LaserYPos);
			nokia_lcd_write_char(0x81, 1);
		}
		
		//Draw Enemies
		for (unsigned k = 0; k < 8; ++k) {
			if (EnemyShips[k]->isDestroyed) {
				continue;
			}
			else {
				nokia_lcd_set_cursor(EnemyShips[k]->EnemyXPos, EnemyShips[k]->EnemyYPos);
				nokia_lcd_write_char(EnemySprite, 1);
			}
		}
		
		//No longer draws laser on next iteration if it's at the top of the screen or if it destroys a ship
		if (LaserYPosStart - LaserYPos < 5 || LaserHit) {
			shootLaser = 0;
			LaserYPos = 0;
			LaserHit = 0;
		}
		
		//nokia_lcd_set_cursor(0,0);
		//nokia_lcd_write_string(" ", 1);
		
		//Renders it all to the screen
		nokia_lcd_render();
		break;
		default:
		break;
	}
}

//Takes input from the player and manipulates various global variables to be used by other functions, like the LCD Task SM
void PLAYER_INPUT_Tick () {
	//Read Joystick Input
	Joystick_Input = adc_read(3);
	
	//State machine transitions
	switch (state_PLAYER) {
		case START_2:
		state_PLAYER = READ_INPUTS_2;
		break;

		case READ_INPUTS_2:
		break;
		
		default:
		break;
	}

	//State machine actions
	switch(state_PLAYER) {
		case START_2:
		break;

		case READ_INPUTS_2:
		++totElapsed;
		if (Joystick_Input >= 800 && XPos > 8) {
			XPos -= 2;
		}
		if (Joystick_Input <= 200 && XPos < 68) {
			XPos += 2;
		}
		if (shootLaser) {
			LaserYPos += 3;
		}
		if (~PINB & 0x04 && !shootLaser) {
			LaserXPos = XPos;
			shootLaser = 1;
		}
		break;
	}
}

void ENEMY_UPDATE_Tick() {
	switch(state_ENEMY) {
		case START_3:
		state_ENEMY = UPDATE_3;
		break;
		
		case UPDATE_3:
		break;
		
		default:
		break;
	}
	switch (state_ENEMY) {
		unsigned char tickReset = 0;
		case START_3:
		break;
		
		case UPDATE_3:
		++tickMovement;
		if (difficulty == 1) {
			TicksNeeded = 5 - numDestroyed/2;
		}
		else {
			TicksNeeded = 5;
		}
		for (size_t k = 0; k < 8; ++k) {
			if (EnemyShips[k]->isDestroyed) {
				//do nothing
				continue;
			}
			else {
				if (EnemyShips[k]->EnemyYPos >= 40) {
					GameOver = 1;
				}
				EnemyShips[k]->isDestroyed = DetectLaserCollision(EnemyShips[k]->EnemyXPos, EnemyShips[k]->EnemyYPos);
				if (tickMovement > TicksNeeded) {
					tickReset = 1;
					if (EnemyShips[k]->EnemyXPos >= 68 && EnemyShips[k]->direction == 0) {
						EnemyShips[k]->direction = 1;
						EnemyShips[k]->EnemyYPos += 7;
					}
					else if (EnemyShips[k]->EnemyXPos <= 8 && EnemyShips[k]->direction == 1) {
						EnemyShips[k]->direction = 0;
						EnemyShips[k]->EnemyYPos += 7;
					}
					else {
						if (EnemyShips[k]->direction == 0) {
							EnemyShips[k]->EnemyXPos += 5;
						}
						else {
							EnemyShips[k]->EnemyXPos -= 5;
						}
					}
				}
			}
		}
		if (tickReset) {
			tickMovement = 0;
		}
		break;
		
		default:
		break;
	}
}

void GAMESTATE_Tick() {
	unsigned char noWin = 0;
	switch(state_GAMESTATE) {
		case START_4:
		state_GAMESTATE = WAIT_4;
		break;
		
		case WAIT_4:
		break;
		
		case WIN_4:
		break;
		
		case LOSS_4:
		break;
		
		default:
		break;
	}
	switch(state_GAMESTATE) {
		case START_4:
		break;
		
		case WAIT_4:
		if (GameOver) {
			state_GAMESTATE = LOSS_4;
		}
		else {
			for(size_t k = 0; k < 8; ++k) { //checks if all enemy ships are destroyed
				if (EnemyShips[k]->isDestroyed == 0) {
					noWin = 1;
				}
				else {
					continue;
				}
			}
			if (!noWin) {
				state_GAMESTATE = WIN_4;
			}
		}
		if (~PINB & 0x08) {
			Game_Init();
		}
		if (~PINB & 0x01 && !(~PINB & 0x02)) {
			difficulty = 0;
			EnemySprite = 0x82;
			break;
		}
		else if (~PINB & 0x02 && !(~PINB & 0x01)) {
			difficulty = 1;
			EnemySprite = 0x83;
			break;
		}
		break;
		
		case WIN_4:
		++EndTimer;
		DisableLCD = 1;
		nokia_lcd_clear();
		nokia_lcd_set_cursor(15,5);
		nokia_lcd_write_string("YOU WON!!!", 1);
		nokia_lcd_set_cursor(15,30);
		nokia_lcd_write_string("Score: ", 1);
		nokia_lcd_set_cursor(55, 30);
		char* score_string;
		snprintf(score_string, 5, "%d", score);
		nokia_lcd_write_string(score_string, 1);
		nokia_lcd_render();
		if (EndTimer >= 50) {
			state_GAMESTATE = WAIT_4;
			Game_Init();
		}
		break;
		
		case LOSS_4:
		++EndTimer;
		DisableLCD = 1;
		nokia_lcd_clear();
		nokia_lcd_set_cursor(15,5);
		nokia_lcd_write_string("You Lost :(", 1);
		nokia_lcd_render();
		if (EndTimer >= 50) {
			state_GAMESTATE = WAIT_4;
			Game_Init();
		}
		break;
		
		default:
		break;
	}
}


int main()
{
	// Set Data Direction Registers
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0x0F; PORTC = 0xF0;
	//DDRD = 0xFF; PORTD = 0x00;
	
	//Initialize LCD Screen, Joystick, and Game
	nokia_lcd_init();
	adc_init();
	Game_Init();
	
	// Period for the tasks
	unsigned long int LCD_DISPLAY_calc = 20;
	unsigned long int PLAYER_INPUT_calc = 40;
	unsigned long int ENEMY_UPDATE_calc = 40;
	unsigned long int GAMESTATE_calc = 10;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(LCD_DISPLAY_calc, PLAYER_INPUT_calc);
	tmpGCD = findGCD(tmpGCD, ENEMY_UPDATE_calc);
	tmpGCD = findGCD(tmpGCD, GAMESTATE_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int LCD_DISPLAY_period = LCD_DISPLAY_calc/GCD;
	unsigned long int PLAYER_INPUT_period = PLAYER_INPUT_calc/GCD;
	unsigned long int ENEMY_UPDATE_period = ENEMY_UPDATE_calc/GCD;
	unsigned long int GAMESTATE_period = GAMESTATE_calc/GCD;
	
	unsigned long int LCD_DISPLAY_elapsed = 0;
	unsigned long int PLAYER_INPUT_elapsed = 0;
	unsigned long int ENEMY_UPDATE_elapsed = 0;
	unsigned long int GAMESTATE_elapsed = 0;
	
	//Turn on Timer
	TimerSet(GCD);
	TimerOn();

	while (1)
	{
		while(!TimerFlag) {}
		TimerFlag = 0;
		if (GAMESTATE_elapsed >= GAMESTATE_period) {
			GAMESTATE_Tick();
			GAMESTATE_elapsed = 0;
		}
		if (ENEMY_UPDATE_elapsed >= ENEMY_UPDATE_period) {
			ENEMY_UPDATE_Tick();
			ENEMY_UPDATE_elapsed = 0;
		}
		if (PLAYER_INPUT_elapsed >= PLAYER_INPUT_period) {
			PLAYER_INPUT_Tick();
			PLAYER_INPUT_elapsed = 0;
		}
		if (LCD_DISPLAY_elapsed >= LCD_DISPLAY_period) {
			LCD_DISPLAY_Tick();
			LCD_DISPLAY_elapsed = 0;
		}
		++LCD_DISPLAY_elapsed;
		++PLAYER_INPUT_elapsed;
		++ENEMY_UPDATE_elapsed;
		++GAMESTATE_elapsed;
		
	}
	return 0;
}
