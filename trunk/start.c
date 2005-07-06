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

   puts("You say: "); puts(cmd); puts("\n");
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
   puts("Ok, now starting the "); puts_color("kernel", 0x09); puts(".\n");
   puts("\n");
   puts_color("HelloOS ", 0x0a);
   // FIXME: Здесь нужен Денисовский printf. Но мне сейчас лень его вписывать. Сейчас предполагается, что цифры в версии <10.
   nputs("v", 1);
   char ver_tmp = VER_MAJOR + 0x30;
   nputs_color(&ver_tmp, 1, 0x09); nputs_color(".", 1, 0x09);
   ver_tmp = VER_MINOR + 0x30;
   nputs_color(&ver_tmp, 1, 0x09);
   ver_tmp = VER_ALPHA;
   nputs_color(&ver_tmp, 1, 0x09);

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
