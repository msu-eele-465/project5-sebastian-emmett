#ifndef LED_BAR_H
#define LED_BAR_H

// configure LED bar pins as outputs: P1.0, P1.4 - P1.7, P1.1, P2.6 - P2.7
// these are in order of the outputs on the LED bar, P1.1 was originally P2.0
// 	but has been remapped due to hardware issues.
void led_bar_init(void);

// update the output displayed on the LED bar
void led_bar_update(unsigned char update_value);

// update the pattern logic on the LED bar, this may entail resetting a
// 	pattern or switching to a new one.
void led_bar_update_pattern(void);

// a delay which happens between pattern updates, can be interrupted by the controller
void led_bar_delay(void);

#endif
