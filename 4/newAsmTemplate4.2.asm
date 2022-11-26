.include "m328PBdef.inc" ;ATmega328P microcontroller definitions
    
.def temp = r16
.def flag = r17
.def adc_l = r21
.def adc_h = r22
    
.org 0x00
jmp reset
.org 0x02A	    ;ADC Conversion Complete Interrupt
rjmp adc_inte
  
reset:
ldi temp, high(RAMEND)
out SPH,temp
ldi temp, low(RAMEND)
out SPL,temp
ldi temp, 0xff
out DDRD, temp ;Set PORTD as output
ldi temp, 0xFF
out DDRB, temp ;Set PORTB as output
ldi temp,0
out DDRC,temp	; DDRC input
    
; REFSn[1:0]=01 => select Vref=5V, MUXn[4:0]=0011 => select ADC3,
; ADLAR=0 => right adjust the ADC result
ldi temp, 0b0100011 ;
sts ADMUX, temp
    
; ADEN=1 => ADC Enable, ADCS=0 => No Conversion,
; ADIE=1 => enable adc interrupt, ADPS[2:0]=111 => fADC=16MHz/128=125KHz
ldi temp, 0b10001111
sts ADCSRA, temp
 
clt
    
rcall lcd_init
    
    
start:
    sei
    lds temp,ADCSRA
    ori temp,(1<<ADSC)
    sts ADCSRA,temp
    
 wait_adc:
    rjmp wait_adc
    

adc_inte:
    lds adc_l,ADCL ; Read ADC result(right adjusted)
    lds adc_h,ADCH
    
checkadc_h:
    andi adc_h,0xff
    breq checkadc_l
    rjmp danger_alert
 
checkadc_l:
 
checklevel6:
    cpi adc_l,0xcd ;cox>=70?
    brlo nodangerlevels
    rjmp danger_alert
    
  
    
nodangerlevels:
    brtc checklevel5
    call display_clear
    clr flag
    ldi temp,0x00
    out portb,temp
    clt
    
 
checklevel5:
    cpi adc_l,0xCD ;60<=cox<70?
    brlo checklevel4
    ldi temp,0x1f
    out portb,temp
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    call delay_ms
    jmp start
 
checklevel4:
    cpi adc_l,0x8b  ;45<= cox<60
    brlo checklevel3
    ldi temp,0xf ;111000
    out portb,temp
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    call delay_ms
    jmp start
  
checklevel3:
    cpi adc_l,0x63 
    brlo checklevel2
    ldi temp,0x7 ;1110000  ;30<Cox<45
    out portb,temp
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    call delay_ms
    jmp start
  
checklevel2:
    cpi adc_l,0x3C   ; 15<cox<30?
    brlo level1
    ldi temp,0x3 ;110000
    out portb,temp
     ldi r24, low(16*100)
    ldi r25, high(16*100)
    call delay_ms
    jmp start
  
level1:
    ldi  temp,0x1   ;cox<15
    out portb,temp
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    call delay_ms 
    jmp start
    
    
danger_alert:    
    ser temp
    out portb,temp
    brts cont
    call display_gas
cont: 
    set
    ldi r24, low(16*20)
    ldi r25, high(16*20)
    call delay_ms 
    clr temp
    out portb,temp
    ldi r24, low(16*20)
    ldi r25, high(16*20)
    call delay_ms 
    ser temp
    out portb,temp
    ldi r24, low(16*20)
    ldi r25, high(16*20)
    call delay_ms 
    clr temp
    out portb,temp
    ldi r24, low(16*20)
    ldi r25, high(16*20)
    call delay_ms 
    clr temp
    out portb,temp
    jmp start
    
    
    
    
    
    
    
display_clear:
 rcall lcd_init
 clr r24
 ldi r24, 'C' ; clear
 rcall lcd_data
 ldi r24, 'L'
 rcall lcd_data
 ldi r24, 'E'
 rcall lcd_data
 ldi r24, 'A'
 rcall lcd_data
 ldi r24, 'R'
 rcall lcd_data
 ret
 
 
 display_gas:
 rcall lcd_init
 clr r24
 ldi r24, 'G' ; gas message
 rcall lcd_data
 ldi r24, 'A'
 rcall lcd_data
 ldi r24, 'S'
 rcall lcd_data
 ldi r24, ' '
 rcall lcd_data
 ldi r24, 'D'
 rcall lcd_data
 ldi r24, 'E'
 rcall lcd_data
 ldi r24, 'T'
 rcall lcd_data
 ldi r24, 'E'
 rcall lcd_data
 ldi r24, 'C'
 rcall lcd_data
 ldi r24, 'T'
 rcall lcd_data
 ldi r24, 'E'
 rcall lcd_data
 ldi r24, 'D'
 rcall lcd_data
 ret  
    
    
    
write_2_nibbles:
push r24
in r25 ,PIND 
andi r25 ,0x0f 
andi r24 ,0xf0 
add r24 ,r25 
out PORTD ,r24 
sbi PORTD ,3 
cbi PORTD ,3 
nop
nop
pop r24
swap r24 
andi r24 ,0xf0 
add r24 ,r25
out PORTD ,r24
sbi PORTD ,3 
cbi PORTD ,3
nop
nop
ret
    
    
lcd_data:
sbi PORTD ,2 
rcall write_2_nibbles 
ldi r24 ,43 
ldi r25 ,0 
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms  
ret
    
    
lcd_command:
cbi PORTD ,2 
rcall write_2_nibbles 
ldi r24 ,100 
ldi r25 ,0 
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms  
ret
    
    
 
lcd_init:
 ldi r24, low(16*40)
 ldi r25, high(16*40)
 call delay_ms  
ldi r24 ,0x30
out PORTD ,r24 
sbi PORTD ,3 
cbi PORTD ,3 
ldi r24 ,100
ldi r25 ,0 
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms   
ldi r24 ,0x30
out PORTD ,r24
sbi PORTD ,3 
cbi PORTD ,3
ldi r24 ,100
ldi r25 ,0
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms  
ldi r24 ,0x20 
out PORTD ,r24
sbi PORTD ,3
cbi PORTD ,3
ldi r24 ,100
ldi r25 ,0
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms  
ldi r24 ,0x28 
rcall lcd_command 
ldi r24 ,0x0c
rcall lcd_command
ldi r24 ,0x01 
rcall lcd_command
ldi r24 ,low(5000)
ldi r25 ,high(5000)
 ldi r24, low(16*1)
 ldi r25, high(16*1)
 call delay_ms  
ldi r24 ,0x06 
rcall lcd_command 
ret
    
    
 delay_mS:
    ldi r23, 249
    
loop_inn:
    dec r23
    nop
    brne loop_inn
    sbiw r24, 1
    brne delay_mS
    ret        