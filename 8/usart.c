#include "usart.h"
#include <stdio.h>
#include "lcd.h"

void usart_init(unsigned int ubrr) {
    UCSR0A = 0;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UBRR0H = (reg_t)(ubrr >> 8);
    UBRR0L = (reg_t)ubrr;
    UCSR0C = (3 << UCSZ00);
}

void usart_transmit(reg_t data) {
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

reg_t usart_receive() {
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void usart_transmit_str(const char* str, int len) {
    for(int i=0; i<len; ++i) 
        usart_transmit(str[i]);
    usart_transmit('\n');
}

void usart_receive_str(char* str, int len) {
    for(int i=0; i<len; ++i) {
        str[i] = usart_receive();
        if(str[i] == '\n') return;
    }
}

int usart_command(const char* str, int n) {
    int len = strlen(str);
    char receive[16];

    usart_transmit_str(str, len);
    usart_receive_str(receive, 16);

    if(n) {
        char turn[2];
        lcd_init();

        sprintf(turn, "%d.", n);
        lcd_clear_and_display(turn);

        if(!strcmp(receive, "\"Success\"")) {
            lcd_display("Success");
            return 0;
        } else if(!strcmp(receive, "\"Fail\"")) {
            lcd_display("Fail");
            return -1;
        } else {
            lcd_display(receive);
            return 1;
        }
    }
    return 0;
}