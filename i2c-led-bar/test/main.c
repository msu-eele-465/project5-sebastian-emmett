#include <msp430fr2310.h>
#include <stdbool.h>
#include "../src/i2c.h"

// Test main.c file for recieving i2c.

#define SLAVE_ADDRESS SLAVE1_ADDR  // Use SLAVE1_ADDR from i2c.h

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set P1.1 as output for LED, P1.0 as output for debug
    P1DIR |= BIT1 | BIT0;
    P1OUT &= ~(BIT1 | BIT0);  // Start both low

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    // Initialize I2C as slave
    i2c_slave_init(SLAVE_ADDRESS);

    // Note: Interrupt is disabled in i2c_slave_init by removing UCB0IE |= UCRXIE;

    while (true)
    {
        if (UCB0IFG & UCRXIFG)  // Poll the receive interrupt flag
        {
            char received = UCB0RXBUF;  // Read the received data
            if (received == '1')
            {
                P1OUT |= BIT1;  // Turn LED on
            }
            else if (received == '2')
            {
                P1OUT &= ~BIT1; // Turn LED off
            }
        }
    }
}