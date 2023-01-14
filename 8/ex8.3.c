#include "adc.h"
#include "lcd.h"
#include "usart.h"
#include "keypad.h"
#include "temperature.h"

int main() {
    usart_init(103);
    keypad_init();
    adc_init();

    usart_command("ESP:restart", 0);
    usart_command("ESP:connect", 1); //in loop?
    _delay_ms(1000);
    usart_command("ESP:url:\"http://192.168.1.250:5000/data\"", 2);
    _delay_ms(1000);

    int nurse_call = 0;
    for(int n = 0; ; ++n) {
        char temp[16], prss[16], key, status[16];
        char display1[16], payload[128];

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

        if(nurse_call) {
            sprintf(status, "NURSE CALL");
        } else {
            sprintf(status, "OK");
        }

        if(n > 100) {
            n = 0;

            int temp_n;
            sscanf(temp, "%d", &temp_n);
            if(temp_n <= 34 || temp_n >= 37) sprintf(status, "CHECK TEMP");

            int prss_n;
            sscanf(prss, "%d", &prss_n);
            if(prss_n <= 4 || prss_n >= 12) sprintf(status, "CHECK PRESSURE");

            //Send Payload
            sprintf(payload, "ESP:payload:"
                             "["
                             "{\"name\": \"temperature\",\"value\": \"%s\"},"
                             "{\"name\": \"pressure\",\"value\": \"%s\"},"
                             "{\"name\": \"team\",\"value\": \"%d\"},"
                             "{\"name\": \"status\",\"value\": \"%s\"}"
                             "]",
                    temp, prss, 19, status
                   );
            usart_command(payload, 3);
            usart_command("ESP:transmit", 4);
        }
    }
}