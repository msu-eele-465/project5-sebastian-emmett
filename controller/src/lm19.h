#ifndef LM19_H
#define LM19_H

#include <stdint.h>
#include <stdbool.h>
#include <../common/i2c.h>

void temp_sensor_init(void);
void set_window_size(uint8_t size);

extern bool fahrenheit_mode;

#endif
