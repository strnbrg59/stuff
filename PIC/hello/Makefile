hello : hello.hex

%.hex : %.asm
	gpasm -pp16f84 $<

clean:
	rm -f *.hex *.lst *.cod *~
