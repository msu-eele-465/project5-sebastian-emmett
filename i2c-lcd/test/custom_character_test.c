#include <msp430fr2310.h>

#include "../src/lcd.h"

char curr_key = 'a';

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	lcd_init();

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    lcd_clear_display();

    lcd_toggle_cursor();
    lcd_toggle_blink();

    const char bitmap_0[] = "\x04\x0E\x0A\x0A\x0A\x11\x15\x0A";	// rocket ship
    const char bitmap_1[] = "\x00\x0A\x15\x11\x0A\x04\x00\x00";	// heart
	const char bitmap_2[] = "\x04\x04\x04\x1F\x04\x04\x04\x00";	// crossed lines
	const char bitmap_3[] = "\x1B\x1B\x00\x00\x0E\x1F\x11\x00";	// creeper face
	const char bitmap_4[] = "\x11\x1B\x0A\x04\x0A\x1B\x11\x00";	// propellor
	const char bitmap_5[] = "\x00\x0E\x0A\x0A\x0A\x0A\x0E\x00";	// rectangle
	const char bitmap_6[] = "\x17\x1D\x00\x1D\x17\x00\x17\x1D";	// wavy lines
	const char bitmap_7[] = "\x00\x04\x0E\x1F\x0E\x04\x00\x00";	// dot

	lcd_create_character(bitmap_0, 0);
	lcd_create_character(bitmap_1, 1);
	lcd_create_character(bitmap_2, 2);
	lcd_create_character(bitmap_3, 3);
	lcd_create_character(bitmap_4, 4);
	lcd_create_character(bitmap_5, 5);
	lcd_create_character(bitmap_6, 6);
	lcd_create_character(bitmap_7, 7);

    lcd_print_line("\x00\x01\x02\x03\x04\x05\x06\x07        ", 0);

    curr_key = '@';
    lcd_print_line("\x07\x06\x05\x04\x03\x02\x01\x00        ", 1);

    while (1);
}
