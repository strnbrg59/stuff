#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


        cblock 0x20
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
        endc

                
        org 0x00
        goto Main


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

InitPorts 
        bsf     STATUS,RP1      ; Put I/O ports in all-digital mode.
        clrf    ANSEL^h'100'
        clrf    ANSELH^h'100'
        bcf     STATUS,RP1

        clrf    PORTC
        
        bsf     STATUS,RP0      
        movlw   b'00100000'     ; Set TRISC all output except for LCD_BF,
        movwf   TRISC^h'80'     ; which we just want to test.
        movlw   b'10110000'     ; PORTB[6] -- serial clock out
        movwf   TRISB^h'80'     ; PORTB[4] -- serial data in
        bcf     STATUS,RP0

        bsf     STATUS,RP0      ; Serial port stuff
        movlw   b'11000000'     ; CKE=1, SMP=1
        movwf   SSPSTAT^80
        bcf     STATUS,RP0
        movlw   b'00100010'     ; Enable Synchronous Serial Port, set clock
        movwf   SSPCON          ; to F_OSC/64

        return

;;;
;;; Move byte, which is in W, to shift reg and present the byte
;;; on the shift reg's ports.
;;;
SerialOut
        movwf   SSPBUF
        return

Main
        call    InitPorts
        movlw   b'10101010'
        call    SerialOut
        
        ;; Flash LED
lp      bsf     PORTC,4
        call DELAY
        call DELAY
        call DELAY
        bcf     PORTC,4
        call DELAY
        call DELAY
        call DELAY
        goto    lp      
        end
