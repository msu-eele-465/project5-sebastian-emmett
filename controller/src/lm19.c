#include <msp430.h>
#include "lm19.h"
#include "msp430fr2355.h"

#define MAX_WINDOW 20

static int16_t queue[MAX_WINDOW];
static uint8_t window_size = 3;
static uint8_t num_samples = 0;
static uint8_t index = 0;
static int32_t sum = 0;
static int16_t temp_to_send = 0;
static int16_t average = 0;


void temp_sensor_init(void)
{
    // Configure P1.4 as ADC input (A4)
    P1SEL0 |= BIT4;
    P1SEL1 |= BIT4;

    // Configure ADC
    // Set conv clock cycles = 16 (10)
    ADCCTL0 &= ~ADCSHT;
    ADCCTL0 |= ADCSHT_2;
    // Turn on ADC
    ADCCTL0 |= ADCON;

    // Choose SMCLK
    ADCCTL1 |= ADCSSEL_2;
    // Sample singal source = sampling timer
    ADCCTL1 |= ADCSHP;

    // Clear resolution
    ADCCTL2 &= ~ADCRES;
    // Set 12-bit resolution
    ADCCTL2 |= ADCRES_2;

    // ADC input = A4 (P1.4)
    ADCMCTL0 |= ADCINCH_4;

    // Configure Timer 2 B0 for ~2Hz if SMCLK ~1MHz
    // SMCLK/8, up mode
    TB2CTL = TBSSEL__SMCLK | ID__8 | MC__UP | TBCLR;
    // further /8 => total /64
    TB2EX0 = TBIDEX__8;
    // 1 second at ~1 MHz/128
    TB2CCR0 = 7812;
    // enable interrupt :D
    TB2CCTL0 = CCIE;

    // Enable global interrupts
    __enable_interrupt();
}

void set_window_size(uint8_t size)
{
    if (size > MAX_WINDOW) size = MAX_WINDOW;
    if (size < 1) size = 1;
    window_size = size;
    num_samples = 0;
    index = 0;
    sum = 0;
}

static void add_to_queue(int16_t value)
{
    if (num_samples < window_size)
    {
        queue[index] = value;
        sum += value;
        index = (index + 1) % window_size;
        num_samples++;
    }
    else
    {
        sum -= queue[index];
        queue[index] = value;
        sum += value;
        index = (index + 1) % window_size;
    }
}

static int16_t get_average(void)
{
    if (num_samples == 0) return 0;
    return (int16_t)(sum / num_samples);
}

#pragma vector=TIMER2_B0_VECTOR
__interrupt void TIMER2_B0_ISR(void)
{
    // Eanble and start conversion
    ADCCTL0 |= ADCENC | ADCSC;
    // Wait for conversion to complete
    while ((ADCIFG & ADCIFG0) == 0);
    long adc_value = ADCMEM0;

    // Compute voltage in mV: Vin = (ADC_value * 3300) / 4095
    long V_in_mV = (adc_value * 3300) / 4095;

    // Compute temperature in tenths of a degree Celsius
    // T(tenths) (°C) = ((Vin(mV) * 100) - 186630) / -117
    long numerator = (V_in_mV * 100) - 186630;
    int16_t temp_tenths = (int16_t)(numerator / -117);

    // Add to averaging queue
    add_to_queue(temp_tenths);

    // Only send average when window is full
    if (num_samples == window_size)
    {
        average = get_average();
        if (fahrenheit_mode)
        {
            // T_F = T_C * 9/5 + 320, in tenths
            temp_to_send = (average * 9 / 5) + 320;
        }
        else
        {
            temp_to_send = average;
        }
        i2c_send_temp(temp_to_send);
    }
}
