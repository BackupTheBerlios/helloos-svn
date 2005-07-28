/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  На текущей стадии это основной файл ядра системы
 *
 *  Реализован примитивный терминал со встроенной
 *  мини-консолью, понимающей набор отладочных команд.
 *
 */


#include <helloos/io.h>
#include <helloos/types.h>
#include <helloos/fd.h>
#include <string.h>
#include <helloos/colors.h>
#include <helloos/fat.h>

#include <stdarg.h>

#include <helloos/user_syscalls.h>

// Реализация стандартного snprintf
// Сейчас поддерживаются форматы: c, d, p, s, u, x
// Ширина поля и точность НЕ поддерживаются
int _vsnprintf(char *str, size_t maxlen, const char *format, va_list curp)
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
                  if((ui>>j))      // цифру делаем младшей и отделяем её
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

int _snprintf(char *str, size_t maxlen, const char *format, ...)
{
   va_list curp;
   uint res;
   va_start(curp, format);
   res = _vsnprintf(str, maxlen, format, curp);
   va_end(curp);
   return res;
}

// Тупая реализация стандартного vprintf с поддержкой цвета
// Используется vsnprintf, а затем puts
// Может выводить НЕ БОЛЕЕ 256(-1) символов
// Цвет просто передается в puts_color
int _vprintf_color(uchar attr, char *format, va_list curp)
{
   char Buffer[256];
   uint res = _vsnprintf(Buffer, 256, format, curp);
   mysys_nputs_color(Buffer, 256, attr);
   return res;
}

// Реализация стандартного vprintf через vprintf_color
int _vprintf(char *format, va_list curp)
{
   return _vprintf_color(0x0e, format, curp);
}

// Цветной printf через vprintf_color
int _printf_color(uchar attr, char *format, ...)
{
   va_list curp;
   uint res;
   va_start(curp, format);
   res = _vprintf_color(attr, format, curp);
   va_end(curp);
   return res;
}

// Реализация стандартного printf через vprintf_color
int _printf(char *format, ...)
{
   va_list curp;
   uint res;
   va_start(curp, format);
   res = _vprintf_color(0x0e, format, curp);
   va_end(curp);
   return res;
}



// Вывод с цветом без ограничения длины
void _puts_color(const char *s, uchar attr)
{
   mysys_nputs_color(s, -1, attr);
}


// Вывод без цвета без длины
void _puts(const char *s)
{
   mysys_nputs_color(s, -1, 0x0e);
}


// Вывод без цвета, с ограничением длины
void _nputs(const char *s, uint n)
{
   mysys_nputs_color(s, n, 0x0e);
}








// Издает бесконечный звук и может быть полезна для отладки
void make_sound()
{
__asm__(
   "movb    $0xb6, %al\n"
   "outb    %al, $0x43\n"
   "movb    $0x0d, %al\n"
   "outb    %al, $0x42\n"
   "movb    $0x11, %al\n"
   "outb    %al, $0x42\n"
   "inb     $0x61, %al\n"
   "orb     $3, %al\n"
   "outb    %al, $0x61\n");
}



