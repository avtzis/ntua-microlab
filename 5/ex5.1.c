#define EXAMPLE

#include "example1.c"

int main() {
    twi_init();
    DDRB = 0;
    DDRD = 0xFF; //?

    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);

    char A, B, C, D, F0, F1;

    while(1) {
        char input = PINB;

        A = input & 1;
        B = (input & 2 >> 1);
        C = (input & 4 >> 2);
        D = (input & 8 >> 3);

        F0 = !(!A & B | !B & C & D);
        F1 = A & C & (B | D);

        PCA9555_0_write(REG_OUTPUT_0, F0 | (F1 << 1));
        //_delay_ms(1000);
    }
}