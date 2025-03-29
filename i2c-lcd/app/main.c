#include <msp430fr2310.h>
#include <stdbool.h>

#include "../src/lcd.h"
#include "../../common/i2c.h"


/* --- global variables --- */

#define SLAVE_ADDRESS SLAVE1_ADDR

char curr_key = 'a';

/* --- program --- */


// this contains the logic to update the transition_period
// 	string according to the base_transition_period
void _update_transition_period(char *transition_period, uint8_t base_transition_period)
{

	// soft cap of base_transition_period at 9.75, since it
	// 	seems unreasonable to display anything more than that
	if (base_transition_period > 39)
	{
		base_transition_period = 39;
	}

	// first turn the integer part into a string
	transition_period[7] = '0' + (base_transition_period >> 2);

	// because the base_transition_period moves in increments of
	//  0.25 it is possible to switch on the lower 4 bits of it
	//  and insert the correct string with no further calculation
	switch (base_transition_period & 0x03)
	{
		case 0x00:
			transition_period[9] = '0';
			transition_period[10] = '0';
			break;

		case 0x01:
			transition_period[9] = '2';
			transition_period[10] = '5';
			break;

		case 0x02:
			transition_period[9] = '5';
			transition_period[10] = '0';
			break;

		case 0x03:
			transition_period[9] = '7';
			transition_period[10] = '5';
			break;
	}
}

int main(void)
{
    // Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	const char *pattern_0 = "static          ";
	const char *pattern_1 = "toggle          ";
	const char *pattern_2 = "up counter      ";
	const char *pattern_3 = "in and out      ";
	const char *pattern_4 = "down counter    ";
	const char *pattern_5 = "rotate 1 left   ";
	const char *pattern_6 = "rotate 7 left   ";
	const char *pattern_7 = "fill left       ";

	char transition_period[] = "period=0.00     ";

	// each integer value will represent a change of 0.25 in the
	//  actual transition_period, due to this the current value
	//  is interpreted as 1
	// this interpretation makes string conversion much faster
	uint8_t base_transition_period = 4;	

	uint8_t locked = 1;

    // Disable low-power mode / GPIO high-impedance
	PM5CTL0 &= ~LOCKLPM5;

	// This double while(1) loop looks redudant but is quite important. Without this
	// 	extra while(1) loop, when powering up a programmed MSP the lcd_init will
	// 	always fail. I do not know why this solves it, but it does.
	// Unfortunately, this extra while(1) loop can cause issues when debugging with CCS.
	while (1)
	{
		lcd_init();

		// Initialize I2C as slave
    	i2c_slave_init(SLAVE_ADDRESS);
		__enable_interrupt();

		while (1)
		{
			// Poll to see if we have a new key - store in curr_key if so
			if (i2c_get_received_data(&curr_key))
        	{
				switch (curr_key)
				{
					case 'D':
						locked = 1;
						lcd_clear_display();

						// this is so the cursor does not show up on a
						//	cleared scrren, that would defeat the purpose
						lcd_set_ddram_addr(0x20);

						break;

					case 'U':
						locked = 0;

						_update_transition_period(transition_period, base_transition_period);
						lcd_print_line(transition_period, 1);

						break;

					case 'A':
						base_transition_period++;

						_update_transition_period(transition_period, base_transition_period);
						lcd_print_line(transition_period, 1);

						break;

					case 'B':
						base_transition_period--;

						_update_transition_period(transition_period, base_transition_period);
						lcd_print_line(transition_period, 1);

						break;

					case 'C':
						lcd_toggle_cursor();

						break;

					case '0':
						lcd_print_line(pattern_0, 0);

						break;

					case '1':
						lcd_print_line(pattern_1, 0);

						break;

					case '2':
						lcd_print_line(pattern_2, 0);

						break;

					case '3':
						lcd_print_line(pattern_3, 0);

						break;

					case '4':
						lcd_print_line(pattern_4, 0);

						break;

					case '5':
						lcd_print_line(pattern_5, 0);

						break;

					case '6':
						lcd_print_line(pattern_6, 0);

						break;

					case '7':
						lcd_print_line(pattern_7, 0);

						break;

					case '9':
						lcd_toggle_blink();

						break;

					default:
						break;
				}

				lcd_update_current_key();
			}
			else
			{
				if (locked)
				{
					lcd_clear_display();
				}
			}
		}
	}
}
