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


int main(void)
{
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("IT'S WORKING!",1);
    nokia_lcd_set_cursor(0, 10);
    nokia_lcd_write_string("Nice!", 3);
    nokia_lcd_render();
	//while (1)
	//{
	//}
	return 0;
}

