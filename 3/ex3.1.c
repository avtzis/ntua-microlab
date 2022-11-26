#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void enable_PWM() {
    TCCR1A = (1<<WGM11) | (1<<COM1A1);
    TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS11);
}

void disable_PWM() {
    OCR1A = 0;
}

int main() {
    DDRD = 0;
    DDRB = 0xFF;

    while(1) {
        disable_PWM();

        if((PIND & 1) == 0) {
            ICR1 = 15999;
            OCR1A = 7999;
            while((PIND & 1) == 0);
        }
        if((PIND & 2) == 0) {
            ICR1 = 7999;
            OCR1A = 3999;
            while((PIND & 2) == 0);
        }
        if((PIND & 4) == 0) {
            ICR1 = 3999;
            OCR1A = 1999;
            while((PIND & 4) == 0);
        }
        if((PIND & 8) == 0) {
            ICR1 = 1999;
            OCR1A = 999;
            while((PIND & 8) == 0);
        }
    }
}