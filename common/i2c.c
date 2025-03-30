#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "i2c.h"
#include "intrinsics.h"

// Helpful Brock video :D https://www.youtube.com/watch?v=BvITEarUMkc

volatile char i2c_received_data;
volatile bool i2c_data_ready = false;

volatile char tx_data;              // Data to send
volatile int8_t tx_temp_whole = 0;      // Temperature whole value to send
volatile int8_t tx_temp_tenths = 0;     // Temperature tenths to send

volatile bool i2c_tx_temp_complete = true; // Transmission complete flag for temp transmission (true when idle)
volatile bool i2c_tx_temp_partial = false; // Transmission partially complete flag for temp transmission
volatile bool i2c_tx_complete = true; // Transmission complete flag (true when idle)

void i2c_master_init(void)
{

    // Configure USCI_B0 for I2C master mode
    UCB0CTLW0 |= UCSWRST;     // Put module in reset
    UCB0CTLW0 |= UCSSEL_3;    // Use SMCLK (ensure SMCLK is ~1MHz for UCB0BRW=10)
    UCB0BRW = 10;             // Prescaler for ~100kHz I2C clock
    UCB0CTLW0 |= UCMODE_3;    // I2C mode
    UCB0CTLW0 |= UCMST;       // Master mode
    UCB0CTLW0 |= UCTR;        // Transmitter mode

    UCB0CTLW1 |= UCASTP_2;    // Auto STOP after UCB0TBCNT bytes
    UCB0TBCNT = 1;            // Send 1 byte per transaction
    
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    P1SEL1 &= ~BIT3;          // P1.3 SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;          // P1.2 SDA
    P1SEL0 |= BIT2;

    UCB0CTLW0 &= ~UCSWRST;    // Clear reset to enable module

    // Enable interrupts for TX
    UCB0IE |= UCTXIE0;        // Local enable for TX0
    __enable_interrupt();     // Enable maskables
}

void i2c_slave_init(uint8_t address)
{
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    P1SEL1 &= ~BIT3;          // P1.3 SCL
    P1SEL0 |= BIT3;
    P1SEL1 &= ~BIT2;          // P1.2 SDA
    P1SEL0 |= BIT2;

    // Configure USCI_B0 for I2C slave mode
    UCB0CTLW0 |= UCSWRST;     // Put module in reset
    UCB0CTLW0 |= UCMODE_3 | UCSYNC; // I2C mode, slave, synchronous
    UCB0I2COA0 = address | UCOAEN;  // Set own address and enable
    UCB0CTLW0 &= ~UCSWRST;    // Clear reset to enable module

    // Enable receive interrupt
    UCB0IE |= UCRXIE;         // Enable RX interrupt for slave
}

void i2c_send(uint8_t slave_address, char data)
{
    UCB0TBCNT = 1;             // Send 1 byte per transaction
    i2c_tx_complete = false;   // Toggle tx complete flag false
    tx_data = data;            // Store data for ISR
    UCB0I2CSA = slave_address; // Set slave address
    UCB0CTLW0 |= UCTXSTT;      // Generate START
}

bool i2c_get_received_data(char* data)
{
    if (i2c_data_ready)
    {
        *data = i2c_received_data;
        i2c_data_ready = false;
        return true;
    }
    return false;
}

void i2c_send_temp(int16_t temp)
{
    // while (!i2c_tx_complete || !i2c_tx_temp_complete); // Wait for idle state
    UCB0TBCNT = 2;                // Send 2 bytes per transaction
    i2c_tx_temp_complete = false; // Toggle tx complete flag false
    tx_temp_whole = temp / 10;    // Calculate the whole number from our given temperature
    tx_temp_tenths = temp % 10;   // Calculate the tenths number from our given temperature
    UCB0I2CSA = SLAVE2_ADDR;      // Set slave address
    UCB0CTLW0 |= UCTXSTT;         // Generate START
}

void i2c_send_to_both(char data)
{
    i2c_send(SLAVE1_ADDR, data);       // Send to first slave
    //int i = 0;
    //for (i = 1; i < 100; i++);   // Yeahhh LOL this is not how this is supposed to be done but the interrupt flags are giving me such a headache that I've just moved away from relying on the stop flag and resorted to this lol
    //i2c_send(SLAVE2_ADDR, data);       // Send to second slave
}

#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void)
{
    if (UCB0IFG & UCRXIFG)  // Slave receive interrupt
    {
        i2c_received_data = UCB0RXBUF;  // Read received data
        i2c_data_ready = true;          // Set flag to indicate new data
    }
    else if (UCB0IFG & UCTXIFG0)  // Master transmit interrupt
    {
        if (i2c_tx_complete == false)
        {
            UCB0TXBUF = tx_data;  // Send data for master
            i2c_tx_complete = true;  // Mark transmission complete
        }
        if (i2c_tx_temp_complete == false)
        {
            if (!i2c_tx_temp_partial)
            {
                UCB0TXBUF = tx_temp_whole; // Send temperature whole number
                i2c_tx_temp_partial = true;  // Mark partial transmission complete
            } else {
                UCB0TXBUF = tx_temp_tenths; // Send temperature tenths number
                i2c_tx_temp_partial = false; // Mark transmission complete
                i2c_tx_temp_complete = true;
            }
            
        }
        
    }
}
