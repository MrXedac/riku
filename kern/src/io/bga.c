#include "bga.h"
#include "bgafont.h"
#include "ioport.h"

extern const struct bitmap_font bgafont;

uint32_t bgacx = 0;
uint32_t bgacy = 1;

void BgaWriteRegister(unsigned short IndexValue, unsigned short DataValue)
{
	outw(VBE_DISPI_IOPORT_INDEX, IndexValue);
	outw(VBE_DISPI_IOPORT_DATA, DataValue);
}

unsigned short BgaReadRegister(unsigned short IndexValue)
{
	outw(VBE_DISPI_IOPORT_INDEX, IndexValue);
	return inw(VBE_DISPI_IOPORT_DATA);
}

int BgaIsAvailable(void)
{
	return (BgaReadRegister(VBE_DISPI_INDEX_ID) == VBE_DISPI_ID4);
}

void BgaSetVideoMode(unsigned int Width, unsigned int Height, unsigned int BitDepth, int UseLinearFrameBuffer, int ClearVideoMemory)
{
	BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
	BgaWriteRegister(VBE_DISPI_INDEX_XRES, Width);
	BgaWriteRegister(VBE_DISPI_INDEX_YRES, Height);
	BgaWriteRegister(VBE_DISPI_INDEX_BPP, BitDepth);
	BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED |
					 (UseLinearFrameBuffer ? VBE_DISPI_LFB_ENABLED : 0) |
					 (ClearVideoMemory ? 0 : VBE_DISPI_NOCLEARMEM));
	
	bga_mode = 1;
}

void BgaSetBank(unsigned short BankNumber)
{
	BgaWriteRegister(VBE_DISPI_INDEX_BANK, BankNumber);
}

uint32_t color2int(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint32_t res = (b & 0x000000FF) | ((g << 8) & 0x0000FF00) | ((r << 16) & 0x00FF0000) | ((a << 24) & 0xFF000000);
	return res;
}

void putpixel(uint32_t color, uint32_t x, uint32_t y)
{
	uint32_t* lfb = (uint32_t*)(VBE_LFB + y * BGA_WIDTH * sizeof(uint32_t) + x * sizeof(uint32_t));
	*lfb = color;
}

void bgaputch_internal_transp(unsigned char c, int x, int y, int fgcolor)
{
	int cx,cy;
	int mask[8]={1,2,4,8,16,32,64,128};
	const unsigned char *glyph=bgafont.Bitmap+ c * 16 * sizeof(uint8_t) - 31 * 16;
 
	for(cy=0;cy<16;cy++){
		for(cx=7;cx>=0;cx--){
			if(glyph[cy]&mask[cx]) putpixel(fgcolor,(x * 8)+(8 - cx),(y * 16)+cy-12);
		}
	}
}

void bgaputch_internal(unsigned char c, int x, int y, int fgcolor, int bgcolor)
{
	int cx,cy;
	int mask[8]={1,2,4,8,16,32,64,128};
	const unsigned char *glyph=bgafont.Bitmap+ c * 16 * sizeof(uint8_t) - 31 * 16;
 
	for(cy=0;cy<16;cy++){
		for(cx=7;cx>=0;cx--){
			putpixel(glyph[cy]&mask[cx]?fgcolor:bgcolor,(x * 8)+(8 - cx - 1),(y * 16)+cy-16);
		}
	}
}

void bgascroll()
{
	if(bgacy >= BGA_CYMAX)
	{
		uint32_t* lfb = (uint32_t*)(VBE_LFB);
		int i;
		for (i = 0; i < BGA_WIDTH * (BGA_HEIGHT - BGA_CYMAX) * sizeof(uint32_t); i++)
		{
			*(uint32_t*)(VBE_LFB + i * sizeof(uint32_t)) = *(uint32_t*)(VBE_LFB + i * sizeof(uint32_t) + BGA_WIDTH * 16 * sizeof(uint32_t));
		}
		for (i = BGA_WIDTH * (BGA_HEIGHT - BGA_CYMAX) * sizeof(uint32_t); i < BGA_WIDTH * BGA_HEIGHT * sizeof(uint32_t); i++)
		{
			*(uint32_t*)(VBE_LFB + i * sizeof(uint32_t)) = 0xFFFFFFFF;
		}
		bgacy = BGA_CYMAX - 1;
	}
}

void invert()
{
	uint32_t i, j;
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 16; j++)
		{
			uint32_t col =*(uint32_t*)(VBE_LFB + /* Y */ BGA_WIDTH * ((bgacy - 1) * 16 + j) * sizeof(uint32_t) + /* X */ ((bgacx * 8) + i) * sizeof(uint32_t));
			if(col)
				*(uint32_t*)(VBE_LFB + /* Y */ BGA_WIDTH * ((bgacy - 1) * 16 + j) * sizeof(uint32_t) + /* X */ ((bgacx * 8) + i) * sizeof(uint32_t)) = 0x00000000;
			else
				*(uint32_t*)(VBE_LFB + /* Y */ BGA_WIDTH * ((bgacy - 1) * 16 + j) * sizeof(uint32_t) + /* X */ ((bgacx * 8) + i) * sizeof(uint32_t)) = 0xFFFFFFFF;

		}
	}
}

int bgaputch(char c)
{
	if (c == 0x08 && bgacx)
	{
		invert();
		bgacx--;
		invert();
	}
	
	else if (c == 0x09)
	{
		invert();
		bgacx = (bgacx+8) & ~(8-1);
		invert();
	}
	
	else if (c == '\r')
	{
		invert();
		bgacx = 0;
		invert();
	}
	
	else if (c == '\n')
	{
		invert();
		bgacx = 0;
		bgacy++;
		invert();
	}
	
	else if(c >= ' ')
	{
		invert();
		bgaputch_internal(c, bgacx, bgacy, color2int(255, 255, 255, 0), color2int(0, 0, 0, 0));
		bgacx++;
		invert();
	}
	
	if (bgacx >= 128)
	{
		invert();
		bgacx = 0;
		bgacy ++;
		invert();
	}
	
	bgascroll();
	//move_cursor();
	return 0;
}

void bgaputs(char* str)
{
	int i = 0;
	while (str[i])
	{
		bgaputch(str[i++]);
	}
}
