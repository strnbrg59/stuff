;;; 
;;; Kitchen timer:
;;;   1. 7-seg LED counts down by minutes.
;;;   2. Decimal point (RB6) blinks once per second.
;;;   3. Pushbutton connected to RA2/INT bumps digit up by 1.
;;;   4. Buzzer, connected to RA1, goes off when digit reaches 0.
;;;
;;; Rules for using the HE4511 decoder (should we want to use it in the
;;; future):
;;;     DIPs 6, 7 and 8 have to be on.
;;;     DIP 5 should be off.
;;;     Remaining DIPs control the digit (through the intermediary
;;;     HE4511 BCD-to-7-segment decoder).
        
        
#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOD_OFF & _IESO_OFF & _FCMEN_OFF)

#define RHDP 6

        cblock 0x20
BITMASKS:10                     ; Has to start at 0x20!!
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
DIGIT                           ; Current displayed digit
W2                              ; Extra work register
TMR0START
NOVF                            ; # TMR0 overflows in a second
NOVFCTR
NSEC                            ; # seconds between digits
NSECCTR
_work
_status                
        endc

                
        org 0x00
        goto Main

        org 0x04
        goto ISR
        

;---------------------------------- spin delay --------------------
;
;; Inner delay loop called by DELAY
INRDLY  movlw   h'FF'
        movwf   DCNT2
dloop2  decfsz  DCNT2,f
        goto    dloop2
        return

;; Delay -- about 1/4 second -- good for debouncing button.  But for
;; timing the digit and decimal point we use TMR0.
DELAY   movlw   h'FF'
        movwf   DCNT1
dloop   call    INRDLY
        decfsz  DCNT1,f
        goto    dloop
        return        
;------------------------------------------------------------------

         
;; Display the digit indicated by contents of the GPR named DIGIT
ShowDigit
        movlw   h'20'           ; Address of BITMASKS[0]
        addwf   DIGIT,w         ; DIGIT is in {0,...,9}
        movwf   FSR
        movf    0,w
        movwf   PORTC           ; Set first six bits
        bcf     PORTA,0         ; Seventh bit is PORTA[0]
        btfsc   0,6
        bsf     PORTA,0
        return

;; Initialize constants for quick TMR0 rollover.
InitTmrConsts_test
        movlw   d'0'
        movwf   TMR0START
        movwf   TMR0
        movlw   d'1'
        movwf   NOVF
        movwf   NOVFCTR
        movlw   d'2'
        movwf   NSEC
        movwf   NSECCTR
        bsf     STATUS,RP0
        movlw   b'10001000'     ; Configure TMR0
        movwf   OPTION_REG^0x80
        bcf     STATUS,RP0
        return

;; Initialize constants for once-per-second blinking.
InitTmrConsts_production        
        movlw   d'17'
        movwf   TMR0START
        movwf   TMR0
        movlw   d'8'
        movwf   NOVF
        movwf   NOVFCTR
        movlw   d'120'
        movwf   NSEC
        movwf   NSECCTR
        bsf     STATUS,RP0
        movlw   b'10000111'     ; Configure TMR0
        movwf   OPTION_REG^0x80
        bcf     STATUS,RP0
        return
        
InitBitMasks
        movlw   b'00111111'     ; 0
        movwf   BITMASKS+0
        movlw   b'00000110'     ; 1
        movwf   BITMASKS+1
        movlw   b'01011011'     ; 2
        movwf   BITMASKS+2
        movlw   b'01001111'     ; 3
        movwf   BITMASKS+3
        movlw   b'01100110'     ; 4
        movwf   BITMASKS+4
        movlw   b'01101101'     ; 5
        movwf   BITMASKS+5
        movlw   b'01111100'     ; 6
        movwf   BITMASKS+6
        movlw   b'00000111'     ; 7
        movwf   BITMASKS+7
        movlw   b'01111111'     ; 8
        movwf   BITMASKS+8
        movlw   b'01101111'     ; 9
        movwf   BITMASKS+9
        return


InitPorts 
        bsf     STATUS,RP1      ; Put I/O ports in all-digital mode.
        movlw	b'00001111'
        movwf	ANSEL^h'100'    ; Takes care of PORTC[0:3]
        movlw	b'00001100'
        movwf	ANSELH^h'100'   ; PORTC[6:7]
        bcf     STATUS,RP1

        clrf    PORTA
        clrf    PORTC

        
        bsf     STATUS,RP0      ; Set TRIS{A,C} to all output except RA2
        clrf    TRISA ^ h'80'
        bsf     TRISA ^ h'80',2 ; RA2 = INT
        bcf     TRISB ^ h'80',RHDP ; pulsating decimal point
        clrf    TRISC ^ h'80'                
        bcf     STATUS,RP0

        return

InitIntrps
        bsf     STATUS,RP0      ; Set interrupt to falling-edge
        bcf     OPTION_REG^h'80',INTEDG
        bcf     STATUS,RP0
        bsf     INTCON,GIE      ; Enable interrupts 
        bsf     INTCON,INTE
        bsf     INTCON,T0IE
        return


Main
        call    InitPorts
        call    InitBitMasks
        call    InitIntrps
        call    InitTmrConsts_production
        movlw   d'9'            ; Init displayed digit
        movwf   DIGIT
        call    ShowDigit
mloop   goto    mloop
        


;;;
;;;;;;;;;;;;;;;;;;;;;;;;;; Interrupt stuff ;;;;;;;;;;;;;;;;;;;;
;;; 
ISR     btfsc   INTCON,INTF
        goto    ISR_btn
        goto    ISR_tmr


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
        bsf     INTCON,GIE
        retfie
        endm


ISR_btn ISR_init                ; Button handler: increment DIGIT, modulo 10
        bcf     INTCON,INTF     ; (but from 9 go to straight to 1).
        movf    DIGIT,w
        sublw   d'9'
        btfsc   STATUS,Z
        movwf   DIGIT           ; DIGIT was 9, now it's 0 and about to incr.
        incf    DIGIT,f
        call    ShowDigit
        call    DELAY           ; Wait out any switch bounce.  As long as this
        call    DELAY           ; is only place we call DELAY, no need to reset
        ISR_exit                ; DELAY's counter (DCNT1).


ISR_tmr ISR_init
        bcf     INTCON,T0IF
        movfw   TMR0START
        movwf   TMR0

        ;; Pulse RHDP, if NOVFCTR==0, then reset NOVFCTR
        decfsz  NOVFCTR,f
        goto ISR_tmr_exit
        movfw   PORTB
        movwf   W2
        bcf     PORTB,RHDP
        btfss   W2,RHDP
        bsf     PORTB,RHDP
        movfw   NOVF
        movwf   NOVFCTR

        ;; Decrement and display digit, if NSECCTR==0, then reset NSECCTR
        decfsz  NSECCTR,f
        goto ISR_tmr_exit

        decf    DIGIT,f         ; Decr DIGIT, stop looping when 0
        btfsc   STATUS,Z
        call    BuzzFinish      ; Buzz, in an infinite loop
        movfw   NSEC
        movwf   NSECCTR
        call    ShowDigit
ISR_tmr_exit ISR_exit


BuzzFinish
        bcf     INTCON,GIE      ; No more interrupts, we're done
        call    ShowDigit       ; Should show zero
b_loop  bsf     PORTA,1
        call    DELAY
        bcf     PORTA,1
        call    DELAY
        goto    b_loop
        return                  ; should never get here

        end
