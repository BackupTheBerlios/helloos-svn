/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции для работы с бинарниками в формате a.out
 *
 *  Сейчас тут пока всего две функции: печать
 *  заголовков и запуск
 *
 *  Поддерживаются только ZMAGIC-файлы
 *
 *  FIXME: Почему у ZMAGIC секции не выровнены по
 *  страницам? В документации ([8]) написано, что
 *  должны быть выровнены. И в макросах из linux'а они
 *  выравниваются. В чем дело?
 *
 *  FIXME: Как настраивать стек для процессов?
 *
 *  FIXME: Этот файл не должен обращаться к переменным
 *  планировщика. Надо это как-то красиво организовать,
 *  с учетом возможной поддержки других форматов
 *  бинарников.
 *
 */



#include <helloos/scrio.h>
#include <string.h>
#include <helloos/types.h>
#include <helloos/fat.h>
#include <helloos/pager.h>
#include <helloos/scheduler.h>
#include <config.h>
#include <helloos/head.h>
#include <helloos/io.h>
#include <helloos/panic.h>

#include <helloos/aout.h>



// Является ли файл A.OUT-бинарником?
bool aout_is(char *Name)
{
   char name83[11];

   Make83Name(Name, name83);
   DirEntry Entry;
   if (FindEntry(0, name83, &Entry) == (uint)-1)
      return 0;

   uint FirstLong;
   LoadPart(&Entry, &FirstLong, 0, sizeof(uint));
   return FirstLong==0x0064010b;
}



// Печать заголовков файла, заданного именем
void aout_info(char *Name)
{
   char name83[11];
   Exec exec;

   Make83Name(Name, name83);
   DirEntry Entry;
   if (FindEntry(0, name83, &Entry) == (uint)-1)
   {
      printf("Cannot open file '%s'!\n", Name);
      return;
   }
   LoadPart(&Entry, &exec, 0, sizeof(Exec));


   printf("midmag\t= 0x%x\n",   exec.a_midmag);
   printf("text\t= %d\n",     exec.a_text);
   printf("data\t= %d\n",     exec.a_data);
   printf("bss\t= %d\n",      exec.a_bss);
   printf("syms\t= %d\n",     exec.a_syms);
   printf("entry\t= 0x%x\n",    exec.a_entry);
   printf("trsize\t= %d\n",   exec.a_trsize);
   printf("drsize\t= %d\n",   exec.a_drsize);

   printf("\nflags\t= 0x%x\n", N_FLAG(exec));
   printf("machine\t= ");
   switch (N_MID(exec))
   {
      case M_OLDSUN2:   printf("OldSun2"); break;
      case M_68010:     printf("m68010"); break;
      case M_68020:     printf("m68020"); break;
      case M_SPARC:     printf("sparc"); break;
      case M_386:       printf("386"); break;
      case M_MIPS1:     printf("mips1"); break;
      case M_MIPS2:     printf("mips2"); break;
      default:          printf("unknown?"); return;
   }
   printf("\n");
   if (N_MID(exec) != M_386)
   {
      printf("Only 386's binaries are supported\n");
      return;
   }

   printf("magic\t= ");
   switch (N_MAGIC(exec))
   {
      case OMAGIC:   printf("omagic"); break;
      case NMAGIC:   printf("nmagic"); break;
      case ZMAGIC:   printf("zmagic"); break;
      case QMAGIC:   printf("qmagic"); break;
      case CMAGIC:   printf("cmagic"); break;
      default:       printf("bad magic"); return;
   }


//   Relocation_Info rels[100];
// FIXME: добавить вывод relocation records
}


// Определена в head.S
extern void user_exit_code();

// FIXME: это количество страниц, выделяемых на стек процесса.
// Нужно как-то лучше придумать.
#define USER_STACK_PAGES   1

