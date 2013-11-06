;;
;; Pushbutton wired to RA2/INT: when pushed, sends a byte to a receiving
;; PIC.
;;
;; Pins of a null modem cable, viewed looking at the cable end:
;;
;;    #    #    #   #
;;  #   TX   RX   #   GND
;;
;; The '#' marks indicate pins we don't care about.
;;

#include <p16F690.inc>
        __config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


#define BAUD    d'4800'
#define XTAL    d'4'    ; megahertz -- clock speed at VDD=5V
#define X       ((XTAL*d'1000000')/(d'64'*BAUD))-1
#define UART_RX PORTB,5
#define UART_TX PORTB,7


;--------------------------- variable declarations-----------------
;
        cblock 0x20
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
        endc
        cblock 0x70             ; Mirrored in all registers
_work
_status
GROWING_CHAR
        endc

        org 0x00
        goto    Main

        org 0x04
        goto    ISR

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



;----------------------------------------------------------
; Set up UART
;----------------------------------------------------------
UART_setup
        bsf     STATUS,RP0
        movlw   X
        movwf   SPBRG^h'80'     ; Set baud rate.
        movlw   b'00100000'     ; 8 data bits, TX enabled
        movwf   TXSTA^h'80'
        bcf     STATUS,RP0
        movlw   b'10010000'     ; UART enabled, 8 data bits
        movwf   RCSTA           ; Enables RX too, even though we don't need it.
        return


;----------------------------------------------------------
; Byte is in W
;----------------------------------------------------------
UART_putchar
        btfss   PIR1,TXIF       ; Poll TX buffer for readiness
        goto    UART_putchar
        movwf   TXREG
        return


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



;----------------------------------------------------------
; Send a byte to receiver PIC.
;----------------------------------------------------------
ISR_uart
        movlw   d'0'
        movwf   GROWING_CHAR
uartlp  nop
        movfw   GROWING_CHAR
        call    UART_putchar
        bsf     PORTC,0         ; Indicator LED
        call    DELAY
        bcf     PORTC,0
        call    DELAY
        incf    GROWING_CHAR,f
        goto    uartlp
        return

;----------------------------------------------------------------------
; Even though we have only one interrupt (so far), we have to check for
; it specifically, as other kinds of interrupts (I know not which) seem
; to be generated at start-up, and we don't want them to trigger our
; ISR_uart.
;----------------------------------------------------------------------
ISR     ISR_init
        btfsc   INTCON,INTF
        goto    ISR_uart
        ISR_exit

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
        bcf     PORTA,1
        clrf    PORTB
        clrf    PORTC


InitIntrps
        bsf     STATUS,RP0      
        bsf     OPTION_REG^h'80',INTEDG ; Set interrupt to rising-edge
        bsf     TRISA^h'80',2   ; RA2/INT
        bcf     STATUS,RP0

        bsf     INTCON,GIE      ; Enable interrupts 
        bcf     INTCON,INTF
        bsf     INTCON,INTE
        return


Main    call    InitPorts
        call    InitIntrps
        call    UART_setup
        ;;call    ISR_uart        ; Just a test!
lp      nop
        goto    lp
        end
