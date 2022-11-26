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
#define V_GAS_0 0.1
#define INVERSE_M 77.5

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
    lcd_command(DISPLAY_CLEAR);
    
    int length; 
    
    if((length = strlen(display)) > 16) length = 16;

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
    ADMUX = 0b01000011; //Α3
    ADCSRA = 0b10000111;
}

void io_init() {
    DDRB = OUTPUT;
    DDRC = INPUT;
    DDRD = OUTPUT;
}

short adc_convert() {
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

int co_emission_level(short adc) {
    int vgas = ((((adc * VREF * 10) >> 5) *10) >> 5);
    return vgas * 77 - 7;
}

char leds(int n) {
    char result = 0;

    for(int i=0; i<n; ++i) {
        result |= (1 << i);
    }

    return result;
}

int main() {
    char msg1[] = " GAS DETECTED";
    char msg2[] = " CLEAR";
    short adc;
    float co_levels;
    int system_in_shock = 0;

    io_init();
    lcd_init();
    adc_init();
    
    while(1) {
        adc = adc_convert();
        co_levels = co_emission_level(adc);
        if(co_levels > 70) {
            if (PORTB) PORTB = 0; else PORTB = 63;
            if(!system_in_shock) {
                lcd_display(msg1);
                system_in_shock = 1;
            }
        } else if(co_levels <= 70 && system_in_shock) {
            PORTB = leds(co_levels / 10);
            lcd_display(msg2);
            system_in_shock = 0;
        } else if(co_levels <= 70 && !system_in_shock) {
            PORTB = leds(co_levels / 10);
        }
        _delay_ms(100);
    }
}