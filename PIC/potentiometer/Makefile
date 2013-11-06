pot : pot.hex

%.hex : %.asm
	gpasm -pp12f683 $<

clean:
	rm -f *.hex *.lst *.cod *~
