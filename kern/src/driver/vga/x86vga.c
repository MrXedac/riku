#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"

uint8_t x86vga_cursor_x = 0;
uint8_t x86vga_cursor_y = 0;

uint8_t backColour = 0;
uint8_t foreColour = 15;

uint8_t ansiColors[8] = {0x0, 0x4, 0x2, 0xE, 0x1, 0x5, 0x3, 0xF};

void x86vga_move_cursor()
{
	uint16_t cursorLocation = x86vga_cursor_y * 80 + x86vga_cursor_x;
	outb(0x3D4, 14);
	outb(0x3D5, cursorLocation >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, cursorLocation);
}

void x86vga_scroll(struct riku_devfs_node* self)
{

	uint8_t attributeByte = (1 /*blue*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attributeByte << 8);
  uint16_t* vmem = (uint16_t*)self->resources[0].begin;

	if(x86vga_cursor_y >= 25)
	{
		int i;
		for (i = 0*80; i < 24*80; i++)
		{
			vmem[i] = vmem[i+80];
		}
		for (i = 24*80; i < 25*80; i++)
		{
			vmem[i] = blank;
		}
		x86vga_cursor_y = 24;
	}
}

uint8_t ansiMode = 0;
char ansiBuffer[16];
int ansiIndex = 0;

void applyAnsi(char* buf)
{
	// printk("Got control sequence %s\n", buf);
	/* Foreground color control */
	if(buf[0] == '3')
	{
		/* FIXME: correct ANSI codes */
		foreColour = ansiColors[buf[1] - '0'];
	} else if(buf[0] == '4') /* Background color control */
	{
		backColour = ansiColors[buf[1] - '0'];
	} else if(buf[0] == '0') { /* Not implemented yet */
		backColour = 0;
		foreColour = 15;
	}
}

void parseAnsi()
{
	if(ansiBuffer[0] != '[')
	{
		/* Wrong format! */
		printk("Wrong ANSI control sequence format: %c\n", ansiBuffer[0]);
		ansiMode = 0;
		ansiIndex = 0;
		return;
	} else {
		/* The remaining stuff is the ANSI stuff */
		int curIndex = 1;
		int cseqBegin = 1;
		do {
			/* Separator, we must parse current ANSI code */
			if(ansiBuffer[curIndex] == ';')
			{
				ansiBuffer[curIndex] = '\0';
				applyAnsi(&(ansiBuffer[cseqBegin]));
				cseqBegin = curIndex + 1;
			}
			curIndex++;
		} while(curIndex < ansiIndex);

		/* End of ANSI sequence; parse current code as well */
		applyAnsi(&(ansiBuffer[cseqBegin]));
	}
}

int x86vga_putch(struct riku_devfs_node* self, char c)
{
	/* Parse ANSI mode */
	if(ansiMode)
	{
		/* ANSI end */
		if(c == 'm')
		{
			ansiMode = 0;
			parseAnsi();
			return 0;
		} else {
			/* Add ANSI control character to buffer */
			ansiBuffer[ansiIndex] = c;
			ansiIndex++;
			return 0;
		}
	}

	/* Enter ANSI mode is ANSI control sequence is written */
	if(c == 0x1B)
	{
		memset(ansiBuffer, 0x0, 16);
		ansiIndex = 0;
		ansiMode = 1;
		return 0;
	}

	uint8_t  attributeByte = (backColour << 4) | (foreColour & 0x0F);

	uint16_t attribute = attributeByte << 8;
	uint16_t *location;

	if (c == 0x08 && x86vga_cursor_x)
	{
		x86vga_cursor_x--;
	}

	else if (c == 0x09)
	{
		x86vga_cursor_x = (x86vga_cursor_x+8) & ~(8-1);
	}

	else if (c == '\r')
	{
		x86vga_cursor_x = 0;
	}

	else if (c == '\n')
	{
		x86vga_cursor_x = 0;
		x86vga_cursor_y++;
	}

	else if(c >= ' ')
	{
		location = (uint16_t*)self->resources[0].begin + (x86vga_cursor_y*80 + x86vga_cursor_x);
		*location = c | attribute;
		x86vga_cursor_x++;
	}

	if (x86vga_cursor_x >= 80)
	{
		x86vga_cursor_x = 0;
		x86vga_cursor_y ++;
	}

	x86vga_scroll(self);
	x86vga_move_cursor();
	return 0;
}

void x86vga_cls(struct riku_devfs_node* self)
{
	uint8_t attributeByte = (0 << 4) | (15 & 0x0F);
	uint16_t blank = 0x20 | (attributeByte << 8);
  uint16_t* vmem = (uint16_t*)self->resources[0].begin;

	int i;
	for (i = 0; i < 80*25; i++)
	{
		vmem[i] = blank;
	}

	x86vga_cursor_x = 0;
	x86vga_cursor_y = 0;
	x86vga_move_cursor();
}

int x86vga_puts(struct riku_devfs_node* self, const char *c, uint32_t count)
{
	int i = 0;
	while (i < count)
	{
		x86vga_putch(self,c[i]);
    i++;
	}
  return 0;
}

void x86vga_puthex(struct riku_devfs_node* self, uintptr_t n)
{
	int tmp;

	x86vga_puts(self,"0x",2);

	char noZeroes = 1;

	int i;
	for (i = 60; i > 0; i -= 4)
	{
		tmp = (n >> i) & 0xF;
		if (tmp == 0 && noZeroes != 0)
		{
			continue;
		}

		if (tmp >= 0xA)
		{
			noZeroes = 0;
			x86vga_putch(self,tmp-0xA+'A' );
		}
		else
		{
			noZeroes = 0;
			x86vga_putch(self, tmp+'0' );
		}
	}

	tmp = n & 0xF;
	if (tmp >= 0xA)
	{
		x86vga_putch(self,tmp-0xA+'A');
	}
	else
	{
		x86vga_putch(self,tmp+'0');
	}

}

void x86vga_putdec(struct riku_devfs_node* self, uintptr_t n)
{

	if (n == 0)
	{
		x86vga_putch(self,'0');
		return;
	}

	int acc = n;
	char c[64];
	int i = 0;
	while (acc > 0)
	{
		c[i] = '0' + acc%10;
		acc /= 10;
		i++;
	}
	c[i] = 0;

	char c2[64];
	c2[i--] = 0;
	int j = 0;
	while(i >= 0)
	{
		c2[i--] = c[j++];
	}
	x86vga_puts(self,c2,strlen(c2));
}

void x86vga_init()
{
  /* Populate devfs:/vga0 */
  struct riku_devfs_node* vgat = hardware_create_node("vga0");
  if(vgat)
  {
    hardware_add_resource(vgat, MMIO, 0xFFFF8000000B8000, 0x4000);
    devfs_add(vgat);
    vgat->write = &x86vga_puts;
    vgat->putch = &x86vga_putch;

    if(!console)
    {
      console = vgat;

      /* Initialize console properly */
      /* Wipe existing stuff on screen */
      x86vga_cls(console);
      console->write(console, "Registered devfs:/vga0 as console\n", 34);
    }
  }
}
