/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции ввода/вывода, в том числе реализации стандартных
 *  функций
 *
 */


#include <helloos/types.h>
#include <helloos/io.h>
#include <helloos/scrio.h>
#include <string.h>
#include <stdarg.h>
#include <helloos/syscall.h>



char *vidmem = (char *)0xb8000; // адрес видеопамати
int vidport;                    // видеопорт
int lines, cols;                // количество линий и строк на экране
int curr_x,curr_y;              // текущее положение курсора



// Иницилизация экранного вывода
// Передаются начальные координаты курсора
void scrio_init(int cur_x, int cur_y)
{
   vidmem = (char*) 0xb8000;
   vidport = 0x3d4;
   lines = 25;
   cols = 80;

   curr_x = cur_x;
   curr_y = cur_y;

//   if (curr_y == 24)
//   {
      int i;
      for (i = 0; i < 24*80*2; i++)
         vidmem[i] = vidmem[i+160];
      for (i = 0; i < 160; i+=2)
      {
         vidmem[24*80*2+i  ] = 0x20;
         vidmem[24*80*2+i+1] = 0x0e;
      }
      curr_y--;
//   }
   lines = 24;
}



//Функция перевода курсора в положение (x,y)
void gotoxy(int x, int y)
{
   int pos;
   pos = x + cols * y;
   outb_p(14, vidport);
   outb_p((pos >> 8) & 0xff, vidport+1);
   outb_p(15, vidport);
   outb_p(pos & 0xff, vidport+1);
   curr_x = x;
   curr_y = y;
}


//Функция прокручивания экрана.
//Работает, используя прямую запись в видеопамять
void scroll()
{
   __asm__ ("movl $0xb8000, %%edi\n"
            "movl $0xb80a0, %%esi\n"
            "movl $920, %%ecx\n"
            "cld\n"
            "rep movsl\n"
            "movl $0x0e200e20, %%eax\n"
            "movl $0xb8e60, %%edi\n"
            "movl $40, %%ecx\n"
            "rep stosl\n"
            :::"di","si");
}



// Вывод строки с заданным аттрибутом и ограниченной длиной
// Поддерживаются следующие метасимволы:
//    \t - табуляция, выравнивание по границе 8
//    \001\0xx - смена цвета символа на 0xx-010
//    \002\0xx - смена цвета фона на 0xx-010
//    \003     - смена цветов на стандартные (attr)
void nputs_color(const char *s, uint n, uchar attr)
{
   int x,y;
   char c;
   uchar cur_attr = attr;
   x = curr_x;
   y = curr_y;
   while ((c = *s++) != '\0'  &&  n--)
   {
      if (c == '\n')
      {
         x = 0;
         if (++y >= lines)
         {
            scroll();
            y--;
         }
      }
      else if (c == '\t')
      {
         int t = 8 - (x % 8);
         gotoxy(x,y);
         nputs_color("        ", t, cur_attr);
         x=curr_x;y=curr_y;
      }
      else if (c == '\001')
      {
         c = *s;
         if (c >= '\010')
         {
            cur_attr = (cur_attr & 0xf0) | ((c - '\010') & 0xf);
            s++;
         }
      }
      else if (c == '\002')
      {
         c = *s;
         if (c >= '\010')
         {
            cur_attr = (cur_attr & 0xf) | ((c - '\010') << 4);
            s++;
         }
      }
      else if (c == '\003')
      {
         cur_attr = attr;
      }
      else
      {
         vidmem [(x + cols * y) * 2] = c;
         vidmem [(x + cols * y) * 2 + 1] = cur_attr;
         if (++x >= cols)
         {
            x = 0;
            if (++y >= lines)
            {
               scroll();
               y--;
            }
         }
      }
   }
   gotoxy(x,y);
}


// Вывод с цветом без ограничения длины
void puts_color(const char *s, uchar attr)
{
   nputs_color(s, -1, attr);
}


// Вывод без цвета без длины
void puts(const char *s)
{
   nputs_color(s, -1, 0x0e);
}


// Вывод без цвета, с ограничением длины
void nputs(const char *s, uint n)
{
   nputs_color(s, n, 0x0e);
}


// Очистка экрана
void clear_screen()
{
   int i;

   for (i = 0; i < lines*cols*2; i+=2)
   {
      vidmem[i] = 0x20;
      vidmem[i+1] = 0x0e;
   }

   gotoxy(0, 0);
}


// Вывод байтов в hex-виде. Size - размер данных в байтах.
void PrintHex(void *val, uchar size)
{
   static unsigned char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
   char s[] = {0, 0, 0};
   unsigned char i;
   for (i = 0; i < size; i++)
      val++;

   while (size)
   {
      val--;
      s[0] = digits[*(uchar*)(val) >> 4];
      s[1] = digits[*(uchar*)(val) & 0xf];
      puts(s);
      size--;
   }
}

#include <lib/stdio.inc>

// Тупая реализация стандартного vprintf с поддержкой цвета
// Используется vsnprintf, а затем puts
// Может выводить НЕ БОЛЕЕ 256(-1) символов
// Цвет просто передается в puts_color
int vprintf_color(uchar attr, char *format, va_list curp)
{
   char Buffer[256];
   uint res = vsnprintf(Buffer, 256, format, curp);
   puts_color(Buffer, attr);
   return res;
}

// Реализация стандартного vprintf через vprintf_color
int vprintf(char *format, va_list curp)
{
   return vprintf_color(0x0e, format, curp);
}

// Цветной printf через vprintf_color
int printf_color(uchar attr, char *format, ...)
{
   va_list curp;
   va_start(curp, format);
   uint res = vprintf_color(attr, format, curp);
   va_end(curp);
   return res;
}