// Запуск файла, заданного именем
// (см. комментарии в binfmt.c)
void aout_load(char *Name)
{
   char name83[11];
   DirEntry Entry;

   Make83Name(Name, name83);

   if (FindEntry(0, name83, &Entry) == (uint)-1)
   {
      printf("Cannot open file '%s'!\n", Name);
      return;
   }

   // Грузим заголовок
   Exec exec;
   LoadPart(&Entry, &exec, 0, sizeof(Exec));

   // Проверяем пригодность файла
   if (N_MID(exec) != M_386)
   {
      printf("Only 386's binaries are supported\n");
      return;
   }
   if (N_MAGIC(exec) != ZMAGIC)
   {
      printf("Not-ZMAGIC binaries are not supported yet\n");
      return;
   }

   // Количество страниц в каждой секции
   ushort TextPages = (exec.a_text + 0xfff) / PAGE_SIZE;
   ushort DataPages = (exec.a_data + 0xfff) / PAGE_SIZE;
   ushort  BSSPages = (exec.a_bss  + 0xfff) / PAGE_SIZE;


   // Создаем каталог страниц
   ulong *pg_dir = (ulong*)alloc_first_page();
   // И записываем в него системные таблицы
   memset(pg_dir, 0, PAGE_SIZE);
   pg_dir[0x200]  = 0x3000 + SYS_PAGE_ATTR;
   pg_dir[0x201]  = 0x4000 + SYS_PAGE_ATTR;


   // Создаем для процесса TaskStruct
   TaskStruct *task = (TaskStruct*)alloc_first_page();

   // Достаем адрес GDT
   GDTDescriptor GDT;
   __asm__("sgdt %0":: "m" (GDT));
   ushort desc_count = (GDT.Size + 1) >> 3;
   // Сюда допишем TSS
   ushort tssn = desc_count;


   // Заполняем атрибуты
   task->pid = CurPID++;
   task->tsss = tssn << 3;
   task->BinFormat = BIN_AOUT;
   // Копируем DirEntry и заголовок A.OUT
   memcpy(&task->file, &Entry, sizeof(DirEntry));
   memcpy(&task->header, &exec, sizeof(Exec));

   
   // Заполняем TSS
   task->tss.tl = 0;
   task->tss.esp0 = (ulong)&task->syscall_stack + sizeof(task->syscall_stack); // Стек для системных вызовов
   task->tss.ss0 = KERNEL_DS;

   task->tss.cr3 = (ulong)pg_dir;
   task->tss.eip = exec.a_entry;
   task->tss.eflags = 0x200; // Только IF
   task->tss.eax = task->tss.ebx =
      task->tss.ecx = task->tss.edx =
      task->tss.esi = task->tss.edi = 0;
   // Стек следует сразу за остальными секциями
   task->tss.esp = task->tss.ebp = (TextPages+DataPages+BSSPages + USER_STACK_PAGES) * PAGE_SIZE - 4; // 4 байта на адрес возврата
   task->tss.cs = USER_CS;
   task->tss.es = task->tss.ss =
      task->tss.ds = task->tss.fs =
      task->tss.gs = USER_DS;
   task->tss.ldt = 0;
   task->tss.iomap_trace = 0;

   // Чтобы процесс имел возможность нормально завершится, мы должны предоставить ему адрес
   // возврата в стеке. Процесс передаст управление по этому адресу при выходе из main().
   // Для этого мы маппируем страницу с функцией user_exit_code (head.S) в АП процесса.
   // Адрес, по которому мы ее будет маппировать, расположим сразу после стека (FIXME: создаем
   // сами себе грабли для динамической линковки...)

   addr_t exit_page = (TextPages+DataPages+BSSPages+USER_STACK_PAGES) * PAGE_SIZE;

   // FIXME: Я НЕ ТЕСТИРОВАЛ ЗАВЕРШЕНИЕ ПРОЦЕССОВ A.OUT
   addr_t stack_page = alloc_first_page();
   map_page(stack_page, task, PAGE_ADDR(task->tss.esp), PAGE_ATTR);
   *(ulong*)(stack_page+0xffc) = exit_page;
   map_page((addr_t)&user_exit_code, task, exit_page, PA_USER | PA_P | PA_NONFREE);


   // Создаем в GDT дескриптор для TSS
   // FIXME: Их нужно удалять при завершении процесса!

   // TSS адресуется через верхнюю память
   ulong tss_addr = (ulong)&task->tss + 0x80000000;
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr - 0x80000000);
   GDT.Addr[tssn].a = (tss_addr<<16)|0x0067;
   GDT.Addr[tssn].b = (tss_addr&0xff000000)|0x00408b00|((tss_addr>>16)&0xff);
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr + 0x80000000);

   GDT.Size += 8; // Один дескриптор добавили
   __asm__("lgdt %0"::"m"(GDT));

   // Ура
   Task[NTasks] = task;
   NTasks++;
}


// Обработчик #PF для A.OUT-процессов
// Аргумент - адрес, по которому программа попыталась обратиться
// Результат - адрес загруженной страницы + флаги, или 0, если процесс "ошибся адресом"
ulong aout_pf(uint address)
{
   TaskStruct *task = Task[Current];

   // Наибольшая страница, которая будет загружаться из файла
   // (я все время предполагаю, что размеры секций кратны странице)
   uint filepages = task->header.a_text + task->header.a_data;

   // Объем АП процесса
   uint maxpage = filepages + PAGE_ADDR(task->header.a_bss + 0xfff)
      + USER_STACK_PAGES * PAGE_SIZE;

   bool ok = 0;

   // Преобразуем адрес в адрес страницы
   ulong pageaddr = PAGE_ADDR(address);
   ulong *tmppage = 0;

   // Если это страница должна грузиться из файла
   if (address < filepages)
   {
      // Выделяем страницу...
      tmppage = (ulong*)alloc_first_page();

      // ... и загружаем ее. Пользуемся тем, что LoadPart остановится
      // на конце файла.
      LoadPart(&task->file, tmppage, pageaddr+N_TXTOFF(task->header), PAGE_SIZE);

      ok = 1;
   }

   // Если это страница bss или стека
   if (address >= filepages && address < maxpage)
   {
      // Выделяем страницу...
      tmppage = (ulong*)alloc_first_page();
      // ... и обнуляем ее
      memset(tmppage, 0, PAGE_SIZE);

      ok = 1;
   }

   if (ok)
      return (ulong)tmppage + PAGE_ATTR;
   else
      return 0;
}
