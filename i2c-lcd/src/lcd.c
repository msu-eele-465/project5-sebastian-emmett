#include <msp430fr2310.h>
#include <stdint.h>

#include "./lcd.h"


/* --- internal variables --- */


// used to control 3 options on the lcd:
//  display on/off, cursor, blink
static uint8_t display_ctrl;


/* --- init --- */


void lcd_init(void)
{
	// set P1.4 - P1.7, P2.0, P2.6, and P2.7 as outputs
	P1SEL0 &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1SEL1 &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1DIR |= BIT4 | BIT5 | BIT6 | BIT7;

	P2SEL0 &= ~(BIT0 | BIT6 | BIT7);
	P2SEL1 &= ~(BIT0 | BIT6 | BIT7);
	P2OUT &= ~(BIT0 | BIT6 | BIT7);
	P2DIR |= BIT0 | BIT6 | BIT7;

	// assuming each cycle is 1 us then 50 ms is 50000 us
	__delay_cycles(50000);

	// incomplete function set, this will tell the lcd to use
	//  4-bit mode, but we will be sending it in '8-bit' mode
	lcd_set_mode(0, 0);

	lcd_clock_e();

	// function set; 4-bit mode
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= BIT5;

	lcd_clock_e();
	__delay_cycles(100);

	// now another function set must be performed, but this time
	//  it is in 4-bit mode
	// function set; 4-bit mode, 2-line mode, 5x8 font
	lcd_cmd_inst(0x28);

	// display control; display on, cursor off, blink off
	display_ctrl = 0x0C;
	lcd_cmd_inst(display_ctrl);
	
	lcd_clear_display();

	// entry mode set; increment, no shift
	lcd_cmd_inst(0x06);
}


/* --- general use --- */


void lcd_print_line(const char *line_chars, uint8_t line_num)
{
	// set the cursor to the beginning of the selected line
	if (line_num)
	{
		lcd_set_ddram_addr(0x40);
	}
	else
	{
		lcd_set_ddram_addr(0x00);
	}

	const char *line_chars_end = line_chars + 16;

	while (line_chars < line_chars_end)
	{
		lcd_cmd_write((uint8_t) *line_chars++);
	}

	// set the cursor to the last character of the current line
	if (line_num)
	{
		lcd_set_ddram_addr(0x4F);
	}
	else
	{
		lcd_set_ddram_addr(0x0F);
	}
}

void lcd_update_current_key(void)
{
	lcd_set_ddram_addr(0x4F);

	lcd_cmd_write((uint8_t) curr_key);

	lcd_set_ddram_addr(0x4F);
}

void lcd_create_character(const char *bitmap, uint8_t index)
{
	lcd_set_cgram_addr(index, 0);

	// write the bitmap into the selected spot
	const char *bitmap_end = bitmap + 8;

	while (bitmap < bitmap_end)
	{
		lcd_cmd_write((uint8_t) *bitmap++);
	}

	// and set the cursor to the last display position
	//	because if it is not set then the cursor would not display
	lcd_set_ddram_addr(0x4F);
}

void lcd_clear_display(void)
{
	lcd_set_mode(0, 0);

	lcd_cmd_send(0x01);

	// 3ms = 3000us
	__delay_cycles(3000);
}

void lcd_toggle_cursor(void)
{
	display_ctrl ^= BIT1;
	lcd_cmd_inst(display_ctrl);
}

void lcd_toggle_blink(void)
{
	display_ctrl ^= BIT0;
	lcd_cmd_inst(display_ctrl);
}


/* --- advanced use --- */


void lcd_clock_e(void)
{
	__delay_cycles(2);
	
	// toggle the enable bit
	P2OUT ^= BIT0;
	
	__delay_cycles(2);
}

void lcd_set_mode(uint8_t rs, uint8_t rw)
{
	if (rs)
	{
		P2OUT |= BIT6;
	}
	else
	{
		P2OUT &= ~BIT6;
	}

	if (rw)
	{
		P2OUT |= BIT7;
	}
	else
	{
		P2OUT &= ~BIT7;
	}

	__delay_cycles(2);
}

void lcd_set_cgram_addr(uint8_t index, uint8_t sub_index)
{
	lcd_set_mode(0, 0);

	// the index is shifted left three so it will select the
	//	appropriate character, and the sub_index will select
	//	the position within that character
	lcd_cmd_inst(0x40 | (index << 3 & 0x38) | (sub_index & 0x07));
}

void lcd_set_ddram_addr(uint8_t address)
{
	lcd_set_mode(0, 0);
	lcd_cmd_inst(0x80 | (address & 0x7F));
}

void lcd_cmd_send(uint8_t byte)
{
	lcd_clock_e();

	// update gpio with upper nibble
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= byte & 0xF0;

	lcd_clock_e();

	// after the upper nibble is sent the lower nibble
	//  is then sent since the lcd is in 4 bit mode
	lcd_clock_e();

	// update gpio with lower nibble
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= byte << 4;

	lcd_clock_e();
}

void lcd_cmd_inst(uint8_t byte)
{
	lcd_set_mode(0, 0);

	lcd_cmd_send(byte);

	__delay_cycles(100);
}

void lcd_cmd_write(uint8_t byte)
{
	// RS set, R/W cleared
	lcd_set_mode(1, 0);

	lcd_cmd_send(byte);

	__delay_cycles(100);
}
