#include "adc.h"

void adc_init() {
    DDRC = INPUT;
    ADMUX = ADC_POT0;
    ADCSRA = ADC_SECOND;
}

unsigned short adc_convert() {
    set_bit(&ADCSRA, ADSC);
    while(ADCSRA & (1 << ADSC));
    return ADC;
}

unsigned get_pot_natural() {
    unsigned short adc;
    unsigned vin;
    adc = adc_convert();
    vin = ((((adc * VREF * 10) >> 5) *10) >> 5);
    return vin;
}

int get_pot_str(char* str) {
    unsigned vin = get_pot_natural();
    sprintf(str, "%d.%02d", vin/100, vin%100);
    return strlen(str);
}