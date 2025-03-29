#include <msp430.h>
#include <stdbool.h>
#include <string.h>
#include "../src/keyboard.h"    // Include this to use init_keypad() and poll_keypad()
#include "../src/heartbeat.h"   // For init_heartbeat()
#include "../src/led_bar.h"     // For led_bar_update_pattern() and led_bar_delay
#include "../src/rgb_led.h"     // For adjusting the pwm signal meant for rgb_led
#include "../src/pwm.h"         // For creating the pwm signal on P6.0 - P6.2
#include "../../common/i2c.h"   // Include I2C header

// Testing main.c file for sending i2c values from keypad

// ----------------------------------------------------------------------------
// Globals! (yes they deserve their own lil space)
// ----------------------------------------------------------------------------
// For general key tracking
bool key_down = false;
char curr_key = 0;
char prev_key = 0;

// For numeric-only tracking
char curr_num = 0;
char prev_num = 0;

// A boolean for locked/unlocked system state
bool locked = true; // Default true

// Our base transition period variable. Technically this is an int representation of how many 1/16s our actual BTP is
int base_transition_period = 16;

// If a numeric key is pressed, set num_update = true
bool num_update = false;

// If the new numeric key == previous numeric key, set reset_pattern = true
bool reset_pattern = false;

// The global int BTP_multiplier:
float BTP_multiplier = 1;

// Variables for our passcode
bool unlocking = false;             // True while we're collecting 4 digits
static const char CORRECT_PASS[] = "1234";  // Hard-coded correct passcode
char pass_entered[5];               // Room for 4 digits + null terminator
unsigned pass_index = 0;            // How many digits we've collected so far

// Timeout tracking: We'll use the heartbeat (1Hz) to decrement
volatile int pass_timer = 0;        // 5-second countdown if unlocking

// ----------------------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog

    init_heartbeat();           // Set up Timer_B0 for blinking P1.0
    init_keypad();              // Init keyboard with P4s as input and P5s as output
    init_responseLED();         // LED on P6.6 to show when a key gets pressed
    init_keyscan_timer();       // Timer_B1 => ~50 ms interrupt
    led_bar_init();             // configure P3.0 - P3.7 as outputs for led bar
    pwm_init();                 // start pwm signal on P6.0 - P6.2
    i2c_master_init();          // Initialize I2C module as master

    PM5CTL0 &= ~LOCKLPM5;       // Unlock I/Os

    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    rgb_set(0xC4, 0x3E, 0x1D);  // start state led as red color, for locked state
    while (1)
    {
        // ----------------------------------------------------------------------------
        // 1) If we are locked but NOT unlocking...
        //    - Wait for the first numeric key press (num_update)
        // ----------------------------------------------------------------------------
        if (locked && !unlocking)
        {
            rgb_set(0xC4, 0x3E, 0x1D);  // set state led as red color, for locked state
            led_bar_update(0x00);   // clear the bar display

            // If no numeric key has been pressed yet, do nothing
            if (!num_update)
            {
                // Just idle here
                __no_operation();
            }
            else
            {
                // We got a numeric press => start unlocking process
                unlocking = true;
                num_update = false;     // We consumed this press
                rgb_set(0xC4, 0x92, 0x1D);      // set state led to yellowish color, for unlocking

                // Start collecting passcode digits
                pass_index = 0;

                // The digit that triggered num_update is in 'curr_num'
                pass_entered[pass_index++] = curr_num;

                // Start 5-second timer
                pass_timer = 5;
            }
        }

        // ----------------------------------------------------------------------------
        // 2) If we ARE locked and in the middle of unlocking...
        //    - Gather a total of 4 numeric keys
        // ----------------------------------------------------------------------------
        else if (locked && unlocking)
        {
            // Check if we've run out of time
            if (pass_timer == 0)
            {
                // Time’s up => reset D:
                unlocking = false;

                rgb_set(0xC4, 0x3E, 0x1D);  // set state led as red color, for locked state
            }
            else
            {
                // We still have time => collect digits!
                if (pass_index < 4)
                {
                    if (num_update)
                    {
                        pass_entered[pass_index++] = curr_num;
                        num_update = false;
                    }
                }
                else
                {
                    rgb_set(0xC4, 0x3E, 0x1D);  // set state LED to red color, for locked

                    // 4 digits => compare
                    pass_entered[4] = '\0';
                    num_update = false;

                    if (strcmp(pass_entered, CORRECT_PASS) == 0)
                    {
                        // Correct => unlock
                        locked = false;
                        curr_num = 0; // Setting this to nothing so it doesn't immedately jump into a pattern
                        rgb_set(0x1D, 0xA2, 0xC4);  // set state LED to blueish color, for unlocked
                        i2c_send_to_both('U');      // Send 'U' to both slaves
                    }
                    // Otherwise => stay locked, reset
                    unlocking = false;
                }
            }
        }

        // ----------------------------------------------------------------------------
        // 3) If we are NOT locked => begin updating led_bar
        // ----------------------------------------------------------------------------
        else
        {
            // locked == false => update led_bar
            led_bar_update_pattern();
            led_bar_delay();
        }
    }
}
