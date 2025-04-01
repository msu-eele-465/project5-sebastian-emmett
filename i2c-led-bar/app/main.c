#include <msp430fr2310.h>
#include <stdbool.h>
#include "../../common/i2c.h"
#include "../src/led_bar.h"

// Globals for LED bar control

// Transition period -> 1s
int transition_period = 16;
// Current pattern number
char curr_num = '0';
// Previous pattern number for reset logic
char prev_num = '0';
// System lock state
bool locked = true;
// Flag for new pattern received
bool num_update = false;
// Flag to reset pattern
bool reset_pattern = false;
// Char to hold incoming data from i2c
char received_buffer[3];
// Char for received character
char received_char = 'a';
// Bool to hold if we're updating the pattern
bool update_pattern = false;
// Bool to hold if we're updating the window
bool update_window = false;

// Use SLAVE1_ADDR from i2c.h
#define SLAVE_ADDRESS SLAVE1_ADDR

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set P1.0 as output for debug
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;

    while (true)
    {
        // Initialize LED bar
        led_bar_init();

        // Disable low-power mode / GPIO high-impedance
        PM5CTL0 &= ~LOCKLPM5;

        // Initialize I2C as slave
        i2c_slave_init(SLAVE_ADDRESS);
        __enable_interrupt();

        while (true)
        {
            // Poll the i2c for new data
            if (i2c_get_received_data(received_buffer) == 1)
            {
                // Update received_char
                received_char = received_buffer[0];
                // Process received data to update LED bar state
                if (received_char == 'D')
                {
                    // Lock the system and clear the LED bar
                    locked = true;
                    led_bar_update(0x00);
                }
                else if (received_char == 'A')
                {
                    // Toggle update_window
                    update_window = !update_window;
                }
                else if (received_char == 'U')
                {
                    // Unlock the system
                    locked = false;
                }
                else if (received_char == 'B')
                {
                    // Enable update_pattern
                    update_pattern = true;
                }
                else if (received_char >= '0' && received_char <= '7' && update_pattern && !update_window)
                {
                    // Clear update_pattern
                    update_pattern = false;
                    // Store previous pattern
                    prev_num = curr_num;
                    // Set new pattern
                    curr_num = received_char;
                    // Flag new pattern received
                    num_update = true;
                    // Reset if same pattern
                    reset_pattern = (prev_num == curr_num);
                }
            }

            // Update LED bar if not locked
            if (!locked)
            {
                led_bar_update_pattern();
                led_bar_delay();
            }
        }
    }
}
