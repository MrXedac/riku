include ../../toolchain.mk

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.s=.o)

CFLAGS=-nostdlib -Wall -Werror -ffreestanding -c -fno-builtin -std=gnu99 -g -fPIC -I/home/xedac/riku-sysroot/usr/include/ -mcmodel=large -O0
ASFLAGS=-felf64 -O0
LDFLAGS=-Tlink.ld -z max-page-size=0x1000

EXEC=hello_nl

$(EXEC): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) $^ /home/xedac/riku-sysroot/usr/lib/crt0.o -o $@ -L/home/xedac/riku-sysroot/usr/lib/ -lc -lg -lm -lnosys

%.o: %.c
	C_INCLUDE_PATH=/home/xedac/riku-sysroot/usr/include/ $(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(AOBJ) $(COBJ)

.PHONY: clean
