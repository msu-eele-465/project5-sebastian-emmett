#include <msp430.h>
#include <stdbool.h>
#include "../src/led_bar.h"

// We'll reference these globals from main.c
extern int base_transition_period;
extern int BTP_multiplier;
extern char new_key;
extern char curr_num;
extern bool locked;
extern bool num_update;
extern bool reset_pattern;

// arrays used for pattern logic
const char COUNT_3_ARRAY[] = {0x18, 0x24, 0x42, 0x81, 0x42, 0x24};
#define COUNT_3_ARRAY_MAX_INDEX 5

// pointers and counts used for the pattern logic
// each is assigned to the last value possible so when starting its
// routine it ends up starting at the beginning
unsigned char count_1 = ~0xAA;
unsigned char count_2 = 255;
unsigned char count_3 = COUNT_3_ARRAY_MAX_INDEX;
unsigned char count_4 = 0;

void led_bar_update(unsigned char update_value){
    // update LED bar with pin mapping for P1.0, P1.4, P1.5, P1.6, P1.7, P1.1, P2.6, P2.7
    P1OUT = (P1OUT & ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7)) | \
            (((update_value) & BIT0) << 0) | \
            (((update_value) & (BIT1 | BIT2 | BIT3 | BIT4)) << 3) | \
            (((update_value) & BIT5) >> 4); \
    P2OUT = (P2OUT & ~(BIT6 | BIT7)) | \
            (((update_value) & (BIT6 | BIT7)) << 0);
}


void led_bar_init(void)
{
    // Configure LED bar pins as outputs: P1.0, P1.4, P1.5, P1.6, P1.7, P1.1, P2.6, P2.7
    P1SEL0 &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);  // Clear SEL0 for GPIO
    P1SEL1 &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);  // Clear SEL1 for GPIO
    P1DIR |= BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7;      // Set as output
    P1OUT &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);   // Initialize low

    P2SEL0 &= ~(BIT6 | BIT7);  // Clear SEL0 for GPIO
    P2SEL1 &= ~(BIT6 | BIT7);  // Clear SEL1 for GPIO
    P2DIR |= BIT6 | BIT7;      // Set as output
    P2OUT &= ~(BIT6 | BIT7);   // Initialize low
}

// ----------------------------------------------------------------------------
// led_bar_update_pattern:
//   Single switch case over curr_num, cases for 0-7
//   Set BTP_multiplier = 4 in the respective value
//   React to reset_pattern
//   Then update the pattern accordingly
// ----------------------------------------------------------------------------
void led_bar_update_pattern(void)
{
    switch (curr_num)
    {
        case '0':
            BTP_multiplier = 4;

            led_bar_update(0xAA);   // display 10101010
            reset_pattern = false;
            break;

        case '1':
            BTP_multiplier = 4;
            if (reset_pattern)
            {
                count_1 = 0xAA;                
                reset_pattern = false;
            }
            else
            {
                count_1 = ~count_1;
            }

            led_bar_update(count_1);    // display either 10101010 or 01010101
            break;

        case '2':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                count_2 = 0;
                reset_pattern = false;
            }
            else
            {
                count_2++;              // unsigned expressions cannot overflow, this will automatically
                                        // roll to 0 at 255
            }

            led_bar_update(count_2);    // display binary count from 0 to 255
            break;

        case '3':
            BTP_multiplier = 2;
            if (reset_pattern)
            {
                count_3 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_3 < COUNT_3_ARRAY_MAX_INDEX) ? (count_3++) : (count_3 = 0);
            }

            led_bar_update(COUNT_3_ARRAY[count_3]);    // display two led's boucing against each other
            break;

        case '4':
            BTP_multiplier = 1;
            if (reset_pattern)
            {
                count_4 = 0;
                reset_pattern = false;
            }
            else
            {
                count_4--;              // unsigned expressions cannot overflow, this will automatically
                                        // roll to 255 at 0
            }

            led_bar_update(count_4);    // display binary count from 255 to 0
            break;

        default:
            reset_pattern = false;
            break;
    }

    return;
}

// ----------------------------------------------------------------------------
// led_bar_delay: 
// Run a delay loop for the appropriate amount of time, kicking out if system gets locked or gets a new pattern
// ----------------------------------------------------------------------------
void led_bar_delay(void)
{
    // local variable to keep track of how long the delay is
    int loop_count = base_transition_period * BTP_multiplier / 4;

    // while loop_count > 0
    while (loop_count > 0)
    {
        // If locked == true, return out of the function
        if (locked == true)
        {
            return;
        }

        // If num_update == true, return out of the function
        if (num_update == true)
        {
            num_update = false;
            return;
        }

        // else, delay for 1/16th of a second
        // (~1 MHz clock => 62500 cycles ~1/16s minus around 1400 to adjust)
        __delay_cycles(61100);

        // decrement loop_count
        loop_count--;
    }
}
