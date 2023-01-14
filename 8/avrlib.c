#include "avrlib.h"

void clear_bit(port_t* port, int bit) {
    *port &= (0xFE << bit);
}

void set_bit(port_t* port, int bit) {
    *port |= (1 << bit);
}

unsigned pow(unsigned base, unsigned power) {
    int result = base;
    if(!power) return 1;
    for(int i=0; i<power-1; ++i) {
        result *= base;
    }
    return result;
}