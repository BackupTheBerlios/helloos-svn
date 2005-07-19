/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Планировщик процессов
 *
 *  Функции, относящиеся к управлению процессами
 *
 */

#include "types.h"
#include "io.h"
#include "panic.h"
#include "config.h"
#include "scheduler.h"
#include "head.h"

#include "hello_stdio.h"
#include "hello_string.h"

#include "fat.h"
#include "pager.h"


// Код для PIT-таймера. См. доки.
#define TIMER_VALUE  (1193180L/CFG_SCHED_HZ)


// Массив Task содержит указатели на страницы, содержащие
// структуру, описывающую процесс.
TaskStruct *Task[CFG_SCHED_MAX_TASK];

ushort NTasks;    // Текущее количество живых процессов
ushort Current;   // Номер текущего процесса в массиве Task
ulong CurPID;     // PID, который будет присвоен очередному
                  // созданному процессу



// Инициализация планировщика
// Для создания первой задачи, head.S выделяет страницу для структуры
// TaskStruct и заполняет ее (используется то, что поле tss является
// первым, а tsss - вторым). Адрес страницы заносится в Task[0] еще
// до вызова этой функции.
void init_scheduler()
{
   // Устанавливаем задержку PIT-таймера
   outb(36, 0x43);
   outb(TIMER_VALUE & 0xff, 0x40);
   outb(TIMER_VALUE >> 8, 0x40);

   // Все остальное для первой задачи уже сделано
   Task[0]->pid = 0;
   CurPID = 1;
   Current = 0;
   NTasks = 1;
}


// Функция убивает процесс, заданный идентификатором
// При этом освобождаются все занятые им страницы, в т.ч. страницы,
// содержащие его каталог страниц и TSS
void scheduler_kill(ulong pid)
{
      ulong i;
      for (i = 0; i < NTasks; i++)
      {
         if (Task[i]->pid == pid)
         {
            if (pid == 0)
               // Главная задача пока обрабатывается особенным образом: у нее CPL=0,
               // для нее смаппированы все 8Mb памяти, в т.ч. и нераспределяемая, и проч.
               // Поэтому мы ее чистить пока не будем.
               printf_color(0x4, "Freeing of main process is not implemented yet\n");
            else
            {
               uint pagecount = 0;
               TaskStruct* task = Task[i];
               int j, k;
               // Проходим по каталогу страниц задачи и ищем непустые записи
               // Для каждой записи проходим по соответствующей таблице страниц
               // и освобождаем выделенные страницы. По пути считаем их количество.
               ulong *pg_dir = (ulong*)(task->tss.cr3 & 0xfffff000);
               for (j = 0; j < 512; j++)
                  if ((ulong)pg_dir[j] & 0x1)
                  {
                     for (k = 0; k < 1024; k++)
                        if (((ulong*)(pg_dir[j]&0xfffff000))[k] & 1)
                        {
                           pagecount++;
                           free_page(((ulong*)(pg_dir[j]&0xfffff000))[k]);
                        }
                     pagecount++;
                     free_page((pg_dir[j]&0xfffff000));
                  }
               pagecount+=2;
               free_page(task->tss.cr3);
               free_page((ulong)task);

               printf_color(0x4, "\n%d pages freed\n", pagecount);
            }

            if (NTasks <= 1) // Система осталась без процессов :(
               return panic("Heh... Last process has died...\n");
            Task[i] = Task[NTasks-1];
            NTasks--;
            if (Current >= NTasks) Current = 0;
            // Вызываем смену задачи, на случай, если убили текущий процесс
            __asm__("int $0x8"); // Над этим надо еще подумать
            return;
         }
      }
}

// Убить текущую задачу. Вызывается из обработчиков #GP, #PF и др.
void scheduler_kill_current()
{
   scheduler_kill(Task[Current]->pid);
}




// Печатает информацию о том, сколько страниц занимает процесс
// К пользовательским страницам относятся те, которые заняты
// кодом и данными процесса, а к системным - содержащие каталог
// и таблицы страниц, TSS
void scheduler_pages(ulong pid)
{
   ulong i;
   for (i = 0; i < NTasks; i++)
   {
      if (Task[i]->pid == pid)
      {
         TaskStruct* task = Task[i];
         int j, k;
         ulong *pg_dir = (ulong*)(task->tss.cr3 & 0xfffff000);
         uint count = 0, syscount = 0;
         for (j = 0; j < 512; j++)
            if ((ulong)pg_dir[j] & 0x1)
            {
               for (k = 0; k < 1024; k++)
                  if (((ulong*)(pg_dir[j]&0xfffff000))[k] & 1)
                     count++;
               syscount++;
            }
         printf("User pages:\t%d\n", count);
         syscount += 2;
         printf("System pages:\t%d\n", syscount);
         printf("Total pages:\t%d\n", count+syscount);
      }
   }
   printf("\n");
}


// А вот тут, собственно, и заключен наш планировщик. Эта функция вызывается из
// обработчика irq0, который обрабатывает таймер. Планировщик переводит Current
// на новую задачу и заменяет в tss'е обработчика прерывания номер сегмента
// прерванной задачи на новый. Может быть, это не самый красивый способ
// переключения.
extern TSSStruct irq0_tss;
void scheduler()
{
   if (NTasks == 0)
      return panic("No processes!");
   Current = (Current+1)%NTasks;

   irq0_tss.tl = Task[Current]->tsss;

}


// Выводит справку о запущенных процессах. Для каждого процесса печатается его
// идентификатор, номер сегмента TSS и адрес каталога страниц.
void scheduler_ps()
{
   printf("PID\tTSSs\tPG_DIR\n");

   uint i;
   for (i = 0; i < NTasks; i++)
   {
      printf("%d\t%xh\t%xh\n",
            Task[i]->pid,
            Task[i]->tsss,
            Task[i]->tss.cr3&0xfffff000);
   }
}
