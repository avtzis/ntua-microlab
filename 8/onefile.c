#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

typedef unsigned char reg_t;
typedef unsigned char bit_t;
typedef volatile unsigned char port_t;

#define INPUT 0x00
#define OUTPUT 0xFF

void clear_bit(port_t* port, int bit) {
    *port &= (0xFE << bit);
}

void set_bit(port_t* port, int bit) {
    *port |= (1 << bit);
}

unsigned powpow(unsigned _base, unsigned _power) {
    int result = _base;
    if(!_power) return 1;
    for(int i=0; i<_power-1; ++i) {
        result *= _base;
    }
    return result;
}

#define ADC_POT0 0x40
#define ADC_SECOND 0x87
#define VREF 5

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
    vin = ((((adc * VREF * 10) >> 5) *1) >> 5);
    return vin;
}

int get_pot_str(char* str) {
    unsigned vin = get_pot_natural();
    vin *= 4; //Normalized for human BP
    sprintf(str, "%d.%01d", vin/10, vin%10);
    return strlen(str);
}

#include <string.h>

#define PORT_LCD_INIT OUTPUT
#define PORT_COMMAND_SET_LCD_8BIT 0x30
#define PORT_COMMAND_SET_LCD_4BIT 0x20

#define LCD_LINE_LENGTH 16
#define LCD_DEGREE_CIRCLE 223

#define C1_CLEAR_DISPLAY 0x01

#define C2_RETURN_HOME 0x02

#define C3_ENTRY_MODE_SHIFT_ENTIRE_DISPLAY 0x05
#define C3_ENTRY_MODE_ADDRESS_AUTO_INCREMENT 0x06

#define C4_DISPLAY_CONTROL_CURSOR_BLINKING 0x09
#define C4_DISPLAY_CONTROL_CURSOR 0x0A
#define C4_DISPLAY_CONTROL_ON 0x0C

#define C6_CURSOR_MOVE_LEFT 0x10
#define C7_CURSOR_MOVE_RIGHT 0x14
#define C8_DISPLAY_SHIFT_LEFT 0x18
#define C9_DISPLAY_SHIFT_RIGHT 0x1C

#define CA_FUNCTION_SET_DISPLAY_FONT_5_11 0x24
#define CA_FUNCTION_SET_DISPLAY_DOUBLE_LINE 0x28
#define CA_FUNCTION_SET_INTERFACE_DATA_LENGTH_8BIT 0x30

#define DISPLAY_SWITCH_TO_LINE_1 0x80
#define DISPLAY_SWITCH_TO_LINE_2 0xC0

#define DISPLAY_LINE_1_START 0x00
#define DISPLAY_LINE_1_END 0x0F
#define DISPLAY_LINE_2_START 0x40
#define DISPLAY_LINE_2_END 0x4F

unsigned __cursor_position__;

void cursor_increment(int steps) {
    __cursor_position__ += steps;
}

int cursor_overflow() {
    return (__cursor_position__ > DISPLAY_LINE_1_END && __cursor_position__ < DISPLAY_LINE_2_START) || __cursor_position__ > DISPLAY_LINE_2_END;
}

void cursor_switch_line() {
    __cursor_position__ = __cursor_position__ < DISPLAY_LINE_2_START ? DISPLAY_LINE_2_START : DISPLAY_LINE_1_START;
}

void send_pulse() {
    set_bit(&PORTD, PD3);
    clear_bit(&PORTD, PD3);
}

void write_2_nibbles(reg_t reg) {
    char temp1 = reg & 0xF0, temp2 = PIND & 0x0F;
    temp1 += temp2;
    PORTD = temp1;
    send_pulse();
    temp1 = (reg & 0x0F) << 4;
    temp1 += temp2;
    PORTD = temp1;
    send_pulse();
}

void lcd_data(reg_t data) {
    set_bit(&PORTD, PD2);
    write_2_nibbles(data);
    _delay_ms(1);
    cursor_increment(1);
}

void lcd_command(reg_t command) {
    clear_bit(&PORTD, PD3);
    write_2_nibbles(command);
    _delay_ms(1);
}

