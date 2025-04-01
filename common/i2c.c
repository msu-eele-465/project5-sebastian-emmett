#include <msp430.h>
#include <stdbool.h>
#include <stdint.h>
#include "i2c.h"
#include "intrinsics.h"

// Helpful Brock video :D https://www.youtube.com/watch?v=BvITEarUMkc

// Volatile variables for I2C slave receive handling
// Buffer to store up to 2 received bytes
volatile char rx_buffer[2];
// Number of bytes received in current transaction
volatile uint8_t rx_count = 0;
// Flag for completed transaction
volatile bool transaction_complete = false;
// Number of bytes in last completed transaction
volatile uint8_t rx_bytes = 0;

// Variables for master transmit (unchanged)
// Data to send
volatile char tx_data;
// Temperature whole value to send
volatile int8_t tx_temp_whole = 0;
// Temperature tenths to send
volatile int8_t tx_temp_tenths = 0;
// Transmission complete flag for temp
volatile bool i2c_tx_temp_complete = true;
// Transmission partially complete flag
volatile bool i2c_tx_temp_partial = false;
// Transmission complete flag
volatile bool i2c_tx_complete = true;

void i2c_master_init(void)
{
    // Configure USCI_B0 for I2C master mode
    // Put module in reset
    UCB0CTLW0 |= UCSWRST;
    // Use SMCLK (ensure SMCLK is ~1MHz for UCB0BRW=10)
    UCB0CTLW0 |= UCSSEL_3;
    // Prescaler for ~100kHz I2C clock
    UCB0BRW = 10;
    // I2C mode
    UCB0CTLW0 |= UCMODE_3;
    // Master mode
    UCB0CTLW0 |= UCMST;
    // Transmitter mode
    UCB0CTLW0 |= UCTR;

    // Auto STOP after UCB0TBCNT bytes
    UCB0CTLW1 |= UCASTP_2;
    // Send 1 byte per transaction
    UCB0TBCNT = 1;
    
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    // P1.3 SCL
    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;

    // P1.2 SDA
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;

    // Clear reset to enable module
    UCB0CTLW0 &= ~UCSWRST;

    // Enable interrupts for TX
    // Local enable for TX0
    UCB0IE |= UCTXIE0;
    // Enable maskables
    __enable_interrupt();
}

void i2c_slave_init(uint8_t address)
{
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    // P1.3 SCL
    P1SEL1 &= ~BIT3;
    P1SEL0 |= BIT3;
    // P1.2 SDA
    P1SEL1 &= ~BIT2;
    P1SEL0 |= BIT2;

    // Configure USCI_B0 for I2C slave mode
    // Put module in reset
    UCB0CTLW0 |= UCSWRST;
    // I2C mode, slave, synchronous
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;
    // Set own address and enable
    UCB0I2COA0 = address | UCOAEN;
    // Clear reset to enable module
    UCB0CTLW0 &= ~UCSWRST;

    // Enable receive interrupt
    // Enable RX interrupt for slave
    UCB0IE |= UCRXIE;
}

void i2c_send(uint8_t slave_address, char data)
{
    // Send 1 byte per transaction
    UCB0TBCNT = 1;
    // Toggle tx complete flag false
    i2c_tx_complete = false;
    // Store data for ISR
    tx_data = data;
    // Set slave address
    UCB0I2CSA = slave_address;
    // Generate START
    UCB0CTLW0 |= UCTXSTT;
}

uint8_t i2c_get_received_data(char* data)
{
    if (transaction_complete)
    {
        if (rx_bytes == 1)
        {
            data[0] = rx_buffer[0];
        }
        else if (rx_bytes == 2)
        {
            data[1] = rx_buffer[0];
            data[2] = rx_buffer[1];
            rx_bytes = 0;
        }
        transaction_complete = false;
        return rx_bytes;
    }
    else
    {
        return 0;
    }
}

void i2c_send_temp(int16_t temp)
{
    // while (!i2c_tx_complete || !i2c_tx_temp_complete); // Wait for idle state
    // Send 2 bytes per transaction
    UCB0TBCNT = 2;
    // Toggle tx complete flag false
    i2c_tx_temp_complete = false;
    // Calculate the whole number from our given temperature
    tx_temp_whole = temp / 10;
    // Calculate the tenths number from our given temperature
    tx_temp_tenths = temp % 10;
    // Set slave address
    UCB0I2CSA = SLAVE1_ADDR;
    // Generate START
    UCB0CTLW0 |= UCTXSTT;
}

void i2c_send_to_both(char data)
{
    // Send to first slave
    i2c_send(SLAVE1_ADDR, data);
    //int i = 0;
    //for (i = 1; i < 100; i++);   // Yeahhh LOL this is not how this is supposed to be done but the interrupt flags are giving me such a headache that I've just moved away from relying on the stop flag and resorted to this lol
    //i2c_send(SLAVE2_ADDR, data);       // Send to second slave
}

#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void)
{
    // Start condition detected
    if (UCB0IFG & UCSTTIFG)
    {
        rx_count = 0;
        // Clear start condition flag
        UCB0IFG &= ~UCSTTIFG;
    }
    // Byte received
    if (UCB0IFG & UCRXIFG)
    {
        if (rx_count < 2)
        {
            // Clears UCRXIFG
            rx_buffer[rx_count] = UCB0RXBUF;
            rx_count++;
        }
        else
        {
            // Discard extra bytes
            UCB0RXBUF;
        }
    }
    // Stop condition detected
    if (UCB0IFG & UCSTPIFG)
    {
        transaction_complete = true;
        rx_bytes = rx_count;
        // Clear stop condition flag
        UCB0IFG &= ~UCSTPIFG;
    }

    // Master transmit handling
    // Master transmit interrupt
    if (UCB0IFG & UCTXIFG0)
    {
        if (i2c_tx_complete == false)
        {
            // Send data for master
            UCB0TXBUF = tx_data;
            // Mark transmission complete
            i2c_tx_complete = true;
        }
        if (i2c_tx_temp_complete == false)
        {
            if (!i2c_tx_temp_partial)
            {
                // Send temperature whole number
                UCB0TXBUF = tx_temp_whole;
                // Mark partial transmission complete
                i2c_tx_temp_partial = true;
            } else {
                // Send temperature tenths number
                UCB0TXBUF = tx_temp_tenths;
                // Mark transmission complete
                i2c_tx_temp_partial = false;
                i2c_tx_temp_complete = true;
            }
            
        }
        
    }
}
