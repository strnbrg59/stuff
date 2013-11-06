;;;
;;; Use TMR0 to blink the first 3 LEDs in sequence, about one per three
;;; seconds, while blinking the fourth LED every second.  At the same time,
;;; a pushbutton connected to RA2/INT controls an LED connected to RA0.
;;; 
        
#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


        cblock 0x20
NSEC                    ; # seconds between LEDs 0-3
NSECCTR                 ; Counts up to NSEC
_work
_status
W2                      ; Extra "work" register
BLINKMASK               ; For blinking LED at PORTC[3]
CYCLEMASK               ; For cycling LEDs PORTC[0:2]
                        ; PORTC <-- BLINKMASK | CYCLEMASK
TMR0START
        endc
        
        
        org 0
        goto MAIN

        org 004
        goto ISR

;; Initialize constants for quick TMR0 rollover.
INIT_test
        movlw   d'0'
        movwf   TMR0START
        movlw   d'1'
        movwf   NSEC
        movwf   NSECCTR
        bsf     STATUS,RP0
        movlw   b'11001000'     ; Configure TMR0
        movwf   OPTION_REG^0x80
        bcf     STATUS,RP0
        return

;; Initialize constants for once-per-second blinking.
INIT_production        
        movlw   d'205'
        movwf   TMR0START
        movlw   d'98'
        movwf   NSEC
        movwf   NSECCTR
        bsf     STATUS,RP0
        movlw   b'11000111'     ; Configure TMR0
        movwf   OPTION_REG^0x80
        bcf     STATUS,RP0
        return

MAIN
        ;; Init constants
        ;call    INIT_test
        call    INIT_production
        clrf    CYCLEMASK
        bsf     CYCLEMASK,2     ; First LED to go on will be #0
        clrf    BLINKMASK
        
        ;; Init PORTC: all-digital, all-output, cleared
        bsf     STATUS,RP1      ; All-digital
        clrf    ANSEL ^ h'100'
        clrf    ANSELH ^ h'100'
        bcf     STATUS,RP1
        clrf    PORTC           ; All-output
        clrf    PORTA
        bsf     STATUS,RP0
        clrf    TRISC^0x80
        clrf    TRISA^0x80
        bsf     TRISA^h'80',2 ; RA2 = INT
        bcf     STATUS,RP0

        bsf     INTCON,GIE      ; Enable interrupts
        bsf     INTCON,T0IE
        bsf     INTCON,INTE

        ;; Set TMR0 to a value to value that gives whole fraction of 1 second
        movfw   TMR0START
        movwf   TMR0


main_A  goto main_A             ; loop forever

;;;;;;;;;;;;;;;;;;;;;; Interrupt service stuff ;;;;;;;;;;;;;;;;;;;;

ISR     btfsc   INTCON,INTF
        goto    ISR_button
        goto    ISR_tmr0


ISR_init macro
        movwf    _work
        swapf   STATUS,w
        movwf   _status
        endm
ISR_exit macro
        movfw   TMR0START
        movwf   TMR0
        swapf   _status,w
        movwf   STATUS
        swapf   _work,f
        swapf   _work,w
        bsf     INTCON,GIE
        retfie
        endm

;; Toggle RA0 (need to stick LED in there)
ISR_button
        ISR_init
        bcf     INTCON,INTF

        movfw   PORTA
        movwf   W2
        bcf     PORTA,0
        btfss   W2,0
        bsf     PORTA,0
        ISR_exit        ; calls retfie
        

        
ISR_tmr0
        ISR_init
        bcf     INTCON,T0IF

        ;; Cycle through LEDs 0-2, and blink LED3
        decfsz  NSECCTR,f
        goto    ISR_c
        call    CycleLEDs

ISR_c   movfw   BLINKMASK
        iorwf   CYCLEMASK,w
        movwf   PORTC
        ISR_exit



CycleLEDs
        ;; Got here because NSECCTR=0; time to cycle to next LED.
        ;; Blink LED3
        movfw   BLINKMASK
        movwf   W2
        bcf     BLINKMASK,3
        btfss   W2,3
        bsf     BLINKMASK,3

        btfsc   CYCLEMASK,2
        goto    Cycle_wrap
        bcf     STATUS,C        ; Shift CYCLEMASK right
        rlf     CYCLEMASK,f
        goto    Cycle_cont
Cycle_wrap
        clrf    CYCLEMASK
        bsf     CYCLEMASK,0
Cycle_cont
        movfw   NSEC            ; Reset NSECCTR for new countdown
        movwf   NSECCTR
        return
                
        goto    $
        end
        
