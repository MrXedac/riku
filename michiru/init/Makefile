include ../../toolchain.mk

LIBC=../../libc/

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.s=.o)

CFLAGS=-nostdlib -nostdinc -Wall -Werror -ffreestanding -c -fno-builtin -std=gnu99 -g -fPIC -I$(LIBC)/include/ -O0
ASFLAGS=-felf64 -O0
LDFLAGS=-Tlink.ld -z max-page-size=0x1000

EXEC=init

$(EXEC): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) $^ -o $@ -L$(LIBC)/lib/ -lc

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(AOBJ) $(COBJ)

.PHONY: clean
