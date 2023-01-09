#include "adc.h"
#include "lcd.h"
#include "usart.h"
#include "keypad.h"
#include "temperature.h"

int main() {
    usart_init(103);
    keypad_init();
    adc_init();

    usart_command("ESP:restart\n", 0);
    usart_command("ESP:connect\n", 1);
    _delay_ms(1000);
    usart_command("ESP:url:\"http://192.168.1.250:5000/data\"", 2);

    int nurse_call = 0;
    while(1) {
        char temp[16], prss[16], key, status[16];

        if(get_temperature(temp)) {
            temp = "error";
        }

        if(!get_pot_str(prss)) {
            prss = "error";
        }
        key = keypad_to_ascii();

        if(key == '9') {
            nurse_call = 1;
        } else if(key == '#') {
            nurse_call = 0;
        }
    }
}