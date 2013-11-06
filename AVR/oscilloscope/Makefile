PRG            = adc
OBJ            = $(PRG).o ../uart/uart.o
ATMEGA_NUM     = 168
MCU_TARGET     = atmega${ATMEGA_NUM}

OPTIMIZE       = -O3

DEFS           =
LIBS           =

include ../make.generic.C++

burn: $(PRG).hex
	avrdude -P$(TTY) -pm${ATMEGA_NUM} -v -cstk500v2 -Uflash:w:./$(PRG).hex

erase:
	avrdude -P$(TTY) -e -pm${ATMEGA_NUM} -v -cstk500v2
