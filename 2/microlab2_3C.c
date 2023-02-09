#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

char flag;

ISR (INT1_vect) {
    do {
        EIFR = (1<<INTF1);
        _delay_ms(5);  //ελέγχος σπινθηρισμού
    } while((EIFR & 2)!=0);

    sei();

    if(flag) {     //σε περίπτωση ανανέωσης
        flag=1;
        PORTB = 0xFF;   //αναμμα ολων των led για 0.5 secs
        _delay_ms(500);
        PORTB = 1;  //PB0 παραμένει ανοικτό για 4 secs συνολικα
        _delay_ms(3500);
        PORTB = 0;
        flag=0;
    } else {
        flag=1;
        PORTB = 1;
        _delay_ms(4000); //αναμμα PB0 για 4secs
        PORTB = 0;
        flag=0;
    }
}

int main() {
    flag = 0;
    EICRA = (1 << ISC11) | (1 << ISC10);
    EIMSK = (1 << INT1);  //αρχικοποίηση και επίτρεψη διακοπης int1
    sei();

    DDRB = 0xFF; //portD=οutput
    PORTB = 0;

    while(1); //αναμονη για διακοπή
}
