#include "usart.h"

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
}

void usart_receive_str(char* str, int len) {
    for(int i=0; i<len; ++i)
        str[i] = usart_receive();
}