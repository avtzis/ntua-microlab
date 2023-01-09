#include "avrlib.h"
#include "lcd.h"
#include "usart.h"

int main() {
    char transmit[64];

    usart_init(103);
    lcd_init();

    usart_command("ESP:restart\n", 0);
    usart_command("ESP:connect\n", 1);
    _delay_ms(1000);
    usart_command("ESP:url:\"http://192.168.1.250:5000/data\"", 2);
}