; PICkit 2 Lesson 1 - "Hello World"
; This turns on DS1 LED on the Low Pin Count Demo Board.
#include <p16f84.inc>
errorlevel -302     ; Turns off spurious "Register in operand not in bank 0".
	org 0
Start
	bsf STATUS,RP0	; select Register Page 0
	bcf	TRISA,0		; make IO Pin A0 an output
	bcf	STATUS,RP0	; back to Register Page 0

	bsf	PORTA,0		; turn on LED C0 (DS1)
	bcf	PORTA,0		; turn on LED C0 (DS1)
	bsf	PORTA,0		; turn on LED C0 (DS1)

    nop
	goto	Start
	end
