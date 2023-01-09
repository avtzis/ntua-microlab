#ifndef _LCD_H
#define _LCD_H 1

#include "avrlib.h"
#include <string.h>

#define PORT_LCD_INIT OUTPUT
#define PORT_COMMAND_SET_LCD_8BIT 0x30
#define PORT_COMMAND_SET_LCD_4BIT 0x20

#define LCD_LINE_LENGTH 16
#define LCD_DEGREE_CIRCLE 223

#define C1_CLEAR_DISPLAY 0x01

#define C2_RETURN_HOME 0x02

#define C3_ENTRY_MODE_SHIFT_ENTIRE_DISPLAY 0x05
#define C3_ENTRY_MODE_ADDRESS_AUTO_INCREMENT 0x06

#define C4_DISPLAY_CONTROL_CURSOR_BLINKING 0x09
#define C4_DISPLAY_CONTROL_CURSOR 0x0A
#define C4_DISPLAY_CONTROL_ON 0x0C

#define C6_CURSOR_MOVE_LEFT 0x10
#define C7_CURSOR_MOVE_RIGHT 0x14
#define C8_DISPLAY_SHIFT_LEFT 0x18
#define C9_DISPLAY_SHIFT_RIGHT 0x1C

#define CA_FUNCTION_SET_DISPLAY_FONT_5_11 0x24
#define CA_FUNCTION_SET_DISPLAY_DOUBLE_LINE 0x28
#define CA_FUNCTION_SET_INTERFACE_DATA_LENGTH_8BIT 0x30

#define DISPLAY_SWITCH_TO_LINE_1 0x80
#define DISPLAY_SWITCH_TO_LINE_2 0xC0

#define DISPLAY_LINE_1_START 0x00
#define DISPLAY_LINE_1_END 0x0F
#define DISPLAY_LINE_2_START 0x40
#define DISPLAY_LINE_2_END 0x4F

unsigned __cursor_position__;
void cursor_increment(int steps);
int cursor_overflow();
void cursor_switch_line();

void send_pulse();
void write_2_nibbles(reg_t reg);
void lcd_data(reg_t data);
void lcd_command(reg_t command);
void lcd_init();

void lcd_clear();
void lcd_clear_line_1();
void lcd_clear_line_2();
void lcd_switch_line();
void __lcd_display__(const char* display, int length);

void lcd_display(const char* display);
void lcd_clear_and_display(const char* display);
void lcd_display_line_1(const char* display);
void lcd_display_line_1_only(const char* display);
void lcd_display_line_2(const char* display);
void lcd_display_line_2_only(const char* display);
void lcd_display_both_lines(const char* display1, const char* display2);

#endif