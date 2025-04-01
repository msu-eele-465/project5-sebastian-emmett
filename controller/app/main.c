#include <msp430.h>
#include <stdbool.h>
#include <string.h>
// Include this to use init_keypad() and poll_keypad()
#include "../src/keyboard.h"
// For init_heartbeat()
#include "../src/heartbeat.h"
// For adjusting the pwm signal meant for rgb_led
#include "../src/rgb_led.h"
// For creating the pwm signal on P6.0 - P6.2
#include "../src/pwm.h"
// Include I2C header
#include "../../common/i2c.h"
#include "msp430fr2355.h"
#include "../src/lm19.h"


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
char curr_pattern = 0;

// A boolean for locked/unlocked system state
bool locked = true;

// Our base transition period variable. Technically this is an int representation of how many 1/16s our actual BTP is
int base_transition_period = 16;

// If a numeric key is pressed, set num_update = true
bool num_update = false;

// If the new numeric key == previous numeric key, set reset_pattern = true
bool reset_pattern = false;

// The global int BTP_multiplier:
float BTP_multiplier = 1;

// Variables for our passcode

// True while we're collecting 4 digits
bool unlocking = false;
// Hard-coded correct passcode
static const char CORRECT_PASS[] = "1234";
// Room for 4 digits + null terminator
char pass_entered[5];
// How many digits we've collected so far
unsigned pass_index = 0;

// Timeout tracking: We'll use the heartbeat (1Hz) to decrement
// 5-second countdown if unlocking
volatile int pass_timer = 0;

// Variables for LM19
bool fahrenheit_mode = false;
bool setting_window = false;
bool setting_pattern = false;

// Function to set RGB LED color based on the current pattern (migrated from led_bar.c)
void set_rgb_for_pattern(char pattern)
{
    switch (pattern)
    {
        case '0':
            // white
            rgb_set(0xFF, 0xFF, 0xFF);
            break;
        case '1':
            // blood
            rgb_set(0x94, 0x00, 0x00);
            break;
        case '2':
            // lime
            rgb_set(0x00, 0xFF, 0x00); 
            break;
        case '3':
            // orange
            rgb_set(0xFF, 0x80, 0x00); 
            break;
        case '4':
            // blue
            rgb_set(0x00, 0x00, 0xFF); 
            break;
        case '5':
            // navy
            rgb_set(0x15, 0x15, 0x2e); 
            break;
        case '6':
            // lavender
            rgb_set(0xFF, 0x00, 0xE1); 
            break;
        case '7':
            // forest
            rgb_set(0x20, 0x46, 0x22); 
            break;
        default:
            // No change or could set a default color (e.g., off: 0x00, 0x00, 0x00)
            break;
    }
}

// ----------------------------------------------------------------------------
// MAIN
// ----------------------------------------------------------------------------
int main(void)
{
    // Stop watchdog
    WDTCTL = WDTPW | WDTHOLD;

    // Set up Timer_B0 for blinking P1.0
    init_heartbeat();
    // Init keyboard with P4s as input and P5s as output
    init_keypad();
    // LED on P6.6 to show when a key gets pressed
    init_responseLED();
    // Timer_B1 => ~50 ms interrupt
    init_keyscan_timer();
    // start pwm signal on P6.0 - P6.2
    pwm_init();
    // Initialize LM19 temperature sensor
    temp_sensor_init();
    // Initialize I2C module as master
    i2c_master_init();

    // Turn on I/O
    PM5CTL0 &= ~LOCKLPM5;


    // Enable global interrupts for the heartbeat timer
    __bis_SR_register(GIE);

    // start state led as red color, for locked state
    rgb_set(0xC4, 0x3E, 0x1D);
    while (1)
    {
        // ----------------------------------------------------------------------------
        // 1) If we are locked but NOT unlocking...
        //    - Wait for the first numeric key press (num_update)
        // ----------------------------------------------------------------------------
        if (locked && !unlocking)
        {
            // set state led as red color, for locked state
            rgb_set(0xC4, 0x3E, 0x1D);

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
                // We consumed this press
                num_update = false;
                // set state led to yellowish color, for unlocking
                rgb_set(0xC4, 0x92, 0x1D);

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
                pass_index = 0;

                // set state led as red color, for locked state
                rgb_set(0xC4, 0x3E, 0x1D);
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
                    // set state LED to red color, for locked
                    rgb_set(0xC4, 0x3E, 0x1D);

                    // 4 digits => compare
                    pass_entered[4] = '\0';
                    num_update = false;

                    if (strcmp(pass_entered, CORRECT_PASS) == 0)
                    {
                        // Correct => unlock
                        locked = false;
                        // Setting this to nothing so it doesn't immedately jump into a pattern
                        curr_num = 0;
                        // set state LED to blueish color, for unlocked
                        rgb_set(0x1D, 0xA2, 0xC4);
                        // Send 'U' to both slaves
                        i2c_send_to_both('U');
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
            // locked == false
            // Update RGB LED based on curr_num when unlocked
            set_rgb_for_pattern(curr_pattern);
        }
    }
}
