#ifndef _TEMPERATURE_H
#define _TEMPERATURE_H 1

#include "avrlib.h"
#include <stdio.h>

#define ERR_NODEV 0x8000

reg_t one_wire_reset();
bit_t one_wire_receive_bit();
void one_wire_transmit_bit(char bit);
char one_wire_receive_byte();
void one_wire_transmit_byte(bit_t bit);
int get_temperature(char* str);

#endif