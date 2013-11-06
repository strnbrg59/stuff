#include <P12F683.INC>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOD_OFF & _IESO_OFF & _FCMEN_OFF)

        ERRORLEVEL -302

        cblock 0x70
_work
_status
        endc

        org     0x0000
        goto    Main
        org     0x0004
        goto    ISR


;----------------------------------------------------------
; Interrupt entry and exit macros
;----------------------------------------------------------
ISR_init macro
        movwf    _work
        swapf   STATUS,w
        movwf   _status
        endm
ISR_exit macro
        swapf   _status,w
        movwf   STATUS
        swapf   _work,f
        swapf   _work,w
        retfie
        endm

ISR     ISR_init
        banksel PIR1
        btfsc   PIR1,EEIF
        bcf     PIR1,EEIF
        ISR_exit



Main    bsf     INTCON,PEIE             ; Init interrupts
        bsf     INTCON,GIE
        banksel PIE1
        bsf     PIE1,EEIE

        banksel EEADR
        clrf    EEADR            ; Beginning of EEPROM space.

        movlw   h'EE'
        movwf   EEDAT            ; Byte to write.

lp      nop
        banksel EECON1
        bsf     EECON1,WREN      ; Enable EEPROM writing.

        bcf     INTCON,GIE       ; Disable INTs

        movlw   h'55'            ; Unlock-write voodoo
        movwf   EECON2
        movlw   h'AA'
        movwf   EECON2

        bsf     EECON1,WR        ; Start the write
        bsf     INTCON,GIE       ; Re-enable INTs
        sleep

        banksel EECON1
        bcf     EECON1,WREN      ; Disable writes

        banksel EEADR
        incf    EEADR,f          ; Position to next EEPROM address.

        goto    lp

        end
