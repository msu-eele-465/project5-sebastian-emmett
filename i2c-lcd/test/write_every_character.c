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

	// print every single character
	char lcd_buffer[16] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
	char lcd_position[16] = "position=       ";
	int i;
	int j;
	
	while (1)
	{
		lcd_init();

		lcd_clear_display();

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
