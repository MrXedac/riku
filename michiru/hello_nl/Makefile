SYSROOT=/usr/x86_64-riku

CC=$(SYSROOT)/bin/x86_64-riku-gcc 

CSOURCES=$(wildcard *.c)
COBJ=$(CSOURCES:.c=.o)

EXEC=hello_nl

$(EXEC): $(COBJ)
	$(CC) $^ -o $@ 

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(EXEC) $(AOBJ) $(COBJ)

.PHONY: clean
