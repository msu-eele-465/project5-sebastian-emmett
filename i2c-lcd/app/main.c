#include <msp430fr2310.h>
#include <stdbool.h>

#include "../src/lcd.h"
#include "../../common/i2c.h"


/* --- global variables --- */


#define SLAVE_ADDRESS SLAVE1_ADDR

char curr_key = 0;
int8_t temperature_data[2];
char receive_data_buffer[3];


/* --- program --- */


int main(void)
{
    // Stop watchdog timer
	WDTCTL = WDTPW | WDTHOLD;

	const char *pattern_0 = "static          ";
	const char *pattern_1 = "toggle          ";
	const char *pattern_2 = "up counter      ";
	const char *pattern_3 = "in and out      ";
	const char *pattern_4 = "down counter    ";
	// const char *pattern_5 = "rotate 1 left   ";
	// const char *pattern_6 = "rotate 7 right  ";
	// const char *pattern_7 = "fill left       ";

	const char *set_window = "set window size ";
	const char *set_pattern = "set pattern     ";

	char temperature_buffer[] = "T=~~.~\xDF""C    N=03";
	uint8_t pattern = 255;

	uint8_t locked = 1;
	uint8_t recv_amt = 0;

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
			// Poll to see if we have new data
			recv_amt = i2c_get_received_data(receive_data_buffer);
			if (recv_amt == 1)
        	{
				// If we have a new key in the data buffer, save it to curr_key
				curr_key = receive_data_buffer[0];
				switch (curr_key)
				{
					case 'D':
						locked = 1;
						pattern = 255;
						lcd_clear_display();

						// this is so the cursor does not show up on a
						//	cleared scrren, that would defeat the purpose
						lcd_set_ddram_addr(0x20);

						break;

					case 'U':
						locked = 0;

						break;

					case '9': // #FIXME should probably remove this
						lcd_toggle_blink();

						break;

					case 'C':
						lcd_toggle_cursor();

						break;

					case '#':
						// 'C' = 01000011
						// 'F' = 01000110
						// xor   00000101, toggles between C and F
						temperature_buffer[7] ^= 0x05;

						// the farenheit temperature has not been received though
						// 	so there is no point in displaying a line

						break;

					case 'A':
						lcd_clear_display();
						lcd_print_line(set_window, 0);

						// #FIXME
						// i2c_get_received_data(curr_key);
						// temperature_buffer[14] = curr_key;

						// i2c_get_received_data(curr_key);
						// temperature_buffer[15] = curr_key;

						// the lcd is cleared here to prevent set_window from
						// 	remaining on the display
						lcd_clear_display();
						lcd_print_line(temperature_buffer, 1);

						break;

					case 'B':
						lcd_clear_display();
						lcd_print_line(set_pattern, 0);

						// #FIXME
						// i2c_get_received_data(curr_key);
						// pattern = *curr_key;

						// the lcd is cleared here to prevent set_pattern from
						// 	remaining on the display
						lcd_clear_display();
						lcd_print_line(temperature_buffer, 1);

						break;

					default:
						break;
				}

				switch (pattern)
				{
					case 0:
						lcd_print_line(pattern_0, 0);
						break;

					case 1:
						lcd_print_line(pattern_1, 0);
						break;

					case 2:
						lcd_print_line(pattern_2, 0);
						break;

					case 3:
						lcd_print_line(pattern_3, 0);
						break;

					case 4:
						lcd_print_line(pattern_4, 0);
						break;

					// case 5:
					// 	lcd_print_line(pattern_5, 0);
					// 	break;

					// case 6:
					// 	lcd_print_line(pattern_6, 0);
					// 	break;

					// case 7:
					// 	lcd_print_line(pattern_7, 0);
					// 	break;

					default:
						break;
				}
			}
			// TODO: This is almost certainly wrong right now
			else if (recv_amt == 2)
			{
				// Pull temperature data from the data buffer into our temperature buffer
				temperature_data[0] = receive_data_buffer[1];
				temperature_data[1] = receive_data_buffer[2];

				while (curr_key > 9)
				{
					temperature_buffer[2]++;
					curr_key -= 10;
				}
				temperature_buffer[3] = '0' + curr_key;
				
				temperature_buffer[4] = '0';
				while (curr_key > 9)
				{
					temperature_buffer[4]++;
					curr_key -= 10;
				}
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
