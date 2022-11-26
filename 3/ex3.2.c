#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main () {
    char table[] = {0x05, 0x1a, 0x2e, 0x43, 0x57, 0x6c, 0x80, 0x94, 0xa9, 0xbd, 0xd2, 0xe6, 0xfb};
    DDRD = 0;
    DDRB = 0xFF;
    TCCR1A = (1 << WGM10) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << CS11);

    int i = 6;
    OCR1AL = table[i];

    while(1) {
        if (!(PIND & 2)) {
            while(!(PIND & 2));
            if(i!=12) OCR1AL = table[++i];
        }
        if (!(PIND & 4)) {
            while(!(PIND & 4));
            if(i!=0) OCR1AL = table[--i];
        }
    }
}