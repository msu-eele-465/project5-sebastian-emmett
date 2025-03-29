#ifndef PWM_H
#define PWM_H

// CCRn based pwm on TimerB3
// there is no ISR for this since it outputs directly to P6.0 - P6.2
void pwm_init(void);

// update CCR1 for TimerB3 to the new on pulse width
// in other words, update the duty cycle
#define pwm_set_duty_ccr1(new_pulse_width) (TB3CCR1 = new_pulse_width)

// update CCR2 for TimerB3 to the new on pulse width
#define pwm_set_duty_ccr2(new_pulse_width) (TB3CCR2 = new_pulse_width)

// update CCR3 for TimerB3 to the new on pulse width
#define pwm_set_duty_ccr3(new_pulse_width) (TB3CCR3 = new_pulse_width)

// update CCR0 for TimerB3 to the new period
#define pwm_set_period(new_period) (TB3CCR0 = new_period)

#endif
