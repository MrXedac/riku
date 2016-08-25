include ../toolchain.mk

IMAGE=kern.bin
SRC_DIR=src

# Sources
CSOURCES=$(wildcard $(SRC_DIR)/boot/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/int/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/io/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/mm/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/task/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/hw/*.c)
CSOURCES+=$(wildcard $(SRC_DIR)/vfs/*.c)

ASOURCES=$(wildcard $(SRC_DIR)/boot/*.s)
ASOURCES+=$(wildcard $(SRC_DIR)/int/*.s)
ASOURCES+=$(wildcard $(SRC_DIR)/io/*.s)
ASOURCES+=$(wildcard $(SRC_DIR)/mm/*.s)
ASOURCES+=$(wildcard $(SRC_DIR)/task/*.s)

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
