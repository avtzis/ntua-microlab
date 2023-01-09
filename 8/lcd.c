#include "lcd.h"

void cursor_increment(int steps) {
    __cursor_position__ += steps;
}

int cursor_overflow() {
    return (__cursor_position__ > DISPLAY_LINE_1_END && __cursor_position__ < DISPLAY_LINE_1_START) || __cursor_position__ > DISPLAY_LINE_2_END;
}

void cursor_switch_line() {
    __cursor_position__ = __cursor_position__ < DISPLAY_LINE_2_START ? DISPLAY_LINE_2_START : DISPLAY_LINE_1_START;
}

void send_pulse() {
    set_bit(&PORTD, PD3);
    clear_bit(&PORTD, PD3);
}

void write_2_nibbles(reg_t reg) {
    char temp1 = reg & 0xF0, temp2 = PIND & 0x0F;
    temp1 += temp2;
    PORTD = temp1;
    send_pulse();
    temp1 = (reg & 0x0F) << 4;
    temp1 += temp2;
    PORTD = temp1;
    send_pulse();
}

void lcd_data(reg_t data) {
    set_bit(&PORTD, PD2);
    write_2_nibbles(data);
    _delay_ms(1);
    cursor_increment(1);
}

void lcd_command(reg_t command) {
    clear_bit(&PORTD, PD3);
    write_2_nibbles(command);
    _delay_ms(1);
}

void lcd_init() {
    DDRD=PORT_LCD_INIT;
    _delay_ms(50);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_4BIT;
    send_pulse();
    _delay_ms(1);

    lcd_command(CA_FUNCTION_SET_DISPLAY_DOUBLE_LINE);
    lcd_command(C4_DISPLAY_CONTROL_ON);
    lcd_command(C1_CLEAR_DISPLAY);
    _delay_ms(2);

    lcd_command(C3_ENTRY_MODE_ADDRESS_AUTO_INCREMENT);
    _delay_ms(2);

    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear() {
    lcd_command(C1_CLEAR_DISPLAY);
    _delay_ms(2);
    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear_line_1() {
    lcd_command(DISPLAY_SWITCH_TO_LINE_1);
    for(int i=0; i<LCD_LINE_LENGTH; ++i)
        lcd_data(' ');

    lcd_command(DISPLAY_SWITCH_TO_LINE_1);
    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear_line_2() {
    lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    for(int i=0; i<LCD_LINE_LENGTH; ++i)
        lcd_data(' ');

    lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    __cursor_position__ = DISPLAY_LINE_2_START;
}

void __lcd_display__(const char* display, int length) {
    int i;
    if(display == NULL) return;
    if(length > LCD_LINE_LENGTH * 2) {
        length = LCD_LINE_LENGTH * 2;
    }
    for(i=0; i<length && !cursor_overflow(); ++i)
        lcd_data(display[i]);

    if(cursor_overflow()) {
        cursor_switch_line();
        __lcd_display__(display + i, length - i)
    }
}

void lcd_display(const char* display) {
    __lcd_display__(display, strlen(display));
}

void lcd_clear_and_display(const char* display) {
    lcd_clear();
    lcd_display(display);
}

void lcd_display_line_1(const char* display) {
    int length;
    lcd_clear_line_1();
    if(length = strlen(display) > 16) length = 16;
    __lcd_display__(display, length);
}

void lcd_display_line_1_only(const char* display) {
    int length;
    lcd_clear_line_2();
    lcd_clear_line_1();
    if(length = strlen(display) > 16) length = 16;
    __lcd_display__(display, length);
}

void lcd_display_line_2(const char* display) {
    int length;
    lcd_clear_line_2();
    if(length = strlen(display) > 16) length = 16;
    __lcd_display__(display, length);
}

void lcd_display_line_2_only(const char* display) {
    int length;
    lcd_clear_line_1();
    lcd_clear_line_2();
    if(length = strlen(display) > 16) length = 16;
    __lcd_display__(display, length);
}

void lcd_display_both_lines(const char* display1, const char* display2) {
    lcd_display_line_1(display1);
    lcd_display_line_2(display2);
}