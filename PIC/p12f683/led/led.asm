#include <p12F683.inc>
    __config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOD_OFF & _IESO_OFF & _FCMEN_OFF)


        cblock 0x70             ; 16 bytes, mirrored between the banks.
DCNT1                           ; Outer loop counter.
DCNT2                           ; Inner loop counter.
        endc
        
        org 0
        goto MAIN

;----------------------------------------------------------------------
; Put I/O ports in all-digital mode, all output.
;----------------------------------------------------------------------
InitPorts
        bsf     STATUS,RP0
        clrf    ANSEL^h'80'
        clrf    TRISIO^h'80'
        bcf     STATUS,RP0
        clrf    GPIO


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

loop    bsf     GPIO,0
        call    DELAY
        bcf     GPIO,0
        call    DELAY

        bsf     GPIO,1
        call    DELAY
        bcf     GPIO,1
        call    DELAY

        bsf     GPIO,2
        call    DELAY
        bcf     GPIO,2
        call    DELAY

        bsf     GPIO,4
        call    DELAY
        bcf     GPIO,4
        call    DELAY

        goto    loop

        end
