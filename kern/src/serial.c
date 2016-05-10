#include "ioport.h"
#include "serial.h"

void init_serial() {
	outb(PORT + 1, 0x00);    // Disable all interrupts
	outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(PORT + 1, 0x00);    //                  (hi byte)
	outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

/**
 * \fn int serial_received()
 * \brief Checks whether we received some data on the serial port
 * \return 1 if some data is available, 0 else
 */
int serial_received() {
	return inb(PORT + 5) & 1;
}

/**
 * \fn char read_serial()
 * \brief Gets a character from the serial port
 * \return The character
 */
char read_serial() {
	while (serial_received() == 0);
	return inb(PORT);
}

/**
 * \fn int is_transmit_empty()
 * \brief Checks whether our serial line buffer is empty or not
 * \return 0 if it's not empty, 1 else
 */
int is_transmit_empty() {
	return inb(PORT + 5) & 0x20;
}

/**
 * \fn void write_serial(char a)
 * \brief Writes a character into the serial port
 * \param a The character to write
 */
void write_serial(char a) {
	while (is_transmit_empty() == 0);
	outb(PORT,a);
}

void slputs(char* a)
{
	int i = 0;
	while (a[i])
		write_serial(a[i++]);
}

void slputhex(int n)
{
	int tmp;
	
	slputs("0x");
	
	char noZeroes = 1;
	
	int i;
	for (i = 28; i > 0; i -= 4)
	{
		tmp = (n >> i) & 0xF;
		if (tmp == 0 && noZeroes != 0)
		{
			continue;
		}
		
		if (tmp >= 0xA)
		{
			noZeroes = 0;
			write_serial(tmp-0xA+'A' );
		}
		else
		{
			noZeroes = 0;
			write_serial( tmp+'0' );
		}
	}
	
	tmp = n & 0xF;
	if (tmp >= 0xA)
	{
		write_serial (tmp-0xA+'A');
	}
	else
	{
		write_serial (tmp+'0');
	}
	
}

void slputdec(int n)
{
	
	if (n == 0)
	{
		write_serial('0');
		return;
	}
	
	int acc = n;
	char c[32];
	int i = 0;
	while (acc > 0)
	{
		c[i] = '0' + acc%10;
		acc /= 10;
		i++;
	}
	c[i] = 0;
	
	char c2[32];
	c2[i--] = 0;
	int j = 0;
	while(i >= 0)
	{
		c2[i--] = c[j++];
	}
	slputs(c2);
}

