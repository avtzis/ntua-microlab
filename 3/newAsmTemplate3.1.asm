.include "m328PBdef.inc"
    
.org 0x0
rjmp reset
.org 0x4
rjmp ISR1
.org 0x1A
rjmp ISR_TIMER1_OVF    
    
reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    clr r24
    out DDRC, r24
    ser r24
    out DDRB, r24
    clr r24
    out PORTB,r24
    ldi r24, (1<<TOIE1)
    sts TIMSK1, r24
    ldi r24, (1<<CS12) | (1<<CS10)
    sts TCCR1B, r24
    ldi r24, (1 << ISC11) | (1 << ISC10)
    sts EICRA, r24
    ldi r24, (1 << INT1)
    out EIMSK, r24
    sei
    clt
    
start:
    in r21, PINC
    andi r21, 32
    cpi r21, 0
    brne start
    
addr1:    
    in r21, PINC
    andi r21, 32
    cpi r21, 0
    breq addr1
    
    brts renew
        
    ldi r24, high(3036)	    ;4sec
    sts TCNT1H, r24
    ldi r24, low(3036)
    sts TCNT1L, r24
    
cont:    
    set
    ldi r22, 1
    out PORTB, r22
    
    rjmp start
    
    
renew:
    ldi r22, 63
    out PORTB, r22
    ldi r24, high(57723)    ;0,5sec
    sts TCNT1H, r24
    ldi r24, low(57723)
    sts TCNT1L, r24
    rjmp start
    
    
ISR_TIMER1_OVF:
    in r22, PORTB
    cpi r22, 63
    brne turnoff
    
    sei
    ldi r24, high(10848)	    ;3,5sec
    sts TCNT1H, r24
    ldi r24, low(10848)
    sts TCNT1L, r24
    rjmp cont
    
turnoff:    
    ldi r24, 0
    out PORTB, r24
    clt
    reti
    
    
ISR1:
    ldi r23, (1 << INTF1)
    out EIFR, r23
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    in r23, EIFR
    andi r23, 2
    cpi r23, 0
    brne ISR1
    
    sei
    
    brts renew
        
    ldi r24, high(3036)	    ;4sec
    sts TCNT1H, r24
    ldi r24, low(3036)
    sts TCNT1L, r24
    
    rjmp cont
   
    
delay_mS:
    ldi r23, 249
    
loop_inn:
    dec r23
    nop
    brne loop_inn
    sbiw r24, 1
    brne delay_mS
    ret        
    


