;;;
;;; Try to use PORTC[6,7], which somehow don't respond.
;;; 
        
#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


        cblock 0x20
DCNT1
DCNT2
        endc
        
        org 0
        goto MAIN

;---------------------------------- spin delay --------------------
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

MAIN
        ;; Init PORTC: all-digital, all-output, cleared
        bsf     STATUS,RP1      ; All-digital
        clrf    ANSEL ^ h'100'
        clrf    ANSELH ^ h'100'
        bcf     STATUS,RP1
        clrf    PORTC           ; All-output
        bsf     STATUS,RP0
        clrf    TRISC^0x80
        bsf     TRISC^0x80,6
        bcf     STATUS,RP0
        movlw   b'11111111'
loop    movwf   PORTC
        call    DELAY
        clrf    PORTC
        call    DELAY
        goto    loop

        end
