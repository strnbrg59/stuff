;;; 
;;; Testing finger-switch-operated interrupt
;;;
;;; On 16F690, INT is RA2.
;;; Wiring hint: when RA2 isn't high, it has to be definitely low, so
;;; connect it, permanently, to ground, via any resistor.
;;; 
        
        
#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOD_OFF & _IESO_OFF & _FCMEN_OFF)

        cblock 0x20
_work
_status
DCNT1
DCNT2
        endc

        org 0
        goto MAIN

        org 004
        goto ISR

                
;; Inner loop
DELAY2  movlw   h'8F'
        movwf   DCNT2
DLOOP2  decfsz  DCNT2,f
        goto DLOOP2
        return
;; Outer loop
DELAY1  movlw   h'FF'
        movwf   DCNT1
DLOOP1  call    DLOOP2
        decfsz  DCNT1,f
        goto    DLOOP1
        return        
        

INITPORTS 
        bsf     STATUS,RP1      ; Put I/O ports in all-digital mode.
        clrf    ANSEL ^ h'100'
        clrf    ANSELH ^ h'100'
        bcf     STATUS,RP1

        clrf    PORTC
        
        bsf     STATUS,RP0     
        bsf     TRISA ^ h'80',2 ; PORTA[2] input -- the interrupt
        clrf    TRISC ^ h'80'   ; all output
        bcf     STATUS,RP0
        return


ISR_init macro
        movwf    _work
        swapf   STATUS,w
        movwf   _status
        bcf     INTCON,INTF
        endm
        
ISR_exit macro
        swapf   _status,w
        movwf   STATUS
        swapf   _work,f
        swapf   _work,w
        retfie
        endm
                
ISR     ISR_init
        bsf     PORTC,0
        call    DELAY1
        bcf     PORTC,0
        ISR_exit
        
MAIN    call    INITPORTS
        bcf     PORTA,2

        bsf     STATUS,RP0      ; Set interrupt to rising-edge
        bsf     OPTION_REG^h'80',INTE
        bcf     STATUS,RP0

        
        bsf     INTCON,INTE
        bsf     INTCON,GIE
SPIN    nop
        goto SPIN

        end
