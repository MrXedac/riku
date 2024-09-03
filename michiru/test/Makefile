include ../../toolchain.mk

SYSROOT=/home/xedac/sysroot

CSOURCES=$(wildcard *.c)
ASOURCES=$(wildcard *.s)

COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.s=.o)

CFLAGS=-nostdlib -Wall -Werror -ffreestanding -c -fno-builtin -std=gnu99 -g -fPIC -I$(SYSROOT)/usr/include/ -mcmodel=large -O0
ASFLAGS=-felf64 -O0
LDFLAGS=-Tlink.ld -z max-page-size=0x1000

EXEC=test

$(EXEC): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) $^ $(SYSROOT)/usr/lib/crt0.o -o $@ -L$(SYSROOT)/usr/lib/ -lc -lg -lm -lnosys

%.o: %.c
	C_INCLUDE_PATH=$(SYSROOT)/usr/include/ $(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f $(EXEC) $(AOBJ) $(COBJ)

.PHONY: clean
