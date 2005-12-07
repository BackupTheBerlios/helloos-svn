/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
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
#include <stdio.h>

#include <helloos/user_syscalls.h>



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


// Функция консольной команды
typedef void (*cmdfunc)(char *arg1, char *arg2);

// Структура описывает одну команду
typedef struct
{
   char *name;
   char *tmpl;
   char *help;
   cmdfunc func;
} Command;


void help   (char*, char*);
void clear  (char*, char*);
//void reboot (char*, char*);
void beep   (char*, char*);
void cpu    (char*, char*);
void ps     (char*, char*);
void kill   (char*, char*);
void info   (char*, char*);
void exe    (char*, char*);
void bg     (char*, char*);
void pages  (char*, char*);
void ipc    (char*, char*);
void gp     (char*, char*);
void cl     (char*, char*);

// Список команд
Command commands[] = 
{
   {
      .name = "help",
      .tmpl = "help",
      .help = "You're here",
      .func = help,
   },
   {
      .name = "clear",
      .tmpl = "clear",
      .help = "Wipe screen",
      .func = clear,
   },
//   {
//      .name = "reboot",
//      .tmpl = "reboot",
//      .help = "Escape",
//      .func = reboot,
//   },
   {
      .name = "beep",
      .tmpl = "beep",
      .help = "Make beeeeep with pc-speaker",
      .func = beep,
   },
   {
      .name = "cpu",
      .tmpl = "cpu",
      .help = "Identify the CPU with cpuid instruction",
      .func = cpu,
   },
   {
      .name = "ps",
      .tmpl = "ps",
      .help = "List all processes in system",
      .func = ps,
   },
   {
      .name = "kill",
      .tmpl = "kill <pid>",
      .help = "Kill specified process",
      .func = kill,
   },
   {
      .name = "info",
      .tmpl = "info <bin>",
      .help = "Print headers of specified ELF-file",
      .func = info,
   },
   {
      .name = "exe",
      .tmpl = "exe <bin> [arg]",
      .help = "Run specified ELF-file with 'arg' as argv[0]",
      .func = exe,
   },
   {
      .name = "bg",
      .tmpl = "bg <bin> [arg]",
      .help = "Some as 'exe' but run in background",
      .func = bg,
   },
   {
      .name = "pages",
      .tmpl = "pages <pid>",
      .help = "Write memory usage for specified process",
      .func = pages,
   },
   {
      .name = "ipc",
      .tmpl = "ipc",
      .help = "List common variables",
      .func = ipc,
   },
   {
      .name = "gp",
      .tmpl = "gp",
      .help = "Do bad thing and cause the General Protection Fault",
      .func = gp,
   },
   {
      .name = "cl",
      .tmpl = "cl",
      .help = "Character color test",
      .func = cl,
   },
};
// Вычисляем количество команд
#define NCmd   ((int)(sizeof(commands) / sizeof(Command)))


// Вызывается чтобы gcc не вопил про unused parameters
#define MAKE_COMPILER_HAPPY      { arg1 = arg1; arg2 = arg2; }

char zero = '\0';

// Парсим строку, извлекая из нее имя команды и два аргумента
// Аргументы разделяются пробелами. Если в строке больше двух
// аргументов, то все после второго пробела пойдет как второй
// аргумент. Двойные пробелы не работают.
void do_cmd(char *cmd)
{
   if (! *cmd)
      return;

   char *arg1, *arg2 = 0;
   arg1 = strchr(cmd, ' ');
   if (arg1)
   {
      *arg1 = '\0';
      arg1++;
      arg2 = strchr(arg1, ' ');
      if (arg2)
      {
         *arg2 = '\0';
         arg2++;
      }
   }

   // Меняем нулевые указатели на указатели на пустую строку,
   // чтобы функции команд не пытались стучатся по нулевому
   // адресу
   if (! arg1) arg1 = &zero;
   if (! arg2) arg2 = &zero;

   // Ищем нашу команду
   int i;
   for (i = 0; i < NCmd; i++)
   {
      if (strcmp(cmd, commands[i].name) == 0)
      {
         commands[i].func(arg1, arg2);
         return;
      }
   }
   printf("Unknown command '%s'\n", cmd);
}


void help(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   int i;
   for (i = 0; i < NCmd; i++)
      printf("%s\t<- %s\n", commands[i].tmpl, commands[i].help);
}


void clear(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;
   sys_clear_screen();
}

//void reboot(char *arg1, char *arg2)
//{
//   MAKE_COMPILER_HAPPY;
//
//   // Я пока не нашел в документации почему это приводит к перезагрузке
//   outb(0xfe, 0x64);
//   outb(0x01, 0x92);
//}

void beep(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   make_sound();
   return;
}

void cpu(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

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
   printf("Maximum CPUID indexes: %p/%p", (char*)maxeax, (char*)maxexeax);

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

   // Наличие некоторых нам интересных фич
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

void ps(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   sys_ps();
   return;
}

void kill(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   uint pid = 0;
   while (*arg1)
   {
      pid *= 10;
      pid += *arg1 - 0x30;
      *arg1++;
   }
   sys_kill(pid);
   return;
}

void info(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   sys_bin_info(arg1);
   return;
}

void exe(char *arg1, char *arg2)
{
   uint pid = sys_bin_load(arg1, arg2);
   printf_color(0x06, "pid=%d\n", pid);
   sys_waitpid(pid);
   return;
}

void bg(char *arg1, char *arg2)
{
   uint pid = sys_bin_load(arg1, arg2);
   printf_color(0x06, "pid=%d\n", pid);
   return;
}

void pages(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   uint pid = 0;
   while (*arg1)
   {
      pid *= 10;
      pid += *arg1 - 0x30;
      *arg1++;
   }
   sys_pages_info(pid);
   return;
}

void ipc(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   sys_ipc_info();
   return;
}

void gp(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   *(uchar*)(0x80001234) = 5;
   return;
}

void cl(char *arg1, char *arg2)
{
   MAKE_COMPILER_HAPPY;

   int i, j;
   for (j = 0; j < 16; j++)
   {
      for (i = 0; i < 16; i++)
         puts_color("X", j*16+i);
      puts("\n");
   }
   return;
}



static char cmd[100];   // Буфер ввода

int main()
{
// Иницилизация экранного ввода/вывода
// При этом считывается предусмотрительно сохраненные координаты курсора
//   scrio_init(*(uchar*)(0x8000+0x90000),
//              *(uchar*)(0x8001+0x90000));

   // Выводим всякую ерунду
   printf(FLGREEN"\nHelloOS "FYELLOW"v%d.%d%s"RST" by "FLCYAN"Ilya Skriblovsky "
           RST"and "FLMAGENTA"Denis Zgursky\n",
           VER_MAJOR, VER_MINOR, VER_ALPHA);


   puts("\nHello World!"); // Куда же без этого?!


   // Имитируем консоль
   while (1)
   {
      puts("\n>>");
      sys_readline(cmd, 100);
      do_cmd(cmd);
   }
}
