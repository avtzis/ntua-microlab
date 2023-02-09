.include "m328PBdef.inc"

.equ FOSC_MHZ=16
.equ DEL_mS=600
.equ DEL_NU=FOSC_MHZ*DEL_mS
.def counter=r21

.org 0x0
rjmp reset
.org 0x2
rjmp ISR0

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    ldi r24, (1 << ISC01) | (1 << ISC00)
    sts EICRA, r24
    ldi r24, (1 << INT0)
    out EIMSK, r24
    sei   ;ορισμος και ενεργοποίηση επιθυμητής διακοπής(int0)
    ser r24
    out DDRC, r24  ;portC=output
    clr r24
    out DDRB, r24  ;portB=input

loop1:
    clr counter
loop2:
    out PORTC, counter ;εμφάνιση μέτρησης στα leds PC0-PC4

    ldi r24, low(DEL_NU)
    ldi r25, high(DEL_NU) ;καθυστερηση περίπου 600mS
    rcall delay_mS

    inc counter ;αύξηση μετρήτη

    cpi counter, 32
    breq loop1 ;αν ο μετρήτης ξεπερασε το 31,μηδενισε τον
                ;και ξεκίνα απο την αρχή
 rjmp loop2 ;αλλιως  σύνέχισε κανονικά την μέτρηση

delay_mS:
    ldi r23, 249

loop_inn:
    dec r23
    nop
    brne loop_inn
    sbiw r24, 1
    brne delay_mS
    ret


ISR0:
    ldi r23, (1 << INTF0)
    out EIFR, r23
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    in r23, EIFR
    andi r23, 1     ;ελεγχος σπινθηρισμου
    cpi r23, 0
    brne ISR0

    rcall display1

    ldi r24, low(16*2000)
    ldi r25, high(16*2000)
    rcall delay_mS

    reti


display1:
    in r23, PINB
    com r23
    ldi r22, 6  ;διαβασμα πατημένων κουμπίων portB
    ldi r24, 0

loop3:
    lsr r23
    brcs incr ;ελεγχος για πατημενο lsb
cont:
    dec r22
    cpi r22, 0
    brne loop3
    out PORTC, r24 ;εξοδος
    ret

incr:
    lsl r24
    ori r24, 1 ;δημιουργια πλήθους led που πρέπει να ανοιξουν
    rjmp cont
