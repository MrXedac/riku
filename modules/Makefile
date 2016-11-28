include ../toolchain.mk

# Sources - for now, we consider that a module is a single C file
CSOURCES=$(wildcard *.c)

# Objects
COBJ=$(CSOURCES:.c=.ko)

# Toolchain flags
CFLAGS=-Tlink.ld -Wall -Werror -nostdlib -fno-builtin -std=gnu99 -ffreestanding -c -Wno-unused-variable -fPIC 
LDFLAGS=-Bshareable -z max-page-size=0x1000

# Include directories : kernel includes
CFLAGS+=-I../kern/include

all: $(COBJ)

clean:
	rm -f $(COBJ)
	rm -f *.o

%.ko: %.o
	$(LD) $(LDFLAGS) $< -o $@
	cp $@ ../boot/boot/modules/$(@:.ko=.rkm)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: all clean
