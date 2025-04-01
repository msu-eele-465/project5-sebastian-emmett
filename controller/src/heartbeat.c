#include <msp430.h>
#include <stdbool.h>
#include "heartbeat.h"

// ----------------------------------------------------------------------------
// init_heartbeat: Toggle P1.0 ~ once per second using Timer_B
// ----------------------------------------------------------------------------
void init_heartbeat(void)
{
    // P1.0 as output, off initially
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // Configure Timer_B0 for ~1Hz if SMCLK ~1MHz
    // SMCLK/8, up mode
    TB0CTL  = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;
    // further /8 => total /64
    TB0EX0  = TBIDEX__8;
    // 1 second at ~1 MHz/64
    TB0CCR0 = 15625;
    // enable CCR0 interrupt :D
    TB0CCTL0= CCIE;
}

// ----------------------------------------------------------------------------
// Timer_B0 ISR => toggles P1.0 (heartbeat)
// ----------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
    // Toggle heartbeat LED!
    P1OUT ^= BIT0;

    // Decrement pass_timer if we're in unlocking mode
    // We'll refer to unlocking from main
    extern bool unlocking;
    extern volatile int pass_timer;

    if (unlocking && pass_timer > 0)
    {
        pass_timer--;
    }
}
