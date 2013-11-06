OBJ            = $(PRG).o
MCU_TARGET     = atmega8515

OPTIMIZE       = -Os

DEFS           =
LIBS           =

include ../make.generic

burn: $(PRG).hex
	~/usr/avr/bin/avrdude -P$(TTY) -pm8515 -v -cstk500v2 -Uflash:w:./$(PRG).hex

erase:
	~/usr/avr/bin/avrdude -P$(TTY) -e -pm8515 -v -cstk500v2
