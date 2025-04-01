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

    const char line_0[] = "write me       0";
    const char line_1[] = "don't write me 1";

    lcd_print_line(line_0, 0);

    lcd_print_line(line_1, 1);

    while (1);
}
