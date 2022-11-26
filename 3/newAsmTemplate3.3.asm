.include "m328PBdef.inc"

.org 0x0
rjmp reset

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    clr r24
    out DDRD, r24
    ser r24
    out DDRB, r24 
    ldi r24, (1<<WGM11) | (1<<COM1A1)
    sts TCCR1A, r24
    ldi r24, (1<<WGM13) | (1<<WGM12) | (1<<CS11)
    sts TCCR1B, r24   
    
    
no_button:
    clr r24
    sts OCR1AH, r24
    sts OCR1AL, r24 
    
push_button0:
    in r21, PIND
    andi r21, 1
    cpi r21, 0
    brne push_button1
    
    ldi r24, high(15999)
    sts ICR1H, r24
    ldi r24, low(15999)
    sts ICR1L, r24
    ldi r24, high(7999)
    sts OCR1AH, r24
    ldi r24, low(7999)
    sts OCR1AL, r24	;TOP=15999 DC=7999 fpwm=125
    
let_button0:
    in r21, PIND
    andi r21, 1
    cpi r21, 0
    breq let_button0
    rjmp no_button
    
    
push_button1:
    in r21, PIND
    andi r21, 2
    cpi r21, 0
    brne push_button2
    
    ldi r24, high(7999)
    sts ICR1H, r24
    ldi r24, low(7999)
    sts ICR1L, r24
    ldi r24, high(3999)
    sts OCR1AH, r24
    ldi r24, low(3999)
    sts OCR1AL, r24	;TOP=999 DC=499 fpwm=250
    
let_button1:
    in r21, PIND
    andi r21, 2
    cpi r21, 0
    breq let_button1
    rjmp no_button    
    
    
push_button2:    
    in r21, PIND
    andi r21, 4
    cpi r21, 0
    brne push_button3
    
    ldi r24, high(3999)
    sts ICR1H, r24
    ldi r24, low(3999)
    sts ICR1L, r24
    ldi r24, high(1999)
    sts OCR1AH, r24
    ldi r24, low(1999)
    sts OCR1AL, r24	;TOP=499 DC=249 fpwm=500
    
let_button2:
    in r21, PIND
    andi r21, 4
    cpi r21, 0
    breq let_button2
    rjmp no_button
    
    
push_button3:    
    in r21, PIND
    andi r21, 8
    cpi r21, 0
    brne finito
    
    ldi r24, high(1999)
    sts ICR1H, r24
    ldi r24, low(1999)
    sts ICR1L, r24
    ldi r24, high(999)
    sts OCR1AH, r24
    ldi r24, low(999)
    sts OCR1AL, r24	;TOP=249 DC=124 fpwm=1000
    
let_button3:
    in r21, PIND
    andi r21, 8
    cpi r21, 0
    breq let_button3
    rjmp no_button    
    
finito:
    rjmp no_button


