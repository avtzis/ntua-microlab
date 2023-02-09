.include "m328PBdef.inc"

.org 0x0
rjmp reset
.org 0x4
rjmp ISR1

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    ldi r24, (1 << ISC11) | (1 << ISC10)
    sts EICRA, r24
    ldi r24, (1 << INT1)
    out EIMSK, r24
    sei ;ορισμος και ενεργοπoιήση επιθυμητής διακοπής(int2)
    ser r24
    out DDRB, r24  ;portb=output
    clr r24
    out PORTB, r24
    clt

start:
    rjmp start  ;αναμονη για διακοπή


ISR1:
    ldi r23, (1 << INTF1)
    out EIFR, r23
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    in r23, EIFR     ;έλεγχος σπινθηρισμου
    andi r23, 2
    cpi r23, 0
    brne ISR1

    sei

    brtc case2

   case1:  ;περίπτωση ανανεωσης

    ldi r21, 255
    out PORTB, r21 ;αναμμα ολων των led για 0.5 secs

    ldi r24, low(16*500)
    ldi r25, high(16*500)
    rcall delay_mS

    ldi r21, 1
    out PORTB, r21 ;PB0 παραμένει ανοικτό για 4 secs συνολικα

    ldi r24, low(16*3500)
    ldi r25, high(16*3500)
    rcall delay_mS

    ldi r21, 0
    out PORTB, r21 ;σβησιμο leds
    clt
    reti

case2:
    set
    ldi r21, 1
    out PORTB, r21

    ldi r24, low(16*4000)
    ldi r25, high(16*4000)
    rcall delay_mS

    ldi r21, 0
    out PORTB, r21  ;σβησιμο leds
    clt
    reti



delay_mS:
    ldi r23, 249

loop_inn:
    dec r23
    nop
    brne loop_inn
    sbiw r24, 1
    brne delay_mS
    ret
