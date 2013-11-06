;
; Reads the potentiometer voltage coming into RA0 on the low-pin-count
; Demo Board and varies the rate at which RC0 flashes.
;


#include <p12f683.inc>
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOD_OFF & _IESO_OFF & _FCMEN_OFF)

        ERRORLEVEL -302

#define SPEAKER GPIO,2
#define EEPROM_BEGIN    0x2100

     cblock 0x70                ; Mirrored in all registers
DCNT1                           ; Outer loop counter
DCNT2                           ; Inner loop counter
DELAYITER                       ; Number of inner loop iterations
TMR0CYCLE                       ; Extra "prescaler"
TMR0CYCLECTR                    ; Counter
ACCEL
W2
_work
_status
     endc

     org       0x0000
     goto      Main

     org       0x0004
     goto      ISR


;----------------------------------------------------------
; Take a g reading somewhere between +-2g and map it to
; something big, so we can hear a significant change in the
; speaker's tone.
; Formula is
;
; if      W< 112 return 255              ---> lowest speaker tone
; if 112<=W<=144 return 255 - 8*(W-112)
; if      W> 144 return 0                ---> highest speaker tone
;
; Input is in W.
; Output is in W.
;----------------------------------------------------------
ScaleG  movwf   ACCEL

        movlw   d'112'          ; Test if ACCEL < 112
        subwf   ACCEL,w
        btfss   STATUS,C        
        retlw   d'255' 

        movlw   d'144'          ; Test if ACCEL > 144
        subwf   ACCEL,w
        btfsc   STATUS,C
        retlw   d'1'

        movlw   d'112'
        subwf   ACCEL,f
        bcf     STATUS,C        ; 112 <= ACCEL <= 144: multiply by 8
        rlf     ACCEL,f         ; and subtract from 255.
        bcf     STATUS,C
        rlf     ACCEL,f
        bcf     STATUS,C
        rlf     ACCEL,f
        movfw   ACCEL
        sublw   h'FF'
        return


;----------------------------------------------------------
; Write W to EEPROM to EEADR as set at entry.
; Increment EEADR for next call to this function.
; Stop writing when EEADR==0xFF (i.e. EEPROM is full).
;----------------------------------------------------------
WriteEEPROM
        banksel EEDAT
        movwf   EEDAT

        movf    EEADR,w         ; If EEADR==h'FF' (i.e. EEPROM is full)
        sublw   h'FF'           ; return without writing anything.
        btfsc   STATUS,Z
        return     

        bcf     INTCON,GIE
        btfsc   INTCON,GIE      ; See AN576 (Microchip document)
        goto    $-2

        banksel EECON1
        bsf     EECON1,WREN     ; Enable EEPROM writing.

        movlw   h'55'           ; Unlock-write voodoo
        movwf   EECON2
        movlw   h'AA'
        movwf   EECON2

        bsf     EECON1,WR       ; Start the write
        bsf     INTCON,GIE
        sleep                   ; Comes out into ISR

        banksel EECON1
        bcf     EECON1,WREN     ; Disable EEPROM writing.

        banksel EEADR
        incf    EEADR,f

        return

;----------------------------------------------------------
; Read accelerometer.  Set DELAYITER to control speaker tone.
;----------------------------------------------------------
ReadAccel
        banksel ADCON0
        bsf     ADCON0,GO_DONE ; start conversion
acclp   btfsc   ADCON0,GO_DONE ; Check if conversion complete
        goto    acclp
        banksel ADRESH
        movf    ADRESH,w
        ;;call    ScaleG

        movwf   DELAYITER
        ;incf    DELAYITER,f     ;; TESTING!
        ;movf    DELAYITER,w
        call    WriteEEPROM
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

ISR     ISR_init
        btfsc   INTCON,T0IF
        call    ISR_tmr0
        banksel PIR1
        btfsc   PIR1,EEIF
        bcf     PIR1,EEIF
        ISR_exit


;----------------------------------------------------------
; TMR0 timeouts trigger a read of the accelerometer.
;----------------------------------------------------------
ISR_tmr0
        bcf     INTCON,T0IF

        decf    TMR0CYCLECTR,f  ; Read accel only once every
        btfss   STATUS,Z        ; TMR0CYCLE entrances to this function.
        return
        movfw   TMR0CYCLE
        movwf   TMR0CYCLECTR

        call    ReadAccel
        return


;---------------------------------- spin delays --------------------
;
;; Inner delay loop called by DELAY
INRDLY  movlw   d'10'
        movwf   DCNT2
dloop2  decfsz  DCNT2,f
        goto    dloop2
        return

;; Delay -- variable
DELAY   movf    DELAYITER,w
        movwf   DCNT1
dloop   call    INRDLY
        decfsz  DCNT1,f
        goto    dloop
        return        
;------------------------------------------------------------------
;
PortSetup        
        banksel ANSEL
        movlw   0x01
        movwf   ANSEL
        movlw   0x01
        banksel TRISIO
        movwf   TRISIO  ; Sll output except 0 (ADC)
        banksel GPIO
        clrf    GPIO
        return

;------------------------------------------------------------------
;
;------------------------------------------------------------------
A2DSetup
        banksel ANSEL
        movf    ANSEL,w
        andlw   b'10001111'
        iorlw   b'01010000'     ; fosc/16 (4:6=101.
        movwf   ANSEL
        banksel ADCON0          ; ADFM=0 (result will be in ADRESH),     
        movlw   b'00000001'     ; VDD is reference (0), channel=0000,
        movwf   ADCON0          ; GO_DONE=0, turn on the A2D module (1)
        return           


;------------------------------------------------------------------
;
;------------------------------------------------------------------
TMR0Setup
        banksel OPTION_REG
        movlw   b'11000111'     ; Prescaler mode 111 gives an interrupt
        movwf   OPTION_REG^0x80 ; every 65 milliseconds (at our 4MHz speed)
                                ; i.e. 15 per second...
        movlw   d'1'            ;
        movwf   TMR0CYCLE       ; ...times TMR0CYCLE.
        movwf   TMR0CYCLECTR
        return

;------------------------------------------------------------------
; Activate TMR0 and EE interrupts.
;------------------------------------------------------------------
InitIntrps
        bsf     INTCON,T0IE
        bsf     INTCON,PEIE
        banksel PIE1
        bsf     PIE1,EEIE
        bsf     INTCON,GIE
        return


InitEEPROM
        banksel EEADR
        clrf    EEADR        ; Position to beginning of EEPROM space.
        return

Main    movlw   d'100'
        movwf   DELAYITER
        call    PortSetup
        call    A2DSetup
        call    TMR0Setup
        call    InitEEPROM
        call    InitIntrps

mainlp  nop
        banksel GPIO
        bsf     SPEAKER
        call    DELAY
        bcf     SPEAKER
        call    DELAY
        goto    mainlp
        end
