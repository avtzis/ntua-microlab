#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char flag;

ISR(TIMER1_OVF_vect) {
    if(PORTB == 63) {
        PORTB = 1;
        TCNT1 = 10848;
        flag = 1;
    } else {
        PORTB = 0;
        flag = 0;
    }
}
ISR(INT1_vect) {
    do {
        EIFR = (1 << INTF1);
        _delay_ms(5);  
    } while((EIFR & 2) != 0);

    sei();

    if(flag) {
        PORTB = 0xFF;
        TCNT1 = 57723;
    } else {
        PORTB = 1;
        TCNT1 = 3036;
        flag = 1;
    }
}

int main() {
    flag = 0;
    TIMSK1 = (1 << TOIE1);
    TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10);
    EICRA = (1 << ISC11) | (1 << ISC10);
    EIMSK = (1 << INT1);
    sei();

    DDRB = 0xFF;
    PORTB = 0;

    while(1) {
        if((PINC & 32) == 0) {
            while((PINC & 32) == 0);
            if(flag) {
                PORTB = 0xff;
                TCNT1 = 57723;
            } else {
                PORTB = 1;
                TCNT1 = 3036;
                flag = 1;
            } 
        }
    }
}




