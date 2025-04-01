#include <msp430.h>
#include <stdbool.h>
#include "keyboard.h"
#include "../src/rgb_led.h"
#include "../../common/i2c.h"
#include "../src/lm19.h"

extern bool key_down;
extern char curr_key;
extern char prev_key;
extern bool locked;
extern bool unlocking;
extern int base_transition_period;
extern char curr_num;
extern char prev_num;
extern bool num_update;
extern bool reset_pattern;

extern bool setting_pattern;
extern bool setting_window;
extern char curr_pattern;

uint8_t temp_window_size = 0;

// 4x4 Keypad Layout
static const char KEYPAD_MAP[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// ----------------------------------------------------------------------------
// init_keypad: 
//   Rows (P5.0..3) as outputs
//   Columns now on P4.4..7 - active-high requires pull-downs
// ----------------------------------------------------------------------------
void init_keypad(void)
{
    // Rows = outputs, start low
    P5DIR |= (BIT0 | BIT1 | BIT2 | BIT3);
    P5OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    // Columns = inputs on P4.4..7
    P4DIR &= ~(BIT4 | BIT5 | BIT6 | BIT7);

    // Enable internal pull-downs for columns on P4.4..7
    P4REN |=  (BIT4 | BIT5 | BIT6 | BIT7);
    P4OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
}

// ----------------------------------------------------------------------------
// poll_keypad:
//   1) For each row, set that row high, all others low
//   2) Read columns (P4.4..7). If any column bit is 1 => pressed key
//   3) Return the char from KEYPAD_MAP[row][col], or 0 if none
// ----------------------------------------------------------------------------
char poll_keypad(void)
{
    #define ROW_MASK (BIT0 | BIT1 | BIT2 | BIT3)

    int row;
    for (row = 0; row < 4; row++)
    {
        // Clear all rows
        P5OUT &= ~ROW_MASK;
        // Drive *only* this row high
        P5OUT |= (BIT0 << row);

        // Small settle delay - DO NOT REMOVE THIS LMAO
        __delay_cycles(50);

        // Read columns from P4.4..7 => shift right by 4, mask 0x0F
        unsigned char col_state = (P4IN >> 4) & 0x0F;

        int col;
        for (col = 0; col < 4; col++)
        {
            // If column bit is high => pressed key
            if (col_state & (1 << col))
            {
                // Reset rows
                P5OUT &= ~ROW_MASK;
                // Return the key from KEYPAD_MAP[row][col]
                return KEYPAD_MAP[row][col];
            }
        }
    }

    // No key found
    P5OUT &= ~ROW_MASK;
    return 0;
}

// ----------------------------------------------------------------------------
// init_responseLED: LED on P6.6 (off initially)
// ----------------------------------------------------------------------------
void init_responseLED(void)
{
    P6DIR |= BIT6;
    P6OUT &= ~BIT6;
}

// ----------------------------------------------------------------------------
// init_keyscan_timer: Timer_B1 => fires ~every 50 ms
// ----------------------------------------------------------------------------
void init_keyscan_timer(void)
{
    // For 50 ms @ ~1 MHz SMCLK, /64 => 1 MHz/64 = 15625 Hz
    // 50 ms => 0.05 * 15625 = 781 counts
    TB1CTL   = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;
    // total /64
    TB1EX0   = TBIDEX__8;
    // ~50 ms 
    TB1CCR0  = 781;
    // Enable CCR0 interrupt
    TB1CCTL0 = CCIE;
}

// ----------------------------------------------------------------------------
// Timer_B1 ISR => polls keypad every ~50ms and runs keyboard_interrupt logic
// ----------------------------------------------------------------------------
#pragma vector=TIMER1_B0_VECTOR
__interrupt void TIMER1_B0_ISR(void)
{
    char key = poll_keypad();
    // A key was detected
    if (key != 0 && !key_down)
    {
        key_down = true;
        // Set P6.6 LED on
        P6OUT |= BIT6;

        // Shift curr_key -> prev_key, store the new key
        prev_key = curr_key;
        curr_key = key;

        // Send the key if not locked
        if (!locked)
        {
            i2c_send_to_both(key);
        }

        // ---------------------------
        // Additional Logic
        // ---------------------------
        
        // 1) If 'D' => set locked to true
        if (key == 'D')
        {
            locked = true;
            unlocking = false;
            num_update = false;
        }
        // 2) If 'A' => base_transition_period -= 4, min=4
        else if (key == 'A' && !locked)
        {
            if (setting_window == false)
            {
                temp_window_size = 0;
                setting_window = true;
            }
            else {
                setting_window = false;
                set_window_size(temp_window_size);
            }
        }
        // 3) If 'B' => base_transition_period += 4
        else if (key == 'B' && !locked)
        {
            setting_pattern = true;
        }
        // 4) If key is numeric => update prev_num/curr_num, set flags
        else if (key >= '0' && key <= '9')
        {
            // SHIFT
            prev_num = curr_num;
            curr_num = key;

            // SET num_update = true
            num_update = true;

            // If prev_num == curr_num => reset_pattern = true
            if (prev_num == curr_num)
            {
                reset_pattern = true;
            }

            if (setting_pattern)
            {
                curr_pattern = curr_num;
                setting_pattern = false;
            }

            if (setting_window)
            {
                if (temp_window_size == 0)
                {
                    // If nothing has been entered for the window size, set it to the num pressed
                    temp_window_size = curr_num - '0';
                } else if (temp_window_size < 10) {
                    // If a number already has been entered, multiply it by 10 (shift to 10s place) and add new num
                    temp_window_size = temp_window_size * 10;
                    temp_window_size = temp_window_size + curr_num - '0';
                }
            }
        }
        else if (key == '#' && !locked)
        {
            fahrenheit_mode = !fahrenheit_mode;
        }
        // else: ignore other keys (#, C)

    } 
    else if (key == 0 && key_down == true) 
    {
        key_down = false;

        // Set P6.6 to off
        P6OUT &= ~BIT6;
    }
}
