IMAGE=rikuldr.bin
SRC_DIR=src

# Toolchain
CC=i386-elf-gcc
AS=nasm
LD=i386-elf-ld

# Sources
CSOURCES=$(wildcard $(SRC_DIR)/*.c)
ASOURCES=$(wildcard $(SRC_DIR)/*.s)

# Objects
COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.c=.o)

CFLAGS=-m32 -Wall -Werror -nostdlib -fno-builtin -std=gnu99 -ffreestanding -c -g -Wno-unused-variable
LDFLAGS=-melf_i386
ASFLAGS=-felf

clean:
	rm -f $(COBJ) $(AOBJ)

all: $(IMAGE)

$(IMAGE): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) -Tlink.ld $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) $< -o $@
