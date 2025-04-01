#ifndef KEYBOARD_H
#define KEYBOARD_H

void init_keypad(void);
char poll_keypad(void);
// LED on P6.6
void init_responseLED(void);
// Timer_B1 => ~50 ms
void init_keyscan_timer(void);

#endif
