#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

#define SLAVE1_ADDR 0x48
#define SLAVE2_ADDR 0x49

void i2c_master_init(void);
void i2c_slave_init(uint8_t address);
void i2c_send(uint8_t slave_address, char data);
bool i2c_get_received_data(char* data);
void i2c_send_to_both(char data);

#endif