// Реализация стандартного printf через vprintf_color
int printf(char *format, ...)
{
   va_list curp;
   va_start(curp, format);
   uint res = vprintf_color(0x0e, format, curp);
   va_end(curp);
   return res;
}






char scanmap[] = 
{
   '\0', // 0x00
   '\0', // 0x01
   '1' , // 0x02
   '2' , // 0x03
   '3' , // 0x04
   '4' , // 0x05
   '5' , // 0x06
   '6' , // 0x07
   '7' , // 0x08
   '8' , // 0x09
   '9' , // 0x0a
   '0' , // 0x0b
   '\0', // 0x0c
   '\0', // 0x0d
   0x08, // 0x0e
   '\0', // 0x0f
   'q' , // 0x10
   'w' , // 0x11
   'e' , // 0x12
   'r' , // 0x13
   't' , // 0x14
   'y' , // 0x15
   'u' , // 0x16
   'i' , // 0x17
   'o' , // 0x18
   'p' , // 0x19
   '[' , // 0x1a
   ']' , // 0x1b
   0x0d, // 0x1c
   '\0', // 0x1d
   'a' , // 0x1e
   's' , // 0x1f
   'd' , // 0x20
   'f' , // 0x21
   'g' , // 0x22
   'h' , // 0x23
   'j' , // 0x24
   'k' , // 0x25
   'l' , // 0x26
   ';' , // 0x27
   '\'', // 0x28
   '`' , // 0x29
   '\0', // 0x2a
   '\0', // 0x2b
   'z', // 0x2c
   'x', // 0x2d
   'c', // 0x2e
   'v', // 0x2f
   'b', // 0x30
   'n', // 0x31
   'm', // 0x32
   ',', // 0x33
   '.', // 0x34
   '/', // 0x35
   '\0', // 0x36
   '\0', // 0x37
   '\0', // 0x38
   ' ', // 0x39
   '\0', // 0x3a
   '\0', // 0x3b
   '\0', // 0x3c
   '\0', // 0x3d
   '\0', // 0x3e
   '\0', // 0x3f
   '\0', // 0x40
   '\0', // 0x41
   '\0', // 0x42
   '\0', // 0x43
   '\0', // 0x44
   '\0', // 0x45
   '\0', // 0x46
   '\0', // 0x47
   '\0', // 0x48
   '\0', // 0x49
   '\0', // 0x4a
   '\0', // 0x4b
   '\0', // 0x4c
   '\0', // 0x4d
   '\0', // 0x4e
   '\0', // 0x4f
   '\0', // 0x50
   '\0', // 0x51
   '\0', // 0x52
   '\0', // 0x53
   '\0', // 0x54
   '\0', // 0x55
   '\0', // 0x56
   '\0', // 0x57
   '\0', // 0x58
   '\0', // 0x59
   '\0', // 0x5a
   '\0', // 0x5b
   '\0', // 0x5c
   '\0', // 0x5d
   '\0', // 0x5e
   '\0', // 0x5f
   '\0', // 0x60
   '\0', // 0x61
   '\0', // 0x62
   '\0', // 0x63
   '\0', // 0x64
   '\0', // 0x65
   '\0', // 0x66
   '\0', // 0x67
   '\0', // 0x68
   '\0', // 0x69
   '\0', // 0x6a
   '\0', // 0x6b
   '\0', // 0x6c
   '\0', // 0x6d
   '\0', // 0x6e
   '\0', // 0x6f
   '\0', // 0x70
   '\0', // 0x71
   '\0', // 0x72
   '\0', // 0x73
   '\0', // 0x74
   '\0', // 0x75
   '\0', // 0x76
   '\0', // 0x77
   '\0', // 0x78
   '\0', // 0x79
   '\0', // 0x7a
   '\0', // 0x7b
   '\0', // 0x7c
   '\0', // 0x7d
   '\0', // 0x7e
   '\0', // 0x7f
};

char scan2ascii(byte scancode)
{
   if (scancode > sizeof(scanmap))
      return '\0';
   return scanmap[scancode];
}


void readline(char *cmd, uint buf_size)
{
   uint cmdlen = 0;
   char c1, c2, stop = 0;
   int nx, ny;

   c2 = inb(0x60);

   while (! stop)
   {
      if (cmdlen > buf_size - 1) cmdlen = buf_size - 1;
      c1 = inb(0x60);
      if (c1 != c2)
      {
         c2 = c1;
         c1 = scan2ascii(c1);
         switch (c1)
         {
            case 0x08:
               if (cmdlen > 0)
               {
                  cmdlen--;
                  nx = curr_x - 1; ny = curr_y;
                  if (nx < 0) { nx = cols - 1; ny--; }
                  gotoxy(nx, ny); puts(" "); gotoxy(nx, ny);
                  curr_x = nx; curr_y = ny;
               }
               break;
            case 0x0d:
               puts("\n");
               cmd[cmdlen] = 0;
               stop = 1;
               break;
            case '\0': break;
            default:
               cmd[cmdlen++] = c1; nputs(&c1, 1);
               break;
         }
      }
   }
}


char syscall_getchar(uint x, uint y)
{
   return vidmem[y*cols*2+x*2];
}

uint syscall_setchar(uint x, uint y, char c)
{
   vidmem[y*cols*2+x*2] = c;
   return 0;
}

uint syscall_setattr(uint x, uint y, byte a)
{
   vidmem[y*cols*2+x*2+1] = a;
   return 0;
}

uint syscall_screen_info(uint *w, uint *h)
{
   memcpy_to_user(w, &cols, sizeof(cols));
   memcpy_to_user(h, &lines,sizeof(lines));
   return 0;
}
