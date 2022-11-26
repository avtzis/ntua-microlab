.include "m328PBdef.inc"
.def level=r22
.org 0x0
rjmp reset
    
table:
.dw 0x1A05, 0x432E, 0x6C57, 0x9480, 0xBDA9, 0xE6D2, 0x00FB

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    clr r24
    out DDRD, r24
    ser r24
    out DDRB, r24
    ldi r24, (1<<WGM10) | (1<<COM1A1)
    sts TCCR1A, r24
    ldi r24, (1<<WGM12) | (1<<CS11)
    sts TCCR1B, r24
    ldi zh, high(table*2)
    ldi zl, low(table*2)
    adiw zl, 6
    lpm
    mov level, r0
    sts OCR1AL, level
    
    
push_button1:
    in r21, PIND
    andi r21, 2
    cpi r21, 0
    brne push_button2
    
let_button1:    
    in r21, PIND
    andi r21, 2
    cpi r21, 0
    breq let_button1
    
    cpi level, 0xFB
    breq push_button2
    
    adiw zl, 1
    lpm
    mov level, r0
    sts OCR1AL, level
    
    
push_button2:
    in r21, PIND
    andi r21, 4
    cpi r21, 0
    brne push_button1
    
let_button2:    
    in r21, PIND
    andi r21, 4
    cpi r21, 0
    breq let_button2  
    
    cpi level, 0x05
    breq push_button1
    
    sbiw zl, 1
    lpm
    mov level, r0
    sts OCR1AL, level
    
    rjmp push_button1


