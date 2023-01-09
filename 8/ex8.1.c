#include "avrlib.h"
#include "lcd.h"
#include "usart.h"

int main() {
    char transmit[64], receive[16];

    usart_init(103);
    lcd_init();

    usart_transmit_str("ESP:restart\n", 13);


    usart_transmit_str("ESP:connect\n", 13)
    usart_receive_str(receive, 16);
    lcd_clear_and_display("1.");
    lcd_display(receive);

    _delay_ms(1000);

    usart_transmit_str("ESP:url:\"http://192.168.1.250:5000/data\"", 41);
    usart_receive_str(receive, 16);
    lcd_clear_and_display("2.");
    lcd_display(receive);

}