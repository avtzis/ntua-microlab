#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define every_1sec ;;_delay_ms(1000);
#define ERR_NODEV 0x8000
#define DISPLAY_CLEAR 0x01

void clear_bit(char port, int bit) {
    port &= (0xFE << bit);
}

void set_bit(char port, int bit) {
    port |= (1 << bit);
}

void send_pulse() {
    set_bit(PORTD, PD3);
    clear_bit(PORTD, PD3);
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
    set_bit(PORTD, PD2);
    write_2_nibbles(data);
    _delay_ms(1);
}

void lcd_command(char command) {
    clear_bit(PORTD, PD3)
    write_2_nibbles(command);
    _delay_ms(1);
}

void lcd_display(const char *display) { 
    int length; 

    lcd_command(DISPLAY_CLEAR);
    lcd_data(' ');

    if((length = strlen(display)) > 16) length = 16;

    for(int i=0; i<length; ++i) {
        lcd_data(display[i]);
    }
}

void lcd_init() {
    DDRD=0xFF;
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

char one_wire_reset() {
    set_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(480);

    clear_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(480);
    
    return (PIND&0x10);
}

char one_wire_receive_bit() {
    char ret;

    set_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(2);

    clear_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(10);

    ret = PIND & 0x10;
    _delay_us(49);

    return ret;
}

void one_wire_transmit_bit(char bit) {
    set_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(2);

    bit ? set_bit(PORTD, PD4) : clear_bit(PORTD, PD4);
    _delay_us(58);

    clear_bit(DDRD, PD4);
    clear_bit(PORTD, PD4);
    _delay_us(1);
}

char one_wire_receive_byte() {
    char res = 0;

    for (int i=0; i<8; i++) {
        if(one_wire_receive_bit())
            res |= 1;
        if(i<7)
            res <<= 1;

    }
    
    return res;
}

void one_wire_transmit_byte(char bit) {
    for(int i=0; i<8; i++)
        one_wire_transmit_bit(bit & (1 << i));
}


int set_temperature(char* prama1){
    char temp, sign;
    if(!one_wire_reset())
        return ERR_NODEV;

    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0x44);


    if(!one_wire_reset())
        return ERR_NODEV;

    one_wire_transmit_byte(0xcc);
    one_wire_transmit_byte(0xBE);


    temp = one_wire_receive_byte();
    sign = one_wire_receive_byte();

    if(sign) {
        temp ^= 0xFF;
        temp++;
        temp *= -1;
    }

    sprintf(prama1, "%+d°C", (temp>-20 && temp<170)? temp/2: temp);

    return 0;
}
 
int main() {
    char temp[16];

    lcd_init();

    for(every_1sec) {    
        if(!(set_temperature(temp))) {
            lcd_display("NO Device");
        } else {
            lcd_display(temp);
        }
    }    
}