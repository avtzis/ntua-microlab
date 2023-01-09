#ifndef _AVRLIB_H
#define _AVRLIB_H 1

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

typedef unsigned char reg_t;
typedef unsigned char bit_t;
typedef volatile unsigned char port_t;

#define INPUT 0x00
#define OUTPUT 0xFF

void clear_bit(port_t* port, int bit);
void set_bit(port_t* port, int bit);

#endif