void lcd_init() {
    DDRD = PORT_LCD_INIT;
    _delay_ms(50);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_8BIT;
    send_pulse();
    _delay_ms(1);

    PORTD = PORT_COMMAND_SET_LCD_4BIT;
    send_pulse();
    _delay_ms(1);

    lcd_command(CA_FUNCTION_SET_DISPLAY_DOUBLE_LINE);
    lcd_command(C4_DISPLAY_CONTROL_ON);
    lcd_command(C1_CLEAR_DISPLAY);
    _delay_ms(2);

    lcd_command(C3_ENTRY_MODE_ADDRESS_AUTO_INCREMENT);
    _delay_ms(2);

    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear() {
    lcd_command(C1_CLEAR_DISPLAY);
    _delay_ms(2);
    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear_line_1() {
    lcd_command(DISPLAY_SWITCH_TO_LINE_1);
    for(int i=0; i<LCD_LINE_LENGTH; ++i)
        lcd_data(' ');

    lcd_command(DISPLAY_SWITCH_TO_LINE_1);
    __cursor_position__ = DISPLAY_LINE_1_START;
}

void lcd_clear_line_2() {
    lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    for(int i=0; i<LCD_LINE_LENGTH; ++i)
        lcd_data(' ');

    lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    __cursor_position__ = DISPLAY_LINE_2_START;
}

void lcd_switch_line() {
    if(__cursor_position__ > DISPLAY_LINE_1_END) 
        lcd_command(DISPLAY_SWITCH_TO_LINE_2);
    else 
        lcd_command(DISPLAY_SWITCH_TO_LINE_1);
}

void __lcd_display__(const char* display, int length) {
    int i;
    if(display == NULL) return;
    if(length > LCD_LINE_LENGTH * 2) {
        length = LCD_LINE_LENGTH * 2;
    }
    for(i=0; i<length && !cursor_overflow(); ++i)
        lcd_data(display[i]);

    if(cursor_overflow()) {
        cursor_switch_line();
        lcd_switch_line();
        __lcd_display__(display + i, length - i);
    }
}

void lcd_display(const char* display) {
    __lcd_display__(display, strlen(display));
}

void lcd_clear_and_display(const char* display) {
    lcd_clear();
    lcd_display(display);
}

void lcd_display_line_1(const char* display) {
    int length;
    lcd_clear_line_1();
    if((length = strlen(display) > 16)) length = 16;
    lcd_display(display);
}

void lcd_display_line_1_only(const char* display) {
    int length;
    lcd_clear_line_2();
    lcd_clear_line_1();
    if((length = strlen(display) > 16)) length = 16;
    lcd_display(display);
}

void lcd_display_line_2(const char* display) {
    int length;
    lcd_clear_line_2();
    if((length = strlen(display) > 16)) length = 16;
    lcd_display(display);
}

void lcd_display_line_2_only(const char* display) {
    int length;
    lcd_clear_line_1();
    lcd_clear_line_2();
    if((length = strlen(display) > 16)) length = 16;
    lcd_display(display);
}

void lcd_display_both_lines(const char* display1, const char* display2) {
    lcd_display_line_1(display1);
    lcd_display_line_2(display2);
}


#define PCA9555_0_ADDRESS 0x40 //A0=A1=A2=0 by hardware
#define TWI_READ 1 // reading from twi device
#define TWI_WRITE 0 // writing to twi device
#define SCL_CLOCK 100000L // twi clock in Hz

//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

// PCA9555 REGISTERS
typedef enum {
    REG_INPUT_0 = 0,
    REG_INPUT_1 = 1,
    REG_OUTPUT_0 = 2,
    REG_OUTPUT_1 = 3,
    REG_POLARITY_INV_0 = 4,
    REG_POLARITY_INV_1 = 5,
    REG_CONFIGURATION_0 = 6,
    REG_CONFIGURATION_1 = 7,
} PCA9555_REGISTERS;

//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10

//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28

//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58
#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)

// Global data
char keypad_table[4][4] = {{'*','0','#','D'},{'7','8','9','C'},{'4','5','6','B'},{'1','2','3','A'}};

//initialize TWI clock
void twi_init(void)
{
    TWSR0 = 0; // PRESCALER_VALUE=1
    TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}

// Read one byte from the twi device ( request more data from device)
unsigned char twi_readAck(void)
{
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR0 & (1<<TWINT)));
    return TWDR0;
}

// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address)
{
    uint8_t twi_status;

    // send START condition
    TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));

    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;

    if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) return 1;

    // send device address
    TWDR0 = address;
    TWCR0 = (1<<TWINT) | (1<<TWEN);

    // wail until transmission completed and ACK/NACK has been received
    while(!(TWCR0 & (1<<TWINT)));

    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;

    if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) )
    {
        return 1;
    }

    return 0;
}

// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
    uint8_t twi_status;
    while ( 1 )
    {
        // send START condition
        TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

        // wait until transmission completed
        while(!(TWCR0 & (1<<TWINT)));

        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) continue;

        // send device address
        TWDR0 = address;
        TWCR0 = (1<<TWINT) | (1<<TWEN);

        // wail until transmission completed
        while(!(TWCR0 & (1<<TWINT)));

        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )
        {
            /* device busy, send stop condition to terminate write operation */
            TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

            // wait until stop condition is executed and bus released
            while(TWCR0 & (1<<TWSTO));
            continue;
        }
        break;
    }
}

// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data )
{
    // send data to the previously addressed device
    TWDR0 = data;
    TWCR0 = (1<<TWINT) | (1<<TWEN);

    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));

    if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
    return 0;
}

// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
    return twi_start( address );
}

// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
    // send stop condition
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

    // wait until stop condition is executed and bus released
    while(TWCR0 & (1<<TWSTO));
}

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
    twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
    twi_write(reg);
    twi_write(value);
    twi_stop();
}

unsigned char twi_readNak() {
    TWCR0 = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR0 & (1 << TWINT)));

    return TWDR0;
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
    uint8_t ret_val;
    twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
    twi_write(reg);
    twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
    ret_val = twi_readNak();
    twi_stop();
    return ret_val;
}

void keypad_init() {
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
}

char scan_row(int row) {
    int n = 1;
    for(int i=1; i<row; ++i) (n <<= 1);
    PCA9555_0_write(REG_OUTPUT_1, n ^ 0xFF);

    char input = ((PCA9555_0_read(REG_INPUT_1) ^ 0xFF) & 0xF0);
    return input ? n | input : 0;
}

char scan_keypad() {
    char ret = 0;
    for(int i=1; !ret && i<=4; ++i) ret = scan_row(i);
    return ret;
}

char scan_keypad_rising_edge() {
    char scan1 = scan_keypad();
    _delay_ms(20);
    char scan2 = scan_keypad();

    return scan1 & scan2;
}

char keypad_to_ascii() {
    char pressed_button = scan_keypad_rising_edge();
    char high_byte = ((pressed_button & 0xF0) >> 4);
    char low_byte = pressed_button & 0x0F;
    
    int high, low;

    for(high = 0; high_byte != 1 && high < 4; ++high) high_byte >>= 1;
    for(low = 0; low_byte != 1 && low < 4; ++low) low_byte >>= 1;

    return high>3 || low>3 ? 0 : keypad_table[low][high];
}

char keypad_key_to_lcd() {
    char display_char;
    while(!(display_char = keypad_to_ascii()));
    while(keypad_to_ascii());
    __lcd_display__(&display_char, 1);
    _delay_ms(1);
    
    return display_char;
}

#include <stdio.h>

#define ERR_NODEV 0x8000

reg_t one_wire_reset() {
    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(480);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(100);
    
    port_t temp = PIND;
    _delay_us(380);
    
    return temp & 0x10;
}

bit_t one_wire_receive_bit() {
    bit_t ret;

    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(2);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(10);

    ret = PIND & 0x10;
    _delay_us(49);

    return ret;
}

void one_wire_transmit_bit(char bit) {
    set_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(2);

    (bit & 1) ? set_bit(&PORTD, PD4) : clear_bit(&PORTD, PD4);
    _delay_us(58);

    clear_bit(&DDRD, PD4);
    clear_bit(&PORTD, PD4);
    _delay_us(1);
}

char one_wire_receive_byte() {
    char res = 0;

    for (int i=0; i<8; i++) {
        res >>= 1;
        if(one_wire_receive_bit())
            res |= 0x80;
    }
    
    return res;
}

void one_wire_transmit_byte(bit_t bit) {
    for(int i=0; i<8; i++) {
        one_wire_transmit_bit(bit & 1);
        bit >>= 1;
    }
}


