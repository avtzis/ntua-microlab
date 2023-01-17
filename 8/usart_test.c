#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

void usart_init(unsigned int ubrr) {
    UCSR0A = 0;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0C = (3 << UCSZ00);
}

void usart_transmit(uint8_t data) {
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

uint8_t usart_receive() {
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

void usart_command(const char* str) {
    int len = strlen(str);
    char receive[16];

    usart_transmit_str(str, len);
    _delay_ms(1000);
    usart_receive_str(receive, 16);
    _delay_ms(1000);
    usart_transmit_str(receive, strlen(receive)-1);
    _delay_ms(1000);
}

int main() {
    usart_init(103);

    usart_command("ESP:restart");

    while(1) {
    usart_command("ESP:connect");
    usart_command("ESP:url\"http://192.168.1.250:5000/data\"");
    }
}