#include <msp430fr2310.h>

#include "../src/lcd.h"

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	lcd_init();

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    lcd_clear_display();

    lcd_set_mode(1, 0);
    lcd_cmd_send(0x61);

	// cursor on, off
	lcd_toggle_cursor();
	lcd_toggle_cursor();

	// blink on, off
	lcd_toggle_blink();
	lcd_toggle_blink();

	// cursor & blink on, cursor and blink off
	lcd_toggle_cursor();
	lcd_toggle_blink();
	lcd_toggle_cursor();
	lcd_toggle_blink();

    while (1);
}
