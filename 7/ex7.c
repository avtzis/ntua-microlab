#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define every_1sec ;;_delay_ms(1000)
#define ERR_NODEV 0x8000
#define DISPLAY_CLEAR 0x01
#define DEGREE_CIRCLE 0b10110010
//#define DS18B20 1

void clear_bit(volatile unsigned char* port, int bit) {
    *port &= (0xFE << bit);
}

void set_bit(volatile unsigned char* port, int bit) {
    *port |= (1 << bit);
}

void send_pulse() {
    set_bit(&PORTD, PD3);
    clear_bit(&PORTD, PD3);
}

void write_2_nibbles(char reg0) {
    char reg1 = reg0 & 0xF0, reg2 = PIND & 0x0F;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
    reg1 = (reg0 & 0x0F) << 4;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
}

void lcd_data(char data) {
    set_bit(&PORTD, PD2);
    write_2_nibbles(data);
    _delay_ms(1);
}

void lcd_command(char command) {
    clear_bit(&PORTD, PD3);
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
    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(480);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(100);
    
    volatile unsigned char temp = PIND;
    _delay_us(380);
    
    return temp & 0x10;
}

char one_wire_receive_bit() {
    char ret;

    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(2);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(10);

    ret = PIND & 0x10;
    _delay_us(49);

    return ret;
}

void one_wire_transmit_bit(char bit) {
    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(2);

    (bit & 1) ? set_bit(&PORTD, PD4) : clear_bit(&PORTD, PD4);
    _delay_us(58);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(1);
}

char one_wire_receive_byte() {
    char res = 0;

    for (int i=0; i<8; i++) {
        res >>= 1;
        if(one_wire_receive_bit())
            res |= 0x80;
    }
    
    return res;
}

void one_wire_transmit_byte(char bit) {
    for(int i=0; i<8; i++) {
        one_wire_transmit_bit(bit & 1);
        bit >>= 1;
    }
}


int get_temperature(char* prama1){
    int ret = 0;
    char temp, sign;

    if(one_wire_reset()) {
        ret = ERR_NODEV;
        goto out;
    }

    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0x44);


    if(one_wire_reset()) {
        ret = ERR_NODEV;
        goto out;
    }

    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0xBE);

#ifndef DS18B20
    temp = one_wire_receive_byte();
    sign = one_wire_receive_byte();

    if(sign) {
        temp ^= 0xFF;
        temp++;
        //temp *= -1;
        char real_temp;
        int idle = temp < 20;
        int limit = temp < 65;
        int point = idle && temp%2;
        if(idle) real_temp = temp/2;
        else if (limit) real_temp = temp - 10;
        else real_temp = 55;
        sprintf(prama1, "-%d.%01d%cC", real_temp, point? 5: 0, (char)223);
    } else {
        char real_temp;
        int idle = temp < 170;
        int limit = temp < 295;
        int point = idle && temp%2;
        if(idle) real_temp = temp/2;
        else if (limit) real_temp = temp - 85;
        else real_temp = 125;
        sprintf(prama1, "%+d.%d%cC", real_temp, point? 5: 0, (char)223);
    }
    
#else
    int temp1 = one_wire_receive_byte();
    int temp2 = one_wire_receive_byte() << 8;
    (int)temp = ((temp1 | temp2) & 0x7FF) * 1000;
    (int)sign = temp2 & 0xF800;
    
    if(sign) {
        temp ^= 0x7FF;
        temp++;
        //temp *= -1;
        sprintf(prama1, "-%d.%03d%cC", temp/16000, temp%16000, (char)223);
    }
    else sprintf(prama1, "%+d.%03d%cC", temp/16000, temp%16000, (char)223);
#endif


    
    
out:
    lcd_init();
    return ret;
}
 
int main() {
    for(every_1sec) {
        char temp[16];
        if(get_temperature(temp)) {
            lcd_display("NO Device");
        } else {
            lcd_display(temp);
        }
    }    
}