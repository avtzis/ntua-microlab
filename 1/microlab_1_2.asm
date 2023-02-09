.include "m328PBdef.inc"
.DEF A=r16 
.DEF B=r17 
.DEF C=r18
.DEF D=r19
.DEF temp1=r20
.DEF temp2=r21
.DEF F0=r22
.DEF F1=r23
.DEF counter=r24
 
 ldi counter,0x06 ;counter for 6 loops
 ldi A,0x55
 ldi B, 0x43 ;variables initialisation
 ldi C,0x22
 ldi D,0x02
 
 operations:
   mov temp1,A
   com temp1 ;A'
   or temp1, D  ;A' + D
   com B   ;B'
   and temp1,B ;B'(A'+D)
   com temp1  ;(B'(A'+D))'
   com B ;restoring B 
   mov F0,temp1 ;F0=(B'(A'+D))'=(A'B'+B'D')'
 
   mov temp1,A 
   mov temp2,D
   or temp1,C  ; A + C
   com temp2 ; D'
   or temp2,B ; D'+B
   and temp1 , temp2 ;(A+C)( B+D')
   MOV F1,temp1 ;F1=(A+C)( B+D')
 
 dec counter 
 breq finish  ;checking if loops are finished
 increment:
 SUBI A,-0x02
 SUBI B,-0x03
 SUBI C,-0x04   ;increasing variables respectively
 SUBI D,-0x05
 jmp operations  ;compute F0 and F1 with new values
 finish:
   nop
 
 
 
 
 
 
 