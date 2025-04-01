#include <msp430.h>
#include "../src/pwm.h"

void rgb_set(unsigned char red_intensity, unsigned char green_intensity, unsigned char blue_intensity)
{
    // Since certain colors use more current or eyes can be more sensitive
    // the pwm ratios are tuned slightly through multiplication
    pwm_set_duty_ccr1((unsigned short)red_intensity * 12);
    pwm_set_duty_ccr2((unsigned short)green_intensity * 6);
    pwm_set_duty_ccr3((unsigned short)blue_intensity * 8);
}
