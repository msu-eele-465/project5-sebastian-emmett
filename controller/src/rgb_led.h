#ifndef RGB_LED_H
#define RGB_LED_H

// values for red, green, and blue will be used to update the appropriate CCRn register
// NOTE: do not call this without first calling pwm_init
void rgb_set(char red_intensity, char green_intensity, char blue_intensity);

#endif
