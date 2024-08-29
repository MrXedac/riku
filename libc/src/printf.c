#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include <stdarg.h>

static char* bf;
static char buf[12];
static uint64_t num;
static char uc;
static char zs;

void printf_dec(uint64_t n)
{
  if (n == 0)
  {
    putc('0', stdout);
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
  write(stdout->fd, c2, strlen(c2));
}

void printf_hex(uint64_t n)
{
  int tmp;

	write(stdout->fd, "0x", 2);

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
			putc(tmp-0xA+'A' , stdout);
		}
		else
		{
			noZeroes = 0;
			putc(tmp+'0', stdout );
		}
	}

	tmp = n & 0xF;
	if (tmp >= 0xA)
	{
		putc(tmp-0xA+'A', stdout);
	}
	else
	{
		putc(tmp+'0', stdout);
	}
}


int printf ( const char * fmt, ... )
{
  va_list va;
	char ch;
	char* p;

	va_start(va,fmt);

	while ((ch=*(fmt++))) {
		if (ch!='%') {
			putc(ch, stdout);
		}
		else {
			char lz=0;
			char w=0;
			ch=*(fmt++);
			if (ch=='0') {
				ch=*(fmt++);
				lz=1;
			}
			if (ch>='0' && ch<='9') {
				w=0;
				while (ch>='0' && ch<='9') {
					w=(((w<<2)+w)<<1)+ch-'0';
					ch=*fmt++;
				}
			}
			bf=buf;
			p=bf;
			zs=0;
			switch (ch) {
				case 0:
					goto abort;
				case 'u':
				case 'd' :
					num=va_arg(va, uint64_t);
					if (ch=='d' && (int)num<0) {
						num = -(int)num;
						putc('-', stdout);
					}
					printf_dec(num);
					break;
				case 'x':
				case 'X' :
					uc= ch=='X';
					num=va_arg(va, uint64_t);
					printf_hex(num);
					break;
				case 'c' :
					putc((char)(va_arg(va, int)), stdout);
					break;
				case 's' :
					p=va_arg(va, char*);
					break;
				case '%' :
					putc('%', stdout);
				default:
					break;
			}
			*bf=0;
			bf=p;
			while (*bf++ && w > 0)
				w--;
			while (w-- > 0)
				putc(lz ? '0' : ' ', stdout);
			while ((ch= *p++))
				putc(ch, stdout);
		}
	}
abort:;
	va_end(va);
  return 0; /* TODO : bad */
}
