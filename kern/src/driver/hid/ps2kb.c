#include <stdint.h>
#include "iotypes.h"
#include "serial.h"
#include "vfs/devfs.h"
#include "mem.h"
#include "heap.h"
#include "serial.h"
#include "ioport.h"
#include "printk.h"
#include "idt64.h"
#include "stream.h"
#include "vfs/readwrite.h"

typedef enum ps2kb_state { PS2KB_STATE_NORMAL, PS2KB_STATE_SHIFTED, PS2KB_STATE_WHAT } ps2kb_state_t;

ps2kb_state_t ps2kb_current_state = PS2KB_STATE_NORMAL;

unsigned char default_map[128] =
{
    0,  27, '&', /* 'é' */ 'e', '"', '\'', '(', '-', /* 'è' */ 'e', '_',	/* 9 */
  /* 'ç' */ 'c', /* 'à' */ 'a', ')', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'a', 'z', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',		/* Enter key */
    0,			/* 29   - Control */
  'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '<', 'w', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  ',', ';', ':', '!',   0,					/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char default_map_shift[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', /* '°' */ '.', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'A', 'Z', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '"', /* '£' */ '.', '\n',		/* Enter key */
    0,			/* 29   - Control */
  'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '>', 'W', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  '?', '.', '/', /*'§'*/ 's',   0,					/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58

#define KEY_ESC 0x01
#define KEY_ENTER 0x1C
#define KEY_SPACE 0x39
#define KEY_TAB 0x0F
#define KEY_BACKSPACE 0x0E
#define KEY_CAPSLOCK 0x3A
#define KEY_ESCAPE 0x01

#define KEY_UP 0x48 
#define KEY_DOWN 0x50
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D

#define KEY_LEFT_SHIFT 0x2A
#define KEY_LEFT_CTRL 0x1D
#define KEY_RIGHT_SHIFT 0x36
#define KEY_NUMBER_LOCK 0x45
#define KEY_SCROLL_LOCK 0x46

#define KEY_KEYPAD_1 0x4F
#define KEY_KEYPAD_2 0x50
#define KEY_KEYPAD_3 0x51
#define KEY_KEYPAD_4 0x4B
#define KEY_KEYPAD_5 0x4C
#define KEY_KEYPAD_6 0x4D
#define KEY_KEYPAD_7 0x47
#define KEY_KEYPAD_8 0x48
#define KEY_KEYPAD_9 0x49
#define KEY_KEYPAD_0 0x52
#define KEY_KEYPAD_PLUS 0x4E
#define KEY_KEYPAD_MINUS 0x4A

riku_stream_t* ps2kb_stream;

/* Handles the keyboard interrupt */
void ps2kb_handler(registers_t *regs)
{
    __asm volatile("cli");
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    scancode = inb(0x60);

    /* If the top bit of the byte we read from the keyboard is
    *  set, that means that a key has just been released */
    if (scancode & 0x80)
    {
        /* Shift released ? */
        if(ps2kb_current_state == PS2KB_STATE_SHIFTED && ((scancode & 0x7F) == (KEY_LEFT_SHIFT & 0x7F) || (scancode & 0x7F) == (KEY_RIGHT_SHIFT & 0x7F))) {
          outb(0x60, 0xED);
          outb(0x60, 0x0);
          ps2kb_current_state = PS2KB_STATE_NORMAL;
        }
    }
    else
    {
        /* Shift ? */
        if((scancode == KEY_LEFT_SHIFT || scancode == KEY_RIGHT_SHIFT) && (ps2kb_current_state == PS2KB_STATE_NORMAL)) {
            while(1) {
                if ((inb(0x64) & 2) == 0) break;
            }
            // Set the keyboard lid
            outb(0x60, 0xED);
            outb(0x60, 0x4);
            ps2kb_current_state = PS2KB_STATE_SHIFTED;
            return;
        }

        char c;

        if(ps2kb_current_state == PS2KB_STATE_SHIFTED)
            c = default_map_shift[scancode];
        else
            c = default_map[scancode];

        /* Add the read character to the ps2kb IO stream */
        //printk("read char: %c\n", c);
        stream_write(ps2kb_stream, c);
    }
    __asm volatile("sti");
}

int ps2kb_getch(struct riku_devfs_node* self, char* c)
{
  while(ps2kb_stream->state == STREAM_EMPTY)
  {
    /* Wait until we have some data in the keyboard buffer */
    wake_on_irq(1);
  }

  char buf_char = stream_read(ps2kb_stream, false);
  *c = buf_char;
  return 0;
}

int ps2kb_read(struct riku_devfs_node* self, const char* buffer, uint32_t count)
{
  int i;
  char* thebuf = (char*)buffer;

  /* Oh damn */
  for(i = 0; i < count; i++) {
   ps2kb_getch(self, (thebuf + i));
   
   /* Sync with stdout */
   if(*(thebuf+i) == '\b')
   {
    write(1, "\b", 1);
    write(1, " ", 1);
    write(1, "\b", 1);
   } else write(1, (thebuf+i), 1);

    /* Break input on newline */
   if(*(thebuf+i) == '\n') 
   {
    break;
   }
  }

   return i+1; /* include terminating byte */
}

/* Probes x86 PS/2 legacy keyboard driver, and adds corresponding devfs entry if required */
void ps2kb_init()
{
  /* Allocate node */
  struct riku_devfs_node* kb_node = hardware_create_node("kb0");
  if(kb_node)
  {
      hardware_add_resource(kb_node, PORTIO, 0x3F8, 0x3F8 + 5);
      kb_node->getch = ps2kb_getch;
      kb_node->putch = 0;
      kb_node->read = ps2kb_read;
      kb_node->write = 0;
      kb_node->type = HIDDevice;
      devfs_add(kb_node);

      /* Registers the keyboard interrupt handler and prepares the stream */
      ps2kb_stream = (riku_stream_t*)kalloc(sizeof(riku_stream_t));
      stream_init(ps2kb_stream);
      printk("ps2kb: stream initialized\n");

      register_irq(1, ps2kb_handler);
      /* unsigned char scancode;

      scancode = inb(0x60); */

      if(!kinput)
      {
        kinput = kb_node;
        printk("Registered devfs:/kb0 as default input device\n");
      } else kinput = devfs_find_node("null");
  }
  return;
}
