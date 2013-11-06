#include <p16F690.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)


#define FLASHING_LED  PORTA,5    ; all-purpose indicator
#define LCD_PORT      PORTC
#define SR_OE         LCD_PORT,0 ; shift register output enable
#define LCD_RS        LCD_PORT,2 ; 0=cmd, 1=data
#define LCD_RW        LCD_PORT,3 ; 0=write, 1=read
#define LCD_E         LCD_PORT,4
#define LCD_BF        LCD_PORT,5 ; Busy flag

        cblock 0x20
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
EE_STR_ADDR                     ; Address of a string in EEPROM
W2                              ; To save W
        endc
        cblock 0x70             ; Mirrored in all registers
_work
_status
        endc

#define EEPROM_BEGIN     0x2100
        org EEPROM_BEGIN
KUNI    de      a'K', a'U', a'N', a'I', h'0'
LEMEL   de      a'L', a'E', a'M', a'E', a'L', h'0'
KUNILEMEL de    a'K', a'U', a'N', a'I', a' ', a'L', a'E', a'M', a'E', a'L', h'0'
YEAGER  de      a'Y', a'E', a'A', a'G', a'E', a'R', h'0'
ABC     de      a'A', a'B', a'C', a'D', a'E', a'F', a'G', a'H', a'I', a'J', a'K', a'L', a'M', a'N', a'O', a'P', a'Q', a'R', a'S', a'T', h'0'

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
        movlw   b'00100000'     ; PORTC: all output except LCD_BF
        movwf   TRISC ^ h'80'
        movlw   b'10110000'     ; PORTB[6] -- serial clock out
        movwf   TRISB^h'80'     ; PORTB[4] -- serial data in
        bcf     TRISA ^ h'80',5 ; FLASHING_LED
        bcf     TRISA ^ h'80',1 ; For pulling down RA2/INT when button pushed
        bsf     TRISA ^ h'80',2 ; INT
        bcf     STATUS,RP0
        bcf     PORTA,1         ; Fixed -- to pull down RA2/INT

        bsf     STATUS,RP0      ; Serial port stuff
        movlw   b'11000000'     ; CKE=1, SMP=1
        movwf   SSPSTAT^80
        bcf     STATUS,RP0
        movlw   b'00100010'     ; Enable Synchronous Serial Port, set clock
        movwf   SSPCON          ; to F_OSC/64

        return

InitIntrps
        bsf     STATUS,RP0      
        bcf     OPTION_REG^h'80',INTEDG ; Set interrupt to falling-edge
        bcf     STATUS,RP0
        bsf     INTCON,GIE      ; Enable interrupts 
        bsf     INTCON,INTE
        bcf     INTCON,INTF
        return

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


;----------------------------------------------------------------------
; Even though we have only one interrupt (so far), we have to check for
; it specifically, as other kinds of interrupts (I know not which) seem
; to be generated at start-up, and we don't want them to trigger our
; ISR_lcd.
;----------------------------------------------------------------------
ISR     btfsc   INTCON,INTF
        goto    ISR_uart
        retfie


ISR_lcd ISR_init
        bsf     FLASHING_LED
        call    DELAY
        bcf     FLASHING_LED
        bcf     INTCON,INTF
        call    LCDinit
        movlw   KUNILEMEL - EEPROM_BEGIN
        call    LCDputStr
        call    DELAY
        call    DELAY
        call    DELAY
        call    DELAY
        call    DELAY
        call    DELAY
        call    DELAY
        call    DELAY
        movlw   b'00001000'     ; display off, blinking cursor
        call    LCDputCmd
        ISR_exit


;------------------------------------------------------------------
; Move byte, which is in W, to shift reg and present the byte
; on the shift reg's ports.
;
SerialOut
        movwf   SSPBUF
        return

;------------------------------------------------------------------
; Return when LCD isn't busy.
;
WhileBusy
        bcf     SR_OE           ; So shift register won't interfere with LCD_B
        call    INRDLY          ; Give shift register time to react.
wbl     bcf     LCD_RS          ; cmd
        bsf     LCD_RW          ; read
        bsf     LCD_E
        bcf     LCD_E
        btfsc   LCD_BF
        goto    wbl
        bsf     SR_OE
        call    INRDLY
        return  
;------------------------------------------------------------------

;------------------------------------------------------------------
; Send command to the LCD.
; Command byte should be in W
;
LCDputCmd
        movwf   W2
        call    SerialOut
        call    WhileBusy
        bcf     LCD_RS          ; cmd
        bcf     LCD_RW          ; write
        bsf     LCD_E
        bcf     LCD_E
        movfw   W2
        return
;------------------------------------------------------------------

;------------------------------------------------------------------
; Send a character to the LCD.
; Character code (byte) should be in W
;
LCDputChar
        movwf   W2
        call    SerialOut
        call    WhileBusy
        bsf     LCD_RS          ; data
        bcf     LCD_RW          ; write
        bsf     LCD_E
        bcf     LCD_E
        movfw   W2
        return
;------------------------------------------------------------------

;------------------------------------------------------------------
; Initialize the LCD: display on, 2 lines, blinking cursor
;
LCDinit call    WhileBusy
        movlw   b'00001111'     ; display on, blinking cursor
        call    LCDputCmd
        movlw   b'00111000'     ; 8 bits, 2 lines, 5x8 pixels
        call    LCDputCmd
        return


;------------------------------------------------------------------
; Copies EEPROM byte, found at address W, onto W.
; Modifies EEADR and EEDATA and doesn't bother to set them back.
;
EEbyte  bsf     STATUS,RP1
        movwf   EEADR^h'100'
        movlw   b'00000001'     ; Set RD for Read cycle and
        bsf     STATUS,RP0
        movwf   EECON1^h'180'   ; read datum into EEDATA
        bcf     STATUS,RP0
        movfw   EEDATA^h'100'
        bcf     STATUS,RP1
        return

;------------------------------------------------------------------
; Displays null-terminated string, found at address W in EEPROM, 
; on LCD.  Note W has to be relative to EEPROM_BEGIN (so if you want
; to display the string at label FOO (inserted into EEPROM with the
; de command), you need to say "movlw FOO-EEPROM_BEGIN" before calling
; this function.
;
; Doesn't restore W.
LCDputStr
        movwf   EE_STR_ADDR
        movlw   b'00000010'
        call    LCDputCmd          ; Return cursor home
        movlw   b'00000001'
        call    LCDputCmd          ; Clear display
        movfw   EE_STR_ADDR
strloop call    EEbyte             ; Char moved to W: if ==0 return
        movwf   W2
        sublw   h'0'
        btfsc   STATUS,Z
        return
        movfw   W2
        call    LCDputChar
        incf    EE_STR_ADDR,f
        movfw   EE_STR_ADDR
        goto    strloop


Main    call    InitPorts
        call    InitIntrps
loop    goto    loop
        end