int get_temperature(char* str) {
    int ret = 0;
    reg_t ls_byte, ms_byte;
    unsigned short index;

    if(one_wire_reset()) {
        ret = ERR_NODEV;
        goto out;
    }

    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0x44);


    if(one_wire_reset()) {
        ret = ERR_NODEV;
        goto out;
    }

    one_wire_transmit_byte(0xCC);
    one_wire_transmit_byte(0xBE);

    ls_byte = one_wire_receive_byte();
    ms_byte = one_wire_receive_byte();
    index = ((short)ms_byte << 8) | ls_byte;
    
    int temp;
    unsigned int decimals;
    //int decimal_accuracy = 1;
    //unsigned daf = powpow(10, decimal_accuracy);
    //char sign = '+';
    if(index <= 0x7FF) {
        temp = index / 16;
        decimals = (((unsigned int)index*10) / 16) % 10;
        if(temp > 125) temp = 125, decimals = 0;
    } else if(index > 0x7FF && index < 0xFC90) {
        temp = 0;
        decimals = 0;
    } else {
        index ^= 0xFFFF;
        ++index;
        temp = index / 16;
        decimals = (((unsigned int)index*10) / 16) % 10;
        if(temp > 55) temp = 55, decimals = 0;
        //sign = '-';
    }

    temp += 15; //Normalized for human temperature
    sprintf(str, /* "%c" */"%d.%01d"/* "%cC" */, /* sign, */ temp, decimals/* , (char)LCD_DEGREE_CIRCLE */);
    
out:
    lcd_init();
    return ret;
}

#include <stdio.h>

void usart_init(unsigned int ubrr) {
    UCSR0A = 0;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UBRR0H = (reg_t)(ubrr >> 8);
    UBRR0L = (reg_t)ubrr;
    UCSR0C = (3 << UCSZ00);
}

void usart_transmit(reg_t data) {
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

reg_t usart_receive() {
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void usart_transmit_str(const char* str, int len) {
    for(int i=0; i<len; ++i) 
        usart_transmit(str[i]);
    usart_transmit('\n');
}

void usart_receive_str(char* str, int len) {
    for(int i=0; i<len; ++i) {
        str[i] = usart_receive();
        if(str[i] == '\n') return;
    }
}

int usart_command(const char* str, int n) {
    int len = strlen(str);
    char receive[16];

    usart_transmit_str(str, len);
    usart_receive_str(receive, 16);

    if(n) {
        char turn[2];
        lcd_init();

        sprintf(turn, "%d.", n);
        lcd_clear_and_display(turn);

        if(!strcmp(receive, "\"Success\"")) {
            lcd_display("Success");
            return 0;
        } else if(!strcmp(receive, "\"Fail\"")) {
            lcd_display("Fail");
            return -1;
        } else {
            lcd_display(receive);
            return 1;
        }
    }
    return 0;
}

int main() {
    //usart_init(103);
    //keypad_init();
    adc_init();
    lcd_init();

    /*usart_command("ESP:restart", 0);
    usart_command("ESP:connect", 1); //in loop?
    _delay_ms(1000);
    usart_command("ESP:url:\"http://192.168.1.250:5000/data\"", 2);
    _delay_ms(1000);*/

    int nurse_call = 0;
    for(/*int n = 0; ; ++n*/;;) {
        char temp[16], prss[16], key, status[16];
        //char payload[256];
        char display1[16], display2[16];

        if(get_temperature(temp)) {
            sprintf(temp, "error");
        }

        if(!get_pot_str(prss)) {
            sprintf(prss, "error");
        }
            
            
        key = 0;//keypad_to_ascii();

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
        
        int temp_n;
        sscanf(temp, "%d", &temp_n);
        if(temp_n < 34 || temp_n > 37) sprintf(status, "CHECK TEMP");
        
        lcd_init();
        sprintf(display1, "T:%s ", temp);
        sprintf(display2, "P:%s", prss);
        //lcd_clear_and_display(display2);
        //lcd_display_both_lines(display1, display2);
        char display[16];
        strcat(display, display1);
        strcat(display, display2);
        lcd_clear_and_display(display);
        _delay_ms(1000);
        lcd_clear_and_display("123");
        _delay_ms(1000);

        int prss_n;
        sscanf(prss, "%d", &prss_n);
        lcd_clear_and_display("345");
        _delay_ms(1000);
        if(prss_n < 4 || prss_n > 12) sprintf(status, "CHECK PRESSURE");
        
        

        //sprintf(display1, "T: %s P: %s", temp, prss);
        //lcd_init();
        //lcd_display_both_lines(display1, status);
        //lcd_display_both_lines("1", "2");
        //lcd_clear_and_display("qwerty");

        //_delay_ms(1000);
    }
}