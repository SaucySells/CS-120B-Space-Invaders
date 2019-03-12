/* Elias Sells esell002@ucr.edu
* Lab Section: 23
* Assignment: Custom Lab Project: Space Invaders
* Exercise Description: [optional - include for your own benefit]
*
* I acknowledge all content contained herein, excluding template or example
* code, is my own original work.
*/

#include <avr/io.h>
#include "nokia5110.c"
#include "timer.h"
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
	unsigned char isDestroyed; //Whether or not enemy is destroyed
	unsigned char direction; // 0 for right, 1 for left
} EnemyShip;


//State enums for state machines
enum LCD_DISPLAY {START_1, WAIT_1} state_LED;
enum PLAYER_INPUT { START_2, READ_INPUTS_2, JOYSTICK_WAIT_2 } state_PLAYER;
enum ENEMY_UPDATE {START_3, UPDATE_3} state_ENEMY;
//----------------------------------------------------

//Global Variables
unsigned char XPos = 38;
unsigned char YPos = 42;
unsigned char shootLaser = 0;
unsigned char LaserXPos = 0;
unsigned char LaserYPosStart = 36;
unsigned char LaserYPos = 0;
unsigned short up = 800;
unsigned short down = 200;
unsigned short Joystick_Input = 0;
unsigned char LaserHit = 0;
static EnemyShip E1 = {15, 5, 0, 0};
EnemyShip *EnemyShips[] = {&E1};



//----------------------------------------------------

unsigned char DetectLaserCollision(unsigned char EnX, unsigned char EnY) {
	if (((LaserXPos >= EnX - 2) && (LaserXPos <= EnX + 2)) && ((LaserYPos >= EnY - 1) && (LaserYPos <= EnY + 7))) {
		LaserHit = 1;
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
		nokia_lcd_clear();
		//Draw Player
		nokia_lcd_set_cursor(XPos, YPos);
		nokia_lcd_write_char(0x7F, 1);
		nokia_lcd_set_cursor(XPos+5, YPos);
		nokia_lcd_write_char(0x80, 1);
		
		//Draw Laser
		if (shootLaser == 1) {
			nokia_lcd_set_cursor(XPos+2, LaserYPosStart - LaserYPos);
			nokia_lcd_write_char(0x81, 1);
			LaserYPos += 3;
		}
		
		//Draw Enemies
		for (unsigned k = 0; k < sizeof(EnemyShips); ++k) {
			if (EnemyShips[k]->isDestroyed) {
				//do nothing
			}
			else {
				nokia_lcd_set_cursor(EnemyShips[k]->EnemyXPos, EnemyShips[k]->EnemyYPos);
				nokia_lcd_write_char(0x82, 1);
			}
		}
		
		//No longer draws laser on next iteration if it's at the top of the screen
		if (LaserYPosStart - LaserYPos < 5 || LaserHit) {
			shootLaser = 0;
			LaserYPos = 0;
			LaserHit = 0;
		}
		
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

	case JOYSTICK_WAIT_2:
		break;
		
	default:
		break;
	}

	//State machine actions
	switch(state_PLAYER) {
	case START_2:
		break;

	case READ_INPUTS_2:
		if (Joystick_Input >= up && XPos > 8) {
			XPos -= 2;
			//state_PLAYER = JOYSTICK_WAIT_2;
		}
		if (Joystick_Input <= down && XPos < 68) {
			XPos += 2;
			//state_PLAYER = JOYSTICK_WAIT_2;
		}
		if (~PINB & 0x04 && !shootLaser) {
			shootLaser = 1;
		}
		break;

	case JOYSTICK_WAIT_2:
		if (Joystick_Input < up && Joystick_Input > down) {
			state_PLAYER = READ_INPUTS_2;
		}
		break;
		
	default:		
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
		case START_3:
			break;
		
		case UPDATE_3:
			for (unsigned k = 0; k < sizeof(EnemyShips); ++k) {
				EnemyShips[k]->isDestroyed = DetectLaserCollision(EnemyShips[k]->EnemyXPos, EnemyShips[k]->EnemyYPos);
				if (EnemyShips[k]->isDestroyed) {
					//do nothing
				}
				else {
					EnemyShips[k]->EnemyXPos += 3;
				}
			}
			break;
		
		default:
			break;
	}
}


int main(void)
{
	// Set Data Direction Registers
	//DDRA = 0xFC; PORTA = 0x03;
	DDRB = 0x00; PORTB = 0xFF;
	//DDRC = 0xF0; PORTC = 0x0F;
	//DDRD = 0xFF; PORTD = 0x00;
	
	//Initialize LCD Screen and Joystick
	nokia_lcd_init();
	adc_init();
	
	// Period for the tasks
	unsigned long int LCD_DISPLAY_calc = 20;
	unsigned long int PLAYER_INPUT_calc = 40;
	unsigned long int ENEMY_UPDATE_calc = 40;
	//unsigned long int SMTick4_calc = 10;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(LCD_DISPLAY_calc, PLAYER_INPUT_calc);
	tmpGCD = findGCD(tmpGCD, ENEMY_UPDATE_calc);
	//tmpGCD = findGCD(tmpGCD, SMTick4_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int LCD_DISPLAY_period = LCD_DISPLAY_calc/GCD;
	unsigned long int PLAYER_INPUT_period = PLAYER_INPUT_calc/GCD;
	unsigned long int ENEMY_UPDATE_period = ENEMY_UPDATE_calc/GCD;
	//unsigned long int SMTick4_period = SMTick4_calc/GCD;
	
	unsigned long int LCD_DISPLAY_elapsed = 0;
	unsigned long int PLAYER_INPUT_elapsed = 0;
	unsigned long int ENEMY_UPDATE_elapsed = 0;
	
	//Turn on Timer
	TimerSet(GCD);
	TimerOn();

	while (1)
	{
		while(!TimerFlag) {}
		TimerFlag = 0;
		
		if (LCD_DISPLAY_elapsed >= LCD_DISPLAY_period) {
			LCD_DISPLAY_Tick();
			LCD_DISPLAY_elapsed = 0;
		}
		if (PLAYER_INPUT_elapsed >= PLAYER_INPUT_period) {
			PLAYER_INPUT_Tick();
			PLAYER_INPUT_elapsed = 0;
		}
		if (ENEMY_UPDATE_elapsed >= ENEMY_UPDATE_period) {
			ENEMY_UPDATE_Tick();
			ENEMY_UPDATE_elapsed = 0;
		}
		++LCD_DISPLAY_elapsed;
		++PLAYER_INPUT_elapsed;
		++ENEMY_UPDATE_elapsed;
		
	}
	return 0;
}
