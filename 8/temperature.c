#include "temperature.h"
#include "lcd.h"

reg_t one_wire_reset() {
    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(480);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(100);
    
    port_t temp = PIND;
    _delay_us(380);
    
    return temp & 0x10;
}

bit_t one_wire_receive_bit() {
    bit_t ret;

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

void one_wire_transmit_byte(bit_t bit) {
    for(int i=0; i<8; i++) {
        one_wire_transmit_bit(bit & 1);
        bit >>= 1;
    }
}


int get_temperature(char* str){
    int ret = 0;
    reg_t ls_byte, ms_byte;
    unsigned short index;

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

    ls_byte = one_wire_receive_byte();
    ms_byte = one_wire_receive_byte();
    index = ((short)ms_byte << 8) | ls_byte;
    
    int temp;
    unsigned int decimals;
    int decimal_accuracy = 1;
    unsigned daf = pow(10, decimal_accuracy);
    char sign = '+';
    if(index <= 0x7FF) {
        temp = index / 16;
        decimals = (((unsigned int)index*daf) / 16) % daf;
        if(temp > 125) temp = 125, decimals = 0;
    } else if(index > 0x7FF && index < 0xFC90) {
        temp = 0;
        decimals = 0;
    } else {
        index ^= 0xFFFF;
        ++index;
        temp = index / 16;
        decimals = (((unsigned int)index*daf) / 16) % daf;
        if(temp > 55) temp = 55, decimals = 0;
        sign = '-';
    }

    temp += 15; //Normalized for human temperature
    sprintf(str, /* "%c" */"%d.%0*d"/* "%cC" */, /* sign, */ temp, decimal_accuracy, decimals/* , (char)LCD_DEGREE_CIRCLE */);
    
out:
    return ret;
}