#include "avrlib.h"

void clear_bit(port_t* port, int bit) {
    *port &= (0xFE << bit);
}

void set_bit(port_t* port, int bit) {
    *port |= (1 << bit);
}