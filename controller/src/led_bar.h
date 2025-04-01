#ifndef LED_BAR_H
#define LED_BAR_H

void led_bar_init(void);

void led_bar_update_pattern(void);

void led_bar_delay(void);

#define led_bar_update(update_value)\
    P3OUT = 0;\
    P3OUT |= update_value

#endif
