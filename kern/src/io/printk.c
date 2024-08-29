#include "printk.h"
#include "vfs/devfs.h"
#include "mem.h"
#include <string.h>
#include <stdarg.h>

static char* bf;
static char buf[12];
static uint64_t num;
static char uc;
static char zs;

uint8_t printk_enabled = 0;

extern uint64_t jiffies;

void printk_dec(uint64_t n)
{
  if (n == 0)
  {
    kconsole->putch(kconsole,'0');
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
  kconsole->write(kconsole, c2, strlen(c2));
}

void printk_hex(uint64_t n)
{
  int tmp;

	kconsole->write(kconsole, "0x", 2);

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
			kconsole->putch(kconsole,tmp-0xA+'A' );
		}
		else
		{
			noZeroes = 0;
			kconsole->putch(kconsole, tmp+'0' );
		}
	}

	tmp = n & 0xF;
	if (tmp >= 0xA)
	{
		kconsole->putch(kconsole,tmp-0xA+'A');
	}
	else
	{
		kconsole->putch(kconsole,tmp+'0');
	}
}

/* Full credits again to https://github.com/rlangoy/ZedBoard-BareMetal-Examples/blob/master/Hello05/printf.c */
void printk_internal(char *fmt, ...)
{
  /* First check whether we have a kernel console or not */
  if(!kconsole || !printk_enabled)
    return;

  /* Perform printk on kconsole node */
  va_list va;
	char ch;
	char* p;

  kconsole->putch(kconsole, '[');
  printk_dec(jiffies / PRINTK_JIFFIES_DIVISOR);
  kconsole->putch(kconsole, '.');
  printk_dec(jiffies % PRINTK_JIFFIES_DIVISOR);
  kconsole->write(kconsole, "] ", 2);

	va_start(va,fmt);

	while ((ch=*(fmt++))) {
		if (ch!='%') {
			kconsole->putch(kconsole,ch);
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
						kconsole->putch(kconsole,'-');
					}
					printk_dec(num);
					break;
				case 'x':
				case 'X' :
					uc= ch=='X';
					num=va_arg(va, uint64_t);
					printk_hex(num);
					break;
				case 'c' :
					kconsole->putch(kconsole,(char)(va_arg(va, int)));
					break;
				case 's' :
					p=va_arg(va, char*);
					break;
				case '%' :
					kconsole->putch(kconsole,'%');
				default:
					break;
			}
			*bf=0;
			bf=p;
			while (*bf++ && w > 0)
				w--;
			while (w-- > 0)
				kconsole->putch(kconsole,lz ? '0' : ' ');
			while ((ch= *p++))
				kconsole->putch(kconsole,ch);
		}
	}
abort:;
	va_end(va);
}
