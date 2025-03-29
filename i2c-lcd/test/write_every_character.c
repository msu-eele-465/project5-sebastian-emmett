#include <msp430fr2310.h>

#include "../src/lcd.h"

char curr_key = ' ';

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	// this is a status led to see if the program is actually running
	P1SEL0 &= ~BIT1;
	P1SEL1 &= ~BIT1;
	P1DIR |= BIT1;
    P1OUT &= ~BIT1;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    const char bitmap_0[] = "\x04\x0E\x0A\x0A\x0A\x11\x15\x0A";	// rocket ship
    const char bitmap_1[] = "\x00\x0A\x15\x11\x0A\x04\x00\x00";	// heart
	const char bitmap_2[] = "\x04\x04\x04\x1F\x04\x04\x04\x00";	// crossed lines
	const char bitmap_3[] = "\x1B\x1B\x00\x00\x0E\x1F\x11\x00";	// creeper face
	const char bitmap_4[] = "\x11\x1B\x0A\x04\x0A\x1B\x11\x00";	// propellor
	const char bitmap_5[] = "\x00\x0E\x0A\x0A\x0A\x0A\x0E\x00";	// rectangle
	const char bitmap_6[] = "\x17\x1D\x00\x1D\x17\x00\x17\x1D";	// wavy lines
	const char bitmap_7[] = "\x00\x04\x0E\x1F\x0E\x04\x00\x00";	// dot

	// print every single character
	char lcd_buffer[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
	char lcd_position[16] = "position=       ";
	int i;
	int j;
	
	while (1)
	{
		lcd_init();

		lcd_clear_display();

		lcd_toggle_cursor();
		lcd_toggle_blink();

		lcd_create_character(bitmap_0, 0);
		lcd_create_character(bitmap_1, 1);
		lcd_create_character(bitmap_2, 2);
		lcd_create_character(bitmap_3, 3);
		lcd_create_character(bitmap_4, 4);
		lcd_create_character(bitmap_5, 5);
		lcd_create_character(bitmap_6, 6);
		lcd_create_character(bitmap_7, 7);

		while (1)
		{
			for (i = 0 ; i < 16; i++)
			{
				lcd_print_line(lcd_buffer, 0);
				for (j = 0; j < 16; j++)
				{
					lcd_buffer[j] += 16;
				}

				lcd_position[9] = '0' + i / 10;
				lcd_position[10] = '0' + i % 10;
				lcd_print_line(lcd_position, 1);

				// delay 4 (ish) seconds
				__delay_cycles(4000000);

				P1OUT ^= BIT1;
			}
		}
	}
}
