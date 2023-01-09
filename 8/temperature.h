#ifndef _TEMPERATURE_H
#define _TEMPERATURE_H 1

#ifdef DS1820
#include "lookup-temp.h"
#define DECIMAL_PRECISION 1
#else
#include "lookup-temp-b.h"
#define DECIMAL_PRECISION 4
#endif

#include "avrlib.h"
#include <stdio.h>

#define ERR_NODEV 0x8000

reg_t one_wire_reset();
bit_t one_wire_receive_bit();
void one_wire_transmit_bit();
char one_wire_receive_byte();
void one_wire_transmit_byte();
int get_temperature(char* str);

#endif