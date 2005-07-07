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


#include "types.h"
#include "io.h"
#include "hello_stdio.h"
#include "hello_string.h"
#include "hello_stdarg.h"



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

   if (curr_y == 24)
   {
      scroll();
      curr_y--;
   }
   lines = 24;
}



//Функция перевода курсора в положение (x,y)
void gotoxy(int x, int y)
{
   int pos;
   pos = (x + cols * y) * 2;
   outb_p(14, vidport);
   outb_p(0xff & (pos >> 9), vidport+1);
   outb_p(15, vidport);
   outb_p(0xff & (pos >> 1), vidport+1);
   curr_x = x;
   curr_y = y;
}


//Функция прокручивания экрана.
//Работает, используя прямую запись в видеопамять
void scroll()
{
   int i;
   memcpy(vidmem, vidmem + cols * 2, (lines - 1) * cols * 2);
   for (i = (lines - 1) * cols * 2; i < lines * cols * 2; i += 2)
      vidmem[i] = ' ';
}



// Вывод строки с заданным аттрибутом и ограниченной длиной
void nputs_color(const char *s, uint n, uchar attr)
{
   int x,y;
   char c;
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
      else
      {
         vidmem [(x + cols * y) * 2] = c;
         vidmem [(x + cols * y) * 2 + 1] = attr;
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


// Реализация стандартного snprintf
// Сейчас поддерживаются форматы: c, d, p, s, u, x
// Ширина поля и точность НЕ поддерживаются
int vsnprintf(char *str, size_t maxlen, const char *format, va_list curp)
{
   /*Проверка на конец буффера*/
#define ifend {if(strpos>=(maxlen-1)){str[maxlen-1]='\0';return strpos;}}

   char c;              // тут,
   char *pc;            // я
   //double d;            // думаю,
   int i, i1;               // всё
   unsigned int ui, ui1;        // понятно
   
   int num;          // понадобятся для подсчёта
   unsigned int fl;        // количества цифр в числе
   int j;               // счётчик. юзается часто.
   int hexnum[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
   
   unsigned int count=0;            // текущая позиция чтения в format
   unsigned int strpos=0;           // текущая позиция записи в str  и одновременно количество уже записанных символов
   
//   va_list curp;           // читай Кернигана
//   va_start(curp,format);        // инициализируем
   

   /* Цикл пошёл */
   while(format[count]!='\0' && strpos<(maxlen-1))
   {
/************************************************************************/
/**/     if(format[count]!='%')
      {
         ifend;
         str[strpos]=format[count];    // иначе пишем символ и
         count++;          // увеличиваем
         strpos++;            // счётчики
         continue;
      }
      switch(format[count+1])
      {
/************************************************************************/
/**/        case '%':
            ifend;
            str[strpos]='%';        // пишем '%'
            count+=2;            // прыгаем на следующий за ним символ
            strpos++;            // увеличиваем на 1 позицию записи
            break;
/************************************************************************/
/**/        case 'c':
            ifend;
            c=va_arg(curp, char);         // читаем его
            str[strpos]=c;          // пишем
            strpos++;            // увеличиваем на 1 позицию записи
            count+=2;            // прыгаем на следующий за ним символ
            break;
/************************************************************************/
/**/        case 'd':
            ifend;
            i=va_arg(curp, int);
            if(i<0)
            {
               str[strpos]='-';
               strpos++;
               i=-i;
            }
            num=1;
            fl=1;
//            while(i>=(fl*=10)) num++;     // считаем количество цифр
//            fl/=10;              // в переменной fl 10 в степени num

            i1 = i;
            while (i1/10)
            { i1/=10; fl*=10; num++; }

            j=1;
            /*пока есть куда писать и что писать*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(i/fl)+'0';    // переводим int в char и пишем в str
               i%=fl;            // избавляемся от считанной цифры
               fl/=10;           // понижаем степень 10
               j++;
               strpos++;
            }
            count+=2;
            break;
/************************************************************************/
/*       case 'f':
            ifend;
            d=va_arg(curp, double);
*/
/************************************************************************/
         case 'p':
            ifend;
            ui=va_arg(curp, unsigned int);
            j=28;
            while(strpos<=(maxlen-2) && j>=0)            // не 0 :(
            {
               str[strpos]=hexnum[(ui>>j)&0xf];
               strpos++;
               j-=4;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 's':
            ifend;
            pc=va_arg(curp, char *);
            j=0;
            /*копируем pc[] в str[]*/
            while(strpos<=(maxlen-2) && pc[j]!=0)
            {
               str[strpos]=pc[j];
               strpos++;
               j++;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 'u':
            ifend;
            ui=va_arg(curp, unsigned int);
            num=1;
            fl=1;
//            while(ui>=(fl*=10)) num++;    
//            fl/=10;              

            ui1 = ui;
            while (ui1/10)                // считаем количество цифр
            { ui1/=10; fl*=10; num++; }   // в переменной fl 10 в степени num-1

            j=1;
            /*пока есть куда писать и что писать*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(ui/fl)+'0';   // переводим int в char и пишем в str
               ui%=fl;           // избавляемся от считанной цифры
               fl/=10;           // понижаем степень 10
               j++;
               strpos++;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 'x':
            ifend;
            ui=va_arg(curp, unsigned int);
            if(ui==0)
            {
               str[strpos]='0';        // если 0, то просто пишем его
               count+=2;
               strpos++;
               break;
            }
            else
            {
               j=28;
               while(strpos<=(maxlen-2) && j>=0)   // не 0 :(
               {
                  if((ui>>j)&0xf)      // цифру делаем младшей и отделяем её
                  {
                     str[strpos]=hexnum[(ui>>j)&0xf];
                     strpos++;
                  }
                  j-=4;
               }
               count+=2;
               break;
            }
/************************************************************************/
         default:
            break;
      }
   }
   ifend;
   str[strpos]='\0';
   return strpos;
}

int snprintf(char *str, size_t maxlen, const char *format, ...)
{
   va_list curp;
   va_start(curp, format);
   uint res = vsnprintf(str, maxlen, format, curp);
   va_end(curp);
   return res;
}



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






// Считать один символ с клавиатуры
// Сейчас это реализовано крайне тупо: просто ждем пока
// в порту клавиатуры не сменится значение.
// Когда-нибудь здесь будет нормальная таблица скан-кодов,
// буфер клавиатуры и проч.
//char get_char()
//{
//   return 0;
//}




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
         switch (c1)
         {
            case 0x0e:
               if (cmdlen > 0)
               {
                  cmdlen--;
                  nx = curr_x - 1; ny = curr_y;
                  if (nx < 0) { nx = cols - 1; ny--; }
                  gotoxy(nx, ny); puts(" "); gotoxy(nx, ny);
                  curr_x = nx; curr_y = ny;
               }
               break;
            case 0x02: cmd[cmdlen++] = '1'; puts("1");break;
            case 0x03: cmd[cmdlen++] = '2'; puts("2");break;
            case 0x04: cmd[cmdlen++] = '3'; puts("3");break;
            case 0x05: cmd[cmdlen++] = '4'; puts("4");break;
            case 0x06: cmd[cmdlen++] = '5'; puts("5");break;
            case 0x07: cmd[cmdlen++] = '6'; puts("6");break;
            case 0x08: cmd[cmdlen++] = '7'; puts("7");break;
            case 0x09: cmd[cmdlen++] = '8'; puts("8");break;
            case 0x0a: cmd[cmdlen++] = '9'; puts("9");break;
            case 0x0b: cmd[cmdlen++] = '0'; puts("0");break;
            case 0x10: cmd[cmdlen++] = 'q'; puts("q");break;
            case 0x11: cmd[cmdlen++] = 'w'; puts("w");break;
            case 0x12: cmd[cmdlen++] = 'e'; puts("e");break;
            case 0x13: cmd[cmdlen++] = 'r'; puts("r");break;
            case 0x14: cmd[cmdlen++] = 't'; puts("t");break;
            case 0x15: cmd[cmdlen++] = 'y'; puts("y");break;
            case 0x16: cmd[cmdlen++] = 'u'; puts("u");break;
            case 0x17: cmd[cmdlen++] = 'i'; puts("i");break;
            case 0x18: cmd[cmdlen++] = 'o'; puts("o");break;
            case 0x19: cmd[cmdlen++] = 'p'; puts("p");break;
            case 0x1A: cmd[cmdlen++] = '['; puts("[");break;
            case 0x1B: cmd[cmdlen++] = ']'; puts("]");break;
            case 0x1C:
                       puts("\n");
                       cmd[cmdlen] = 0;
                       stop = 1;
                       break;
            case 0x1E: cmd[cmdlen++] = 'a'; puts("a");break;
            case 0x1F: cmd[cmdlen++] = 's'; puts("s");break;
            case 0x20: cmd[cmdlen++] = 'd'; puts("d");break;
            case 0x21: cmd[cmdlen++] = 'f'; puts("f");break;
            case 0x22: cmd[cmdlen++] = 'g'; puts("g");break;
            case 0x23: cmd[cmdlen++] = 'h'; puts("h");break;
            case 0x24: cmd[cmdlen++] = 'j'; puts("j");break;
            case 0x25: cmd[cmdlen++] = 'k'; puts("k");break;
            case 0x26: cmd[cmdlen++] = 'l'; puts("l");break;
            case 0x27: cmd[cmdlen++] = ';'; puts(";");break;
            case 0x28: cmd[cmdlen++] = '\''; puts("'");break;
            case 0x29: cmd[cmdlen++] = '`'; puts("`");break;
            case 0x2C: cmd[cmdlen++] = 'z'; puts("z");break;
            case 0x2D: cmd[cmdlen++] = 'x'; puts("x");break;
            case 0x2E: cmd[cmdlen++] = 'c'; puts("c");break;
            case 0x2F: cmd[cmdlen++] = 'v'; puts("v");break;
            case 0x30: cmd[cmdlen++] = 'b'; puts("b");break;
            case 0x31: cmd[cmdlen++] = 'n'; puts("n");break;
            case 0x32: cmd[cmdlen++] = 'm'; puts("m");break;
            case 0x33: cmd[cmdlen++] = ','; puts(",");break;
            case 0x34: cmd[cmdlen++] = '.'; puts(".");break;
            case 0x35: cmd[cmdlen++] = '/'; puts("/");break;
            case 0x39: cmd[cmdlen++] = ' '; puts(" ");break;
         }
      }
   }
}
