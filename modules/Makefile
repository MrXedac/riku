include ../toolchain.mk

# Sources - for now, we consider that a module is a single C file
CSOURCES=$(wildcard *.c)

# Objects
COBJ=$(CSOURCES:.c=.ko)

# Toolchain flags
CFLAGS=-Wall -Werror -nostdlib -fno-builtin -std=gnu99 -ffreestanding -c -Wno-unused-variable -mcmodel=large

# Include directories : kernel includes
CFLAGS+=-I../kern/include

all: $(COBJ)

clean:
	rm -f $(COBJ)

%.ko: %.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: all clean
