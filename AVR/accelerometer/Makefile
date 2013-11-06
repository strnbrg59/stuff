PRG            = accel
OBJ            = $(PRG).o ../utils/utils.o
#MCU_TARGET     = attiny85
MCU_TARGET     = atmega168
EXTRA_DEFS = "-DF_CPU=1000000UL"

ifeq (attiny,$(findstring attiny,$(MCU_TARGET)))
    AVRDUDE_PART = t$(subst attiny,,$(MCU_TARGET))
else
    AVRDUDE_PART = m$(subst atmega,,$(MCU_TARGET))
endif

OPTIMIZE       = -Os

DEFS           =
LIBS           =

include ../make.generic.C++

burn: $(PRG).hex
	avrdude -P$(TTY) -p$(AVRDUDE_PART) -v -cstk500v2 -Uflash:w:./$(PRG).hex

erase:
	avrdude -P$(TTY) -e -p$(AVRDUDE_PART) -v -cstk500v2
