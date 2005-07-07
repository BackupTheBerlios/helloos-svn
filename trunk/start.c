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



#include "io.h"
#include "types.h"
#include "version.h"
#include "panic.h"
#include "fd.h"
#include "hello_stdio.h"
#include "hello_string.h"
#include "fat.h"
#include "scheduler.h"



/*функция издающая долгий и протяжных звук. Использует только ввод/вывод в порты поэтому очень полезна для отладки*/

void make_sound()
{
__asm__("                     \
   movb    $0xB6, %al\n\t     \
   outb    %al, $0x43\n\t     \
   movb    $0x0D, %al\n\t     \
   outb    %al, $0x42\n\t     \
   movb    $0x11, %al\n\t     \
   outb     %al, $0x42\n\t    \
   inb     $0x61, %al\n\t     \
   orb     $3, %al\n\t        \
   outb    %al, $0x61\n\t     \
");
}



uchar *LoadAddress = (uchar*) 0x50000;

bool LoadFileCallback(uchar *Block, ulong len, void *Data)
{
   memcpy(LoadAddress, Block, len);
   LoadAddress += len;

   Data=Data;

   return 1;
}

typedef void (*voidfunc)();


uint dbg(uint addr, void *tss_addr);


uint curaddr = 0x50000;
void command(char *cmd)
{
   uint i;

   printf("You say: %s\n", cmd);
   if (strcmp(cmd, "clear") == 0)
   {
      clear_screen();
      return;
   }
   if (strcmp(cmd, "bin") == 0)
   {
      // Загружаем 5 бинарников из файла bintest.bin, но не больше 25

//      if (curbin >= 25)
//      {
//         puts("already\n");
//         return;
//      }

      DirEntry Entry;
      if (FindEntry(0, "BINTEST BIN", &Entry) != (uint)-1)
      {
         LoadAddress = (uchar*)curaddr;
         FileIterate(&Entry, LoadFileCallback, 0);

         scheduler_dbg(curaddr);
         for (i = 0; i < 4; i++)
         {
            memcpy((void*)(curaddr+0x100), (void*)curaddr, 0x100);
            scheduler_dbg(curaddr+0x100);
            curaddr += 0x100;
         }
         curaddr += 0x100;
      }
      else puts("bintest.bin not found\n");


      return;
   }
   if (strcmp(cmd, "panic") == 0)
   {
      panic("As you wish!");
      return;
   }
   if (strcmp(cmd, "reboot") == 0)
   {
      // Я пока не нашел в документации почему это приводит к перезагрузке
      // На самом деле даже одна любая из этих команд вызывает перезагрузку
      outb(0xfe, 0x64);
      outb(0x01, 0x92);
   }
   if (strcmp(cmd, "fat") == 0)
   {
      fat_main();
      return;
   }
   if (strcmp(cmd, "dbg") == 0)
   {
      outw(0x8A00, 0x8A00);
      outw(0x8AE0, 0x8A00);
      return;
   }
   if (strcmp(cmd, "cli") == 0)
   {
      __asm__("cli");
      return;
   }
   if (strcmp(cmd, "sti") == 0)
   {
      __asm__("sti");
      return;
   }
   if (strcmp(cmd, "beep") == 0)
   {
      make_sound(); // Выключить его невозможно ;)
      return;
   }
   if (strcmp(cmd, "cpu") == 0)
   {
      // Идентификация процессора (см. [1])

      ulong maxeax, maxexeax; // Макс. индексы для Basic и Extended CPUID информации
      ulong vendor[3]; // Имя производителя
      __asm__ volatile("movl $0x0, %%eax\n"
              "cpuid"
              :"=a"(maxeax), "=b"(vendor[0]), "=c"(vendor[2]), "=d"(vendor[1]):);
      __asm__ volatile ("movl $0x80000000, %%eax\n"
              "cpuid"
              :"=a"(maxexeax):);
      printf("Maximum CPUID indexes: %p/%p", maxeax, maxexeax);

      puts("\nVendor: "); nputs((char*)&vendor, 12);

      puts("\nBrand String: ");

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
         puts((char*)&BrandString);
      }
      else
         puts("Not Supported");

      ulong eax, ebx, ecx, edx;
      __asm__ volatile ("movl $0x1, %%eax\n"
              "cpuid"
              :"=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx):);
      // Информация о модели процессора
      printf("\nFamily   : %d", (eax >> 8) & 0x0f);
      printf("\nModel    : %d", (eax >> 4) & 0x0f);
      printf("\nStepping : %d", eax & 0x0f);
      printf("\nProcessor Type: %d", (eax >> 12) & 0x03);
      switch ((eax >> 12) & 0x03)
      {
         case 0: puts(" (Original OEM)"); break;
         case 1: puts(" (Intel OverDrive(r))"); break;
         case 2: puts(" (Dual processor)"); break;
         case 3: puts(" (reserved?)"); break;
      }

      // Наличие некоторых интересных фич
      puts("\nOn-Chip FPU: ");
      if (!(edx & 0x1)) puts("NO"); else puts("YES");
      puts("\nDebugging Extensions: ");
      if (!(edx & 0x8)) puts("NO"); else puts("YES");
      puts("\nPage Size Extensions: ");
      if (!(edx & 0x10)) puts("NO"); else puts("YES");
      puts("\nOn-Chip APIC: ");
      if (!(edx & 0x200)) puts("NO"); else puts("YES");
      puts("\nMMX : ");
      if (!(edx & 0x800000)) puts("NO"); else puts("YES");
      puts("\nSSE : ");
      if (!(edx & 0x2000000)) puts("NO"); else puts("YES");
      puts("\nSSE2: ");
      if (!(edx & 0x4000000)) puts("NO"); else puts("YES");
      return;
   }
   puts("Unknown command\n");
}



/*А вот и основная функция*/

static char cmd[100];

int start_my_kernel()
{
// Иницилизация экранного ввода/вывода
// При этом считывается предусмотрительно сохраненные координаты курсора
   scrio_init(*(uchar*)(0x8000+0x90000),
              *(uchar*)(0x8001+0x90000));

   puts_color("woow!\n", 0x0c);
   puts("Ok, now starting the "); puts_color("kernel", 0x09); puts(".\n\n");
   puts_color("HelloOS ", 0x0a);
   printf_color(0x09, "v%d.%d%c", VER_MAJOR, VER_MINOR, VER_ALPHA);

   puts(" by ");
   puts_color("Denis Zgursky", 0x0d); puts(" and ");
   puts_color("Ilya Skriblovsky\n\n", 0x0b);

   puts("Starting FDC driver... ");
   fd_init();
   puts_color("ok\n", 0x0a);

   puts("Starting FAT driver... ");
   fat_init();
   puts_color("ok\n", 0x0a);

   puts("\nHello World!");

   while (1)
   {
      puts("\n>>");
      readline(cmd, 100);
      command(cmd);
   }

/*уходим в бесконечный цикл*/
   make_sound();
   while(1);
}
