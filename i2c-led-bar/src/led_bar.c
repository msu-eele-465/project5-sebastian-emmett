#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "../src/led_bar.h"


/* --- globals --- */


// We'll reference these globals from main.c
extern int transition_period;
extern char new_key;
extern char curr_num;
extern bool locked;
extern bool num_update;
extern bool reset_pattern;

// Both of these are necessary for breaking out of the dalay loop
// Flag for completed transaction
extern volatile bool transaction_complete;
// Number of bytes in last completed transaction
extern volatile uint8_t rx_bytes;


/* --- pattern variables --- */


// arrays used for pattern logic
static const uint8_t ARRAY_3[] = {0x18, 0x24, 0x42, 0x81, 0x42, 0x24};
static const uint8_t ARRAY_5[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static const uint8_t ARRAY_6[] = {0x7F, 0xBF, 0xDf, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
static const uint8_t ARRAY_7[] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

// counts used for the pattern logic
// each is assigned to the last value possible so when starting its
// 	routine it ends up starting at the beginning
static uint8_t count_1 = ~0xAA;
static uint8_t count_2 = 255;
static uint8_t count_3 = 5;
static uint8_t count_4 = 0;
static uint8_t count_5 = 7;
static uint8_t count_6 = 7;
static uint8_t count_7 = 7;


/* --- program --- */


void led_bar_init(void)
{
    // configure LED bar pins as outputs: P1.0, P1.4, P1.5, P1.6, P1.7, P1.1, P2.6, P2.7

	// set as digital I/O
    P1SEL0 &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);
    P1SEL1 &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);
	// clear output
    P1OUT &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);
	// set as output
    P1DIR |= BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7;

	// set as digital I/O
    P2SEL0 &= ~(BIT6 | BIT7);
    P2SEL1 &= ~(BIT6 | BIT7);
	// clear output
    P2OUT &= ~(BIT6 | BIT7);
	// set as output
    P2DIR |= BIT6 | BIT7;
}

void led_bar_update(unsigned char update_value){
    // update LED bar following pin mapping of: P1.0, P1.4 - P1.7, P1.1, P2.6 - P2.7
    P1OUT = (P1OUT & ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7)) | \
            (((update_value) & BIT0) << 0) | \
            (((update_value) & (BIT1 | BIT2 | BIT3 | BIT4)) << 3) | \
            (((update_value) & BIT5) >> 4); \
    P2OUT = (P2OUT & ~(BIT6 | BIT7)) | \
            (((update_value) & (BIT6 | BIT7)) << 0);
}

void led_bar_update_pattern(void)
{
    curr_num = curr_num;
    switch (curr_num)
    {
		// display 10101010
        case '0':
			// no delay, since it doesn't matter here and improves response time
			transition_period = 0;

            led_bar_update(0xAA);
            reset_pattern = false;

            break;

		// switch between 10101010 and 01010101
        case '1':
			transition_period = 16;

            if (reset_pattern)
            {
                count_1 = 0xAA;                
                reset_pattern = false;
            }
            else
            {
                count_1 = ~count_1;
            }

            led_bar_update(count_1);
            break;

		// display binary count from 0 to 255
        case '2':
			transition_period = 8;

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

            led_bar_update(count_2);
            break;

		// display two led's bouncing against each other
        case '3':
			transition_period = 8;

            if (reset_pattern)
            {
                count_3 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_3 < 5) ? (count_3++) : (count_3 = 0);
            }

            led_bar_update(ARRAY_3[count_3]);
            break;

		// display binary count from 255 to 0
        case '4':
			transition_period = 4;

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

            led_bar_update(count_4);
            break;

		// set LED rolling across LED bar left
        case '5':
			transition_period = 24;

            if (reset_pattern)
            {
                count_5 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_5 < 7) ? (count_5++) : (count_5 = 0);
            }

            led_bar_update(ARRAY_5[count_5]);
            break;

		// 7 set LEDs rolling across LED bar right
        case '6':
			transition_period = 8;

            if (reset_pattern)
            {
                count_6 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_6 < 7) ? (count_6++) : (count_6 = 0);
            }

            led_bar_update(ARRAY_6[count_6]);
            break;

		// fill LED bar with sets from the right
        case '7':
			transition_period = 16;

            if (reset_pattern)
            {
                count_7 = 0;
                reset_pattern = false;
            }
            else
            {
                (count_7 < 7) ? (count_7++) : (count_7 = 0);
            }

            led_bar_update(ARRAY_7[count_7]);
            break;

        default:
            reset_pattern = false;
            break;
    }

    return;
}

void led_bar_delay(void)
{
    int loop_count = transition_period;

    while (loop_count > 0)
    {
        // if a char is received
        if (transaction_complete && (rx_bytes == 1))
        {
            return;
        }

        // else, delay for 1/16th of a second
        // (~1 MHz clock => 62500 cycles ~1/16s minus around 1400 to adjust)
        __delay_cycles(61100);

        // decrement loop_count
        loop_count--;
    }
}
