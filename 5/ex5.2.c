#define EXAMPLE

#include "example1.c"

int main() {
    twi_init();
    PCA9555_0_write(REG_OUTPUT_0, 0x0F);
    PCA9555_0_write(REG_OUTPUT_1, 0x01);
    PCA9555_0_write(REG_INPUT_1, 0xF0);

    DDRD = 0xFF; //?

    while(1) {
        char input = PCA9555_0_read(REG_INPUT_1);

        if(input & 0x10) {
            PCA9555_0_write(REG_OUTPUT_0, 1);
            while(input & 0x10);
        }
        if(input & 0x20) {
            PCA9555_0_write(REG_OUTPUT_0, 2);
            while(input & 0x20);
        }
        if(input & 0x30) {
            PCA9555_0_write(REG_OUTPUT_0, 4);
            while(input & 0x30);
        }
        if(input & 0x40) {
            PCA9555_0_write(REG_OUTPUT_0, 8);
            while(input & 0x40);
        }
        else {
            PCA9555_0_write(REG_OUTPUT_0, 0);
            while(!(input & 0xF0));
        }
    }
}