void command(char *cmd)
{
   if (strcmp(cmd, "clear") == 0)   // Очистить экран
   {
      mysys_clear_screen();
      return;
   }
   if (strcmp(cmd, "panic") == 0)   // Напугать ядро
   {
      mysys_panic("As you wish!");
      return;
   }
   if (strcmp(cmd, "reboot") == 0)  // Перезагрузиться
   {
      // Я пока не нашел в документации почему это приводит к перезагрузке
      // На самом деле даже одна любая из этих команд вызывает перезагрузку
      outb(0xfe, 0x64);
      outb(0x01, 0x92);
   }
   if (strcmp(cmd, "fat") == 0)     // Запустить FAT-браузер
   {
//      fat_main();
      return;
   }
   if (strcmp(cmd, "dbg") == 0)     // Вызвать отладчик Bochs
   {
      outw(0x8A00, 0x8A00);
      outw(0x8AE0, 0x8A00);
      return;
   }
   if (strcmp(cmd, "cli") == 0)     // Отключить прерывания, остановить процессы
   {
      __asm__("cli");
      return;
   }
   if (strcmp(cmd, "sti") == 0)     // Включить прерывания
   {
      __asm__("sti");
      return;
   }
   if (strcmp(cmd, "beep") == 0)    // Наступить на хвост
   {
      make_sound();
      return;
   }
   if (strcmp(cmd, "cpu") == 0)     // Определить процессор
   {
      // Идентификация процессора (см. [1])

      ulong eax, ebx, ecx, edx;
      ulong maxeax, maxexeax; // Макс. индексы для Basic и Extended CPUID информации
      ulong vendor[3]; // Имя производителя
      __asm__ volatile("movl $0x0, %%eax\n"
              "cpuid"
              :"=a"(maxeax), "=b"(vendor[0]), "=c"(vendor[2]), "=d"(vendor[1]):);
      __asm__ volatile ("movl $0x80000000, %%eax\n"
              "cpuid"
              :"=a"(maxexeax):);
      _printf("Maximum CPUID indexes: %p/%p", (char*)maxeax, (char*)maxexeax);

      _puts("\nVendor: "); _nputs((char*)&vendor, 12);

      _puts("\nBrand String: ");

      if ((maxexeax & 0x80000000) && maxexeax >= 0x80000004) // Если процессор поддерживает Brand String
      {
         ulong BrandString[12];
         __asm__ volatile ("movl $0x80000002, %%eax\n"
               "cpuid"
               :"=a"(BrandString[0]), "=b"(BrandString[1]), "=c"(BrandString[2]), "=d"(BrandString[3]):);
         __asm__ volatile ("movl $0x80000003, %%eax\n"
               "cpuid"
               :"=a"(BrandString[4]), "=b"(BrandString[5]), "=c"(BrandString[6]), "=d"(BrandString[7]):);
         __asm__ volatile ("movl $0x80000004, %%eax\n"
               "cpuid"
               :"=a"(BrandString[8]), "=b"(BrandString[9]), "=c"(BrandString[10]), "=d"(BrandString[11]):);
         _puts((char*)&BrandString);
      }
      else
         _puts("Not Supported");

      __asm__ volatile ("movl $0x1, %%eax\n"
              "cpuid"
              :"=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx):);
      // Информация о модели процессора
      _printf("\nFamily   : %d", (eax >> 8) & 0x0f);
      _printf("\nModel    : %d", (eax >> 4) & 0x0f);
      _printf("\nStepping : %d", eax & 0x0f);
      _printf("\nProcessor Type: %d", (eax >> 12) & 0x03);
      switch ((eax >> 12) & 0x03)
      {
         case 0: _puts(" (Original OEM)"); break;
         case 1: _puts(" (Intel OverDrive(r))"); break;
         case 2: _puts(" (Dual processor)"); break;
         case 3: _puts(" (reserved?)"); break;
      }

      // Наличие некоторых нам интересных фич
      _puts("\nOn-Chip FPU: ");
      if (!(edx & 0x1)) _puts("NO"); else _puts("YES");
      _puts("\nDebugging Extensions: ");
      if (!(edx & 0x8)) _puts("NO"); else _puts("YES");
      _puts("\nPage Size Extensions: ");
      if (!(edx & 0x10)) _puts("NO"); else _puts("YES");
      _puts("\nOn-Chip APIC: ");
      if (!(edx & 0x200)) _puts("NO"); else _puts("YES");
      _puts("\nMMX : ");
      if (!(edx & 0x800000)) _puts("NO"); else _puts("YES");
      _puts("\nSSE : ");
      if (!(edx & 0x2000000)) _puts("NO"); else _puts("YES");
      _puts("\nSSE2: ");
      if (!(edx & 0x4000000)) _puts("NO"); else _puts("YES");
      return;
   }
   if (strcmp(cmd, "ps") == 0)      // Показать всех
   {
      mysys_ps();
      return;
   }
   if (strncmp(cmd, "kill", 4) == 0)   // Убить процесс. pid задан параметром
   {
      uint i = 5, pid = 0, len = strlen(cmd);
      while (i < len)
      {
         pid *= 10;
         pid += cmd[i++] - '0';
      }
      _printf("%d\n", pid);
      mysys_kill(pid);
      return;
   }
   if (strncmp(cmd, "info", 4) == 0)   // Прочитать a.out-заголовки у файла, заданного параметром
   {
      mysys_bin_info(&cmd[5]);
      return;
   }
   if (strncmp(cmd, "exe", 3) == 0)    // Запустить a.out-файл, заданный параметром
   {
      mysys_bin_load(&cmd[4]);
      return;
   }
   if (strncmp(cmd, "pages", 5) == 0)  // Сколько занимает в памяти процесс. pid задан параметром
   {
      uint i = 6, pid = 0, len = strlen(cmd);
      while (i < len)
      {
         pid *= 10;
         pid += cmd[i++] - '0';
      }
      mysys_pages(pid);
      return;
   }
   if (strcmp(cmd, "gp") == 0)
   {
      *(uchar*)(0xc0000000) = 5;
      return;
   }
   if (strcmp(cmd, "cl") == 0)
   {
      int i;
      for (i = 0; i < 16; i++)
         _puts_color("X", i);
      return;
   }
   _puts("Unknown command\n");
}



static char cmd[100];   // Буфер ввода

int main()
{
// Иницилизация экранного ввода/вывода
// При этом считывается предусмотрительно сохраненные координаты курсора
//   scrio_init(*(uchar*)(0x8000+0x90000),
//              *(uchar*)(0x8001+0x90000));

   // Выводим всякую ерунду
   _printf(FLGREEN"\nHelloOS "FYELLOW"v%d.%d%c"RST" by "FLMAGENTA"Denis Zgursky "
           RST"and "FLCYAN"Ilya Skriblovsky\n",
           VER_MAJOR, VER_MINOR, VER_ALPHA);


   _puts("\nHello World!"); // Куда же без этого?!


   // Имитируем консоль
   while (1)
   {
      _puts("\n>>");
      mysys_readline(cmd, 100);
      command(cmd);
   }
}
