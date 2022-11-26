.include "m328PBdef.inc" ;ATmega328P microcontroller definitions
 
.equ DEL_NU=16*1000	;FOSC_MHZ*DEL_mS
.equ PD3=3   
.equ PD2=2   
.def temp = r16
.def ADC_L = r21
.def ADC_H = r22
.def counter = r17
 
.org 0x00 
 jmp reset
.org 0x2A ;ADC Conversion Complete Interrupt
 jmp ADC_interrupt
 
reset: 
    ldi temp, high(RAMEND)
    out SPH,temp
    ldi temp, low(RAMEND)
    out SPL,temp
 
    ldi temp, 0xFF
    out DDRD, temp ;Set PORTD as output

    ldi temp, 0xFF
    out DDRB, temp ;Set PORTB as output
    
    ldi temp, 0x00
    out DDRC, temp ;Set PORTC as input 
    
    rcall lcd_init 
    ldi r24, low(2)
    ldi r25, high(2)
    rcall wait_msec
    
    
    ldi counter,0x00	;set counter to zero
    
 ; REFSn[1:0]=01 => select Vref=5V, MUXn[4:0]=0000 => select ADC0(pin PC0),
 ; ADLAR=1 => Left adjust the ADC result
    ldi temp, 0b01000000 ; 
    sts ADMUX, temp
    
 ; ADEN=1 => ADC Enable, ADCS=0 => No Conversion,
 ; ADIE=0 => disable adc interrupt, ADPS[2:0]=111 => fADC=16MHz/128=125KHz
    ldi temp, 0b11101111
    sts ADCSRA, temp
    
    ldi temp, 0b00000000
    sts ADCSRB, temp 
    
    
 start:
    sei
    
    lds temp,ADCSRA
    ori temp,(1<<ADSC)
    sts ADCSRA,temp
    
 wait_adc:
    out PORTB, counter
    inc counter
    rjmp wait_adc


ADC_interrupt:
    rcall lcd_init
    
    lds ADC_L,ADCL ; Read ADC result
    lds ADC_H,ADCH ;
    
    mov r19,ADC_L
    mov r20,ADC_H
    lsl r20
    sbrc r19,7
    ori r20,0x01
    lsl r19
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    add r19,ADC_L
    adc r20,ADC_H
    
    mov r18,r20
    andi r18,0x3C   ;integer->r18
    lsr r18
    lsr r18
    ori r18,0x30
    andi r20,0x03
    
    mov r24, r18
    rcall lcd_data
    
    
    
    ldi r24,0x2C
    rcall lcd_data	; ','
    
    mov ADC_L,r19
    mov ADC_H,r20
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    add r19,ADC_L
    adc r20,ADC_H
    
    add r19,ADC_L
    adc r20,ADC_H
    
    mov r18,r20
    andi r18,0x3C   ;first decimal->r18
    lsr r18
    lsr r18
    
    ori r18,0x30
    andi r20,0x03
    
    mov r24, r18
    rcall lcd_data
    
    
    mov ADC_L,r19
    mov ADC_H,r20
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    lsl r20
    sbrc r19,7
    sbr r20,0
    lsl r19
    
    add r19,ADC_L
    adc r20,ADC_H
    
    add r19,ADC_L
    adc r20,ADC_H
    
    mov r18,r20
    andi r18,0x3C   ;second decimal->r18
    lsr r18
    lsr r18	;ready
    ori r18,0x30
    
    mov r24, r18
    rcall lcd_data
    
    ldi r24,low(16*50)
    ldi r25,high(16*50)
    rcall wait_msec
    reti
    
    
write_2_nibbles:
    push r24 ; 
    in r25 ,PIND
    andi r25 ,0x0f
    andi r24 ,0xf0
    add r24 ,r25
    out PORTD ,r24 
    sbi PORTD ,PD3 
    nop
    nop
    cbi PORTD ,PD3
    nop
    nop
    pop r24 
    swap r24 
    andi r24 ,0xf0
    add r24 ,r25
    out PORTD ,r24
    sbi PORTD ,PD3 
    nop
    nop
    cbi PORTD ,PD3
    nop
    nop
    ret
    
lcd_data:
    sbi PORTD ,PD2 
    rcall write_2_nibbles
    ldi r24 ,100 
    ldi r25 ,0
    rcall wait_usec
    ret
    
lcd_command:
    cbi PORTD ,PD2
    rcall write_2_nibbles
    ldi r24 ,100
    ldi r25 ,0 
    rcall wait_usec 
    ret    
    
lcd_init: 
    ldi r24 ,40 
    ldi r25 ,0 
    rcall wait_msec 
    ldi r24 ,0x30 
    out PORTD ,r24 
    sbi PORTD ,PD3 
    cbi PORTD ,PD3 
    ldi r24 ,100
    ldi r25 ,0 
    rcall wait_usec 
    
    ldi r24 ,0x30 
    out PORTD ,r24
    sbi PORTD ,PD3
    cbi PORTD ,PD3
    ldi r24 ,100
    ldi r25 ,0
    rcall wait_usec 
    ldi r24 ,0x20 
    out PORTD ,r24
    sbi PORTD ,PD3
    cbi PORTD ,PD3
    ldi r24 ,100
    ldi r25 ,0
    rcall wait_usec 
    ldi r24 ,0x28 
    rcall lcd_command 
    ldi r24 ,0x0c 
    rcall lcd_command 
    ldi r24 ,0x01 
    rcall lcd_command
    ldi r24 ,low(5000)
    ldi r25 ,high(5000)
    rcall wait_usec 
    ldi r24 ,0x06  
    rcall lcd_command 
    
    ret
    
    ;delay of 1000*DEL_NU+6    
wait_msec:
    ldi r23,249
loop_inn:
    dec r23
    nop
    brne loop_inn
    
    sbiw r24,1
    brne wait_msec
    ret
wait_usec:   
    sbiw r24 ,1      		
    nop           		
    nop          		
    nop           		
    nop           		
    brne wait_usec		
    ret             		


