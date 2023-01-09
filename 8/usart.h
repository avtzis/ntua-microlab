#ifndef _USART_H
#define _USART_H

#include "avrlib.h"

void usart_init(unsigned int ubrr);
void usart_transmit(reg_t data);
reg_t usart_receive();
void usart_transmit_str(const char* str, int len);
void usart_receive_str(char* str, int len);

#endif