#include "lcd.h"

int main() {
    lcd_init();

    lcd_display("test 1");
    _delay_ms(3000);

    lcd_clear_and_display("test 2");
    _delay_ms(3000);

    lcd_display_line_2("test  3");
    _delay_ms(3000);

    lcd_display_line_1("test 4");
    _delay_ms(3000);

    lcd_display_line_1_only("test 5");
    _delay_ms(3000);

    lcd_display_line_2_only("test 6");
    _delay_ms(3000);

    lcd_display_both_lines("test 7", "test 8");
}