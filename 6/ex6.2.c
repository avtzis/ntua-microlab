#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#define PCA9555_0_ADDRESS 0x40 //A0=A1=A2=0 by hardware
#define TWI_READ 1 // reading from twi device
#define TWI_WRITE 0 // writing to twi device
#define SCL_CLOCK 100000L // twi clock in Hz
#define DISPLAY_CLEAR 0x01 // clear lcd

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

void send_pulse() {
    PORTD |= (1 << PD3);
    PORTD &= (254 << PD3);
}

void write_2_nibbles(char reg0) {
    char reg1 = reg0 & 0xf0, reg2 = PIND & 0x0f;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
    reg1 = (reg0 & 0x0f) << 4;
    reg1 += reg2;
    PORTD = reg1;
    send_pulse();
}

void lcd_data(char data) {
    PORTD |= (1 << PD2);
    write_2_nibbles(data);
    _delay_ms(1);
}

void lcd_command(char command) {
    PORTD &= (254 << PD2);
    write_2_nibbles(command);
    _delay_ms(1);
}

void lcd_display(const char* display, int length) { 
    for(int i=0; i<length; ++i) {
        lcd_data(display[i]);
    }
}

void lcd_init() {
    DDRD = 0xFF;
            
    _delay_ms(50);

    PORTD = 0x30;
    send_pulse();
    _delay_ms(1);

    PORTD = 0x30;
    send_pulse();
    _delay_ms(1);

    PORTD = 0x30;
    send_pulse();
    _delay_ms(1);

    PORTD = 0x20;
    send_pulse();
    _delay_ms(1);

    lcd_command(0x28);
    lcd_command(0x0c);
    lcd_command(0x01);
    _delay_ms(2);

    lcd_command(0x06);
    _delay_ms(2);
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

void DC_init() {
    TCCR1A = (1 << WGM10) | (1 << COM1A1); //?
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1AL = 255;
}

void clear_display() {
    lcd_command(DISPLAY_CLEAR);
    lcd_data(' ');
}

int main() {
    twi_init();
    lcd_init();
    DC_init();
    DDRB = 0xFF;
    PORTB = 0;
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    clear_display();

    while(1) {
        char display_char1;
        display_char1 = keypad_to_ascii();
        if(display_char1);// lcd_display(&display_char1, 1);
        else continue;
        
        char display_char2;
        while(display_char1 == (display_char2 = keypad_to_ascii()));
        while(!display_char2) display_char2 = keypad_to_ascii();
        //lcd_display(&display_char2, 1);

        _delay_ms(100);
        //clear_display();
        
        char password[2] = {display_char1, display_char2};
        lcd_display(password, 2);
        if(!strcmp(password, "19")) {
            PORTB = 0xFF;
            lcd_display(" access granted", 16);
            _delay_ms(4000);
            PORTB = 0;
            _delay_ms(1000);
            clear_display();
        } else {
            lcd_display(" incorrect", 10);
            OCR1AL = 12;
            for(int i=0; i<5; ++i) {
                PORTB = 0xFF;
                _delay_ms(500);
                PORTB = 0;
                _delay_ms(500);
            }
            OCR1AL = 255;
            clear_display();
        }
    }
}
