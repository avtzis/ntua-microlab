#ifndef _ADC_H
#define _ADC_H 1

#include "avrlib.h"
#include <string.h>
#include <stdio.h>

#define ADC_POT0 0x40
#define ADC_SECOND 0x87
#define VREF 5

void adc_init();
unsigned short adc_convert();
unsigned get_pot_natural();
int get_pot_str(char* str);

#endif