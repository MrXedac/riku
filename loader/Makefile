include ../toolchain.mk

IMAGE=rikuldr.bin
SRC_DIR=src

# Sources
CSOURCES=$(wildcard $(SRC_DIR)/*.c)
ASOURCES=$(wildcard $(SRC_DIR)/*.s)

# Objects
COBJ=$(sort $(CSOURCES:.c=.o))
AOBJ=$(sort $(ASOURCES:.s=.o))

# Toolchain flags
CFLAGS=-m32 -Wall -Werror -nostdlib -fno-builtin -std=gnu99 -ffreestanding -c -g -Wno-unused-variable -fPIC -fno-stack-protector 
LDFLAGS=-melf_i386 -z max-page-size=0x1000
ASFLAGS=-felf

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

