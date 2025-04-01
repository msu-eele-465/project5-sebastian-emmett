#include <msp430fr2310.h>
#include <stdbool.h>

#include "../src/lcd.h"
#include "../../common/i2c.h"


/* --- defines --- */


// this is used in the slave init for i2c
#define SLAVE_ADDRESS SLAVE1_ADDR


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
	const char *pattern_5 = "rotate 1 left   ";
	const char *pattern_6 = "rotate 7 right  ";
	const char *pattern_7 = "fill left       ";

	const char *set_window = "set window size ";
	const char *set_pattern = "set pattern     ";

	char temperature_buffer[] = "T=//./\xDF""C    N=03";
	uint8_t pattern = 255;

	uint8_t locked = 1;

	uint8_t recv_amt;
	char receive_data_buffer[3];

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
				switch (receive_data_buffer[0])
				{
					case 'D':
						locked = 1;
						pattern = 255;

						lcd_clear_display();

						break;

					case 'U':
						locked = 0;
						lcd_print_line(temperature_buffer, 1);

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

						// 1 digit of window size is received with each call, so
						//  receive then update the buffer twice
						while(!i2c_get_received_data(receive_data_buffer));
						temperature_buffer[14] = receive_data_buffer[0];

						while(!i2c_get_received_data(receive_data_buffer));
						temperature_buffer[15] = receive_data_buffer[0];

						// user should press 'A' here to signify end of input,
						//	there is no reason to verify though
						while(!i2c_get_received_data(receive_data_buffer));

						// the lcd is cleared here to prevent set_window from
						// 	remaining on the display
						lcd_clear_display();
						lcd_print_line(temperature_buffer, 1);

						break;

					case 'B':
						lcd_clear_display();
						lcd_print_line(set_pattern, 0);

						// a letter is received so subtract '0' to get the integer
						while(!i2c_get_received_data(receive_data_buffer));
						pattern = receive_data_buffer[0] - '0';

						// the lcd is cleared here to prevent set_pattern from
						// 	remaining on the display
						lcd_clear_display();
						lcd_print_line(temperature_buffer, 1);

						break;

					default:
						break;
				}

				// this should be a switch statement but that uses a few more bytes
				if (pattern == 0)
				{
					lcd_print_line(pattern_0, 0);
				}
				else if (pattern == 1)
				{
					lcd_print_line(pattern_1, 0);
				}
				else if (pattern == 2)
				{
					lcd_print_line(pattern_2, 0);
				}
				else if (pattern == 3)
				{
					lcd_print_line(pattern_3, 0);
				}
				else if (pattern == 4)
				{
					lcd_print_line(pattern_4, 0);
				}
				else if (pattern == 5)
				{
					lcd_print_line(pattern_5, 0);
				}
				else if (pattern == 6)
				{
					lcd_print_line(pattern_6, 0);
				}
				else if (pattern == 7)
				{
					lcd_print_line(pattern_7, 0);
				}
			}
			else if (!locked)
			{
				// for reference:
				//  receive_data_buffer[1] -> whole number part of temp
				//  receive_data_buffer[2] -> float part of temp, as tenths

				// extract whole number part
				temperature_buffer[2] = '0';
				temperature_buffer[3] = receive_data_buffer[1];
				while (temperature_buffer[3] > 9)
				{
					temperature_buffer[2]++;
					temperature_buffer[3] -= 10;
				}
				temperature_buffer[3] += '0';
				
				// extract float part
				temperature_buffer[5] = '0' + receive_data_buffer[2];
				lcd_print_line(temperature_buffer, 1);
			}
		}
	}
}
