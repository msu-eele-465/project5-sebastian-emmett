#ifndef KEYBOARD_H
#define KEYBOARD_H

void init_keypad(void);
char poll_keypad(void);
void init_responseLED(void);  // LED on P6.6
void init_keyscan_timer(void); // Timer_B1 => ~50 ms

#endif
