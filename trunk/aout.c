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



#include "hello_stdio.h"
#include "hello_string.h"
#include "types.h"
#include "fat.h"
#include "pager.h"
#include "scheduler.h"
#include "config.h"
#include "head.h"
#include "io.h"

#include "aout.h"



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


extern ulong CurPID; // Гадость, я знаю :(
extern ushort NTasks;
extern TaskStruct *Task[CFG_SCHED_MAX_TASK];

// Запуск файла, заданного именем
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

   Exec exec;
   LoadPart(&Entry, &exec, 0, sizeof(Exec));

   if (N_MID(exec) != M_386)
   {
      printf("Only 386's binaries are supported\n");
      return;
   }
   if (N_MAGIC(exec) != ZMAGIC)
   {
      printf("Not-zmagic binaries are not supported yet\n");
      return;
   }

   ushort TextPages = (exec.a_text + 0xfff) / 0x1000;
   ushort DataPages = (exec.a_data + 0xfff) / 0x1000;
   ushort  BSSPages = (exec.a_bss  + 0xfff) / 0x1000;

   ulong *pg_dir, *pg0;
   pg_dir   = (ulong*)alloc_first_page();
   pg0      = (ulong*)alloc_first_page();

   memset(pg_dir, 0, 0x1000);
   memset(pg0   , 0, 0x1000);
   pg_dir[0]      = (ulong)pg0 + 0x7;
   pg_dir[0x200]  = 0x3000 + 0x7;
   pg_dir[0x201]  = 0x4000 + 0x7;

   int i;
   ulong pg;
   for (i = 0; i < TextPages; i++)
   {
      pg = alloc_first_page();
      LoadPart(&Entry, (void*)pg, N_TXTOFF(exec)+i*0x1000, 0x1000);
      pg0[i] = pg + 0x7;
   }
   // Я надеюсь, что хотя бы размеры секций выровнены по страницам
   for (i = 0; i < DataPages; i++)
   {
      pg = alloc_first_page();
      LoadPart(&Entry, (void*)pg, N_DATOFF(exec)+i*0x1000, 0x1000);
      pg0[TextPages+i] = pg + 0x7;
   }
   for (i = 0; i < BSSPages; i++)
   {
      pg = alloc_first_page();
      memset((void*)pg, 0, 0x1000); // В доке [8] сказано занулять
      pg0[TextPages+DataPages+i] = pg + 0x7;
   }

   // FIXME: Для стека выделяется одна отдельная страница
   ulong pg_stack = alloc_first_page();
   ulong user_stack_page = (TextPages+DataPages+BSSPages) * 0x1000;
   pg0[TextPages+DataPages+BSSPages] = pg_stack + 0x7;



   TaskStruct *task = (TaskStruct*)alloc_first_page();

   GDTDescriptor GDT;
   __asm__("sgdt %0":: "m" (GDT));
   ushort desc_count = (GDT.Size + 1) >> 3;
   ushort tssn = desc_count;


   task->pid = CurPID++;
   task->tsss = tssn << 3;

   task->tss.tl = 0;
   task->tss.esp0 = (ulong)&task->syscall_stack + sizeof(task->syscall_stack); // Стек для системных вызовов
   task->tss.ss0 = KERNEL_DS;

   task->tss.cr3 = (ulong)pg_dir;
   task->tss.eip = exec.a_entry;
   task->tss.eflags = 0x200; // Только IF
   task->tss.eax = task->tss.ebx =
      task->tss.ecx = task->tss.edx =
      task->tss.esi = task->tss.edi = 0;
   task->tss.esp = task->tss.ebp = user_stack_page-4;
   task->tss.cs = USER_CS;
   task->tss.es = task->tss.ss =
      task->tss.ds = task->tss.fs =
      task->tss.gs = USER_DS;
   task->tss.ldt = 0;
   task->tss.iomap_trace = 0;

   // TSS адресуется через верхнюю память
   ulong tss_addr = (ulong)&task->tss + 0x80000000;
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr - 0x80000000);
   GDT.Addr[tssn].a = (tss_addr<<16)|0x0067;
   GDT.Addr[tssn].b = (tss_addr&0xff000000)|0x00408b00|((tss_addr>>16)&0xff);
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr + 0x80000000);

   GDT.Size += 8; // Один дескриптор добавили
   __asm__("lgdt %0"::"m"(GDT));

   Task[NTasks] = task;
   NTasks++;
}
