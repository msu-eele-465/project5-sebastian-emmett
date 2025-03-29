#ifndef LCD_H
#define LCD_H

#include <stdint.h>

/* --- external variables --- */


// this is used by lcd_update_current_char and lcd_print_line
// 	it should be defined in whatever includes lcd.h
extern char curr_key;


/* --- init --- */


// go through the init routine for the lcd, this takes about 50ms
// will set P1.4 - P1.7, P2.0, P2.6, and P2.7 as outputs
void lcd_init(void);

/* --- general use --- */


// choose a line and print the given characters to it
// the cursor will appear at the end of the given line
void lcd_print_line(const char *line_chars, uint8_t line_num);

// display curr_key at the end of line 1
// the cursor will also be set to the end of line 1
void lcd_update_current_key(void);

// create a custom character at the specified index using a provided bitmap
// the index may be a value within [0, 7]
// when printing these values the same index can be used
//  i.e., "\x00" would print custom character 0
//   "\x01" would print custom character 1
//
//  after calling this function the cursor is set to the last display position
void lcd_create_character(const char *bitmap, uint8_t index);

// clear the lcd's display
void lcd_clear_display(void);

// toggle the lcd's cursor between on or off
void lcd_toggle_cursor(void);

// toggle the lcd's cursor blink between on or off
void lcd_toggle_blink(void);


/* --- advanced use --- */


// toggles the current value for e, which is stored in P2.0
void lcd_clock_e(void);

// set the RS and R/W bits accordingly
void lcd_set_mode(uint8_t rs, uint8_t rw);

// provide an index between 0 and 7 to move to in the CGRAM
// 	each index corresponds to a unique custom character
// the sub_index corresponds to a specific line in the bitmap
void lcd_set_cgram_addr(uint8_t index, uint8_t sub_index);

// provide an address 0x00 - 0x7F to move to in the DDRAM
void lcd_set_ddram_addr(uint8_t address);

// send the input data to the lcd in 4-bit mode
// the RS and R/W bits should be appropriately set before calling
void lcd_cmd_send(uint8_t byte);

// send an lcd command instruction (command with RS and R/W cleared)
void lcd_cmd_inst(uint8_t byte);

// send an lcd write instruction (command with RS set and R/W cleared)
void lcd_cmd_write(uint8_t byte);

#endif
