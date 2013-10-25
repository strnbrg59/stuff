ifeq (,$(filter build%,$(notdir $(CURDIR))))
include target.mk
else

VPATH = $(SRCDIR)

mybin : main.o mylib.so
	gcc $^ -o $@

mylib.so : libsrc.o
	gcc -shared $^ -Wl -o $@

%.o : %.c
	gcc -c -g -Wall $< -o $@

endif
