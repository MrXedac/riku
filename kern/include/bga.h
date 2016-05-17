#ifndef __BGA_H__
#define __BGA_H__

#include <stdint.h>

#define VBE_DISPI_IOPORT_INDEX		0x01CE
#define VBE_DISPI_IOPORT_DATA		0x01CF
#define VBE_DISPI_INDEX_ID			0
#define VBE_DISPI_INDEX_XRES		1
#define VBE_DISPI_INDEX_YRES		2
#define VBE_DISPI_INDEX_BPP			3
#define VBE_DISPI_INDEX_ENABLE		4
#define VBE_DISPI_INDEX_BANK		5
#define VBE_DISPI_INDEX_VIRT_WIDTH	6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET	8
#define VBE_DISPI_INDEX_Y_OFFSET	9
#define VBE_DISPI_ID0				0xB0C0
#define VBE_DISPI_ID1				0xB0C1
#define VBE_DISPI_ID2				0xB0C2
#define VBE_DISPI_ID3				0xB0C3
#define VBE_DISPI_ID4				0xB0C4
#define VBE_DISPI_ID5				0xB0C5
#define VBE_DISPI_DISABLED			0x0
#define VBE_DISPI_ENABLED			0x1
#define VBE_DISPI_LFB_ENABLED		0x40
#define VBE_DISPI_NOCLEARMEM		0x80

#define VBE_LFB						0xFD000000

void BgaWriteRegister(unsigned short IndexValue, unsigned short DataValue);

unsigned short BgaReadRegister(unsigned short IndexValue);

int BgaIsAvailable(void);

void BgaSetVideoMode(unsigned int Width, unsigned int Height, unsigned int BitDepth, int UseLinearFrameBuffer, int ClearVideoMemory);

void BgaSetBank(unsigned short BankNumber);

uint32_t color2int(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

int bgaputch(char c);
void bgaputs(char* str);

uint8_t bga_mode;

#endif