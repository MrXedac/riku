IMAGE=kern.bin
SRC_DIR=src

# Toolchain
CC=x86_64-elf-gcc
AS=/opt/local/bin/nasm
LD=x86_64-elf-ld

# Sources
CSOURCES=$(wildcard $(SRC_DIR)/*.c)
ASOURCES=$(wildcard $(SRC_DIR)/*.s)

# Objects
COBJ=$(CSOURCES:.c=.o)
AOBJ=$(ASOURCES:.s=.o)

# Toolchain flags
CFLAGS=-Wall -Werror -nostdlib -fno-builtin -std=gnu99 -ffreestanding -c -Wno-unused-variable -mcmodel=large
LDFLAGS=-z max-page-size=0x1000
ASFLAGS=-felf64

# Include directories
CFLAGS+=-Iinclude

clean:
	rm -f $(COBJ) $(AOBJ) $(IMAGE)

all: $(IMAGE)

$(IMAGE): $(AOBJ) $(COBJ)
	$(LD) $(LDFLAGS) -Tlink.ld $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@
