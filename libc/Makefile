include ../toolchain.mk

# Directories
SRCDIR=src
INCDIR=include

# Generate source and object list
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJ=$(SOURCES:.c=.o)
ASOURCES=$(wildcard $(SRCDIR)/*.S)
AOBJ=$(ASOURCES:.S=.o)

# C compiler flags
CFLAGS=-c -ffreestanding -nostdlib -Wall -Werror -Wextra -fno-builtin -Wno-unused-parameter -Wno-unused-variable -O2 -fno-caller-saves -std=gnu99
CFLAGS+=-I$(INCDIR)

ASFLAGS=-c -I. -I./include -fomit-frame-pointer --freestanding -nostdlib -fno-stack-protector

# Archiver flags
ARFLAGS=rcs

# Lib directory and output file
LIBDIR=lib
LIB=$(LIBDIR)/libc.a

.PHONY: all

all: $(LIBDIR) $(LIB)

clean:
	rm -rf $(LIBDIR) $(OBJ) $(AOBJ)

$(LIBDIR):
	mkdir -p $(LIBDIR)

$(LIB): $(OBJ) $(AOBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@
