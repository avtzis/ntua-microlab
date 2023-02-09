.include "m328PBdef.inc"
.def counter=r21
.def PD7=r22    
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
    sts EICRA, r24  ;ορισμος επιπέδου ενεργοποιήσης διακοπών
    ldi r24, (1 << INT1)
    out EIMSK, r24 ;επίτρεψη διακοπης int1
    sei ;γενικη ενεργοποιηση διακοπων
    clr counter
    ser r24
    out DDRC, r24 ;portC=output
    clr r24
      out DDRD,R24 ;PORTD=INPUT
    out PORTC, r24 ;C=0x00
    
start:
    rjmp start   ;αναμονη διακοπης
    

ISR1:
    in PD7, PIND
    andi PD7, 0x80
    cpi PD7, 0        
    brne rout1       ;Πάγωμα μέτρησης όταν είναι πατημένο το PD7
    reti
    
rout1:
    ldi r23, (1 << INTF1)
    out EIFR, r23
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    in r23, EIFR     ; Έλεγχος σπινθηρισμού
    andi r23, 2
    cpi r23, 0
    brne rout1        
    
    cpi counter, 31 ;counter=31??
    breq rout3
    inc counter  ;αν οχι,τοτε counter+=1
    out PORTC, counter
    reti
    
rout3:
    clr counter        
    out PORTC, counter  ;μηδενισμος counter
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
