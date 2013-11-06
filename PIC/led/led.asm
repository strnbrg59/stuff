#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


        cblock 0x20
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
        endc
        
        org 0
        goto MAIN

;----------------------------------------------------------------------
; Put I/O ports in all-digital mode, all output.
;----------------------------------------------------------------------
InitPorts
        bsf     STATUS,RP1
        clrf    ANSEL^h'100'
        clrf    ANSELH^h'100'
        bcf     STATUS,RP1
        bsf     STATUS,RP0      
        clrf    TRISA^h'80'
        clrf    TRISB^h'80'
        clrf    TRISC^h'80'
        bcf     STATUS,RP0

        clrf    PORTA
        bsf     PORTA,1
        clrf    PORTB
        clrf    PORTC


;---------------------------------- spin delays --------------------
;
;; Inner delay loop called by DELAY
INRDLY  movlw   h'FF'
        movwf   DCNT2
dloop2  decfsz  DCNT2,f
        goto    dloop2
        return

;; Delay -- about 1/4 second.
DELAY   movlw   h'FF'
        movwf   DCNT1
dloop   call    INRDLY
        decfsz  DCNT1,f
        goto    dloop
        return        
;------------------------------------------------------------------

MAIN    call    InitPorts

        bsf     STATUS,RP1
        clrf    EEADR^h'100'        ; Position to beginning of EEPROM space.
        bcf     STATUS,RP1

loop	bsf		PORTC,0
		call	DELAY
		bcf		PORTC,0
		call	DELAY	

		bsf		PORTC,1
		call    DELAY
		bcf     PORTC,1
		call    DELAY

		bsf     PORTC,2
		call    DELAY
		bcf     PORTC,2
                call 	DELAY

		bsf     PORTC,3
		call    DELAY
		bcf     PORTC,3

sndlp           bsf     PORTC,2
                call    INRDLY
				call	INRDLY
                bcf     PORTC,2
                call    INRDLY
				call	INRDLY
                goto    sndlp                

		goto	loop
lp2     goto    lp2
        end
