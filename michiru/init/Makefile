CC=gcc
LD=ld
AS=nasm

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.s=.o)

CFLAGS=-nostdlib -Wall -Werror -ffreestanding -c -fpic -fpie
ASFLAGS=-felf64
LDFLAGS=-Tlink.ld -z max-page-size=0x1000

EXEC=init

$(EXEC): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(AOBJ) $(COBJ)

.PHONY: clean
