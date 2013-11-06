CXX             = avr-g++
override CXXFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(EXTRA_DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

$(PRG).elf: $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

include ../make.generic
