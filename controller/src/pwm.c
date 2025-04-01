#include <msp430.h>

void pwm_init(void)
{
    // setup Timer3B
    // clear timer and dividers
    TB3CTL |= TBCLR;
    // select 16-bit counter length
    TB3CTL |= CNTL__16;
    // source = SMCLK (1 MHz)
    TB3CTL |= TBSSEL__SMCLK;
    // count in up mode (for timer compares)
    TB3CTL |= MC__UP;

    // set capture compare register information
    TB3CCR0 = 10000;
    TB3CCR1 = 1000;
    TB3CCR2 = 1000;
    TB3CCR3 = 1000;

    // set both dividers to 1
    TB3CTL |= ID__1;
    TB3EX0 |= TBIDEX__1;

    // setup CCR's to automatically update pins P6.0 - P6.2
    TB3CCTL1 |= OUTMOD_7;
    TB3CCTL2 |= OUTMOD_7;
    TB3CCTL3 |= OUTMOD_7;

    // clear output
    P6OUT &= ~(BIT0 | BIT1 | BIT2);
    // set to an output
    P6DIR |= BIT0 | BIT1 | BIT2;

    // setup P6.0 - P6.2 to accept automatic updates
    P6SEL0 |= BIT0 | BIT1 | BIT2;
    P6SEL1 &= ~(BIT0 | BIT1 | BIT2);
}
