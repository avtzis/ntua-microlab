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

#define VREF 5

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
    int length; 
    
    lcd_command(DISPLAY_CLEAR);
    
    if((length = strlen(display)) > 16) length = 16;

    for(int i=0; i<length; ++i) {
        lcd_data(display[i]);
    }
}

void lcd_init() {
    _delay_ms(50);

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
    ADMUX = 0b01000000;  //A0
    ADCSRA = 0b10000111;    
}

void io_init() {
    DDRC = INPUT;
    DDRD = OUTPUT;
}

short adc_convert() {
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

int main() {
    unsigned short adc, prev = 0;
    unsigned short vin;
    char display[15];

    io_init();
    lcd_init();
    adc_init();

    while(1) {
        adc = adc_convert();
        if(prev == adc) continue;
        prev = adc;
        vin = ((((adc * VREF * 10) >> 5) *10) >> 5);

        sprintf(display, " %d.%02d", vin/100, vin%100);
        lcd_display(display);
    }
}