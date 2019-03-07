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


int main(void)
{	
	TimerSet(100);
	TimerOn();
	DDRB = 0x00; PORTB = 0xFF;
	unsigned char XPos = 38;
	unsigned char YPos = 42;
	unsigned char shootLaser = 0;
	unsigned char LaserXPos = 0;
	unsigned char LaserYPosStart = 36;
	unsigned char LaserYPos = 0;
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("IT'S WORKING!",1);
    //nokia_lcd_set_cursor(5, 10);
    //nokia_lcd_write_string("Nice!", 3);
	//nokia_lcd_write_char(0x7F, 1);
	//nokia_lcd_set_cursor(10, 10);
	//nokia_lcd_write_char(0x80, 1);
    nokia_lcd_render();
	while (1)
	{	
		while (!TimerFlag) {}
		if (~PINB & 0x01) {
			if (XPos > 8) {
				XPos -= 10;
			}
		}
		if (~PINB & 0x02) {
			if (XPos < 68) {	
				XPos += 10;
			}
		}
		if (~PINB & 0x04 && !shootLaser) {
			shootLaser = 1;
		}
		TimerFlag = 0;
		nokia_lcd_clear();
		nokia_lcd_set_cursor(XPos, YPos);
		nokia_lcd_write_char(0x7F, 1);
		nokia_lcd_set_cursor(XPos+5, YPos);
		nokia_lcd_write_char(0x80, 1);
		if (shootLaser == 1) {
			nokia_lcd_set_cursor(XPos+2, LaserYPosStart - LaserYPos);
			nokia_lcd_write_char(0x81, 1);
			LaserYPos += 3;
		}
		nokia_lcd_render();
		if (LaserYPosStart - LaserYPos < 5) {
			shootLaser = 0;
			LaserYPos = 0;
		}
	}
	return 0;
}

