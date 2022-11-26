#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define INPUT 0x00
#define OUTPUT 0xff

#define DISPLAY_SHIFT_RIGHT 0x1c
#define DISPLAY_SHIFT_LEFT 0x18
#define DISPLAY_CLEAR 0x01
#define DISPLAY_SWITCH_TO_LINE_1 0x80
#define DISPLAY_SWITCH_TO_LINE_2 0xc0

#define VREF 5
#define DC_20 80

void send_pulse() {
    PORTD |= (1 << PD3);
    PORTD &= (254 << PD3);
}

void write_2_nibbles(char reg0) {
    char reg1 = reg0 & 0xf0, reg2 = PIND & 0x0f;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
    reg1 = (reg0 & 0x0f) << 4;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
}

void lcd_data(char data) {
    PORTD |= (1 << PD2);
    write_2_nibbles(data);
    _delay_ms(1);
}

void lcd_command(char command) {
    PORTD &= (254 << PD2);
    write_2_nibbles(command);
    _delay_ms(1);
}

void lcd_display(char *display) {
    int length = strlen(display);

    for(int i=0; i<length; ++i) {
        lcd_data(display[i]);
    }
}

void lcd_init() {
    _delay_ms(1);

    PORTD = 0x30;
    send_pulse();
    _delay_ms(1);

    PORTD = 0x30;
    send_pulse();
    _delay_ms(1);

    PORTD = 0x20;
    send_pulse();
    _delay_ms(1);

    lcd_command(0x28);
    lcd_command(0x0c);
    lcd_command(0x01);
    _delay_ms(2);

    lcd_command(0x06);
    _delay_ms(2);
}

void adc_init() {
    ADMUX = 0b01000000;
    ADCSRA = 0b10000111;
}

void io_init() {
    DDRB = INPUT;
    DDRC = INPUT;
    DDRD = OUTPUT;
}

short adc_convert() {
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

void enable_PWM() {
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 399;
}

void lcd_display_double(char *display1, char *display2) {
    lcd_command(DISPLAY_CLEAR);
    lcd_display(display1);

    lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    lcd_display(display2);
}

void convert_and_display() {
    unsigned short adc;
    unsigned short vin, vout;
    char display1[17], display2[16];

    adc = adc_convert();

    vin = ((((adc * VREF * 10) >> 5) * 10) >> 5);
    vout = OCR1A * vin;

    sprintf(display1, " %d", OCR1A);
    sprintf(display2, "%d.%02d", vout/100, vout%100);

    lcd_display_double(display1, display2);
}

int main() {
    io_init();
    lcd_init();
    adc_init();
    enable_PWM();
    
    OCR1A = 0;

    while(1) {
        if(OCR1A) OCR1A = 0;

        while(!(PINB & 4)) {
            OCR1A = DC_20;
            convert_and_display();
        }
        while(!(PINB & 8)) {
            OCR1A = DC_20 * 2;
            convert_and_display();
        }
        while(!(PINB & 16)) {
            OCR1A = DC_20 * 3;
            convert_and_display();
        }
        while(!(PINB & 32)) {
            OCR1A = DC_20 * 4;
            convert_and_display();
        }
    }
}