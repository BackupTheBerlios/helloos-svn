/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Планировщик процессов
 *
 *  Пока это лишь скромная попытка привести в
 *  порядок то, что уже сделано из многозадачности.
 *
 */

#include "types.h"
#include "io.h"
#include "panic.h"
#include "config.h"
#include "scheduler.h"

#include "hello_stdio.h"


// Код для PIT-таймера. См. доки.
#define TIMER_VALUE  (1193180L/CFG_SCHED_HZ)


// TaskPool - пул контекстов процессов. Если поле live равно 1, то
// контекст "живой", т.е. контекст соответствует существующему в
// системе процессу и подлежит планированию. Для всех живых процессов
// в массиве Task должен быть указатель на соответствующий элемент
// массива TaskPool.
TaskStruct TaskPool[CFG_SCHED_MAX_TASK];
TaskStruct *Task[CFG_SCHED_MAX_TASK];
ushort NTasks = 0;   // Текущее количество живых процессов
ushort Current;      // Номер текущего процесса в массиве Task



// Инициализация планировщика. Для создания первой главной задачи
// используются не очень красивые трюки. Загрузочный код в head.S заполняет
// TSS главной задачи, используя адрес TaskPool, т.е. в TaskPool[0]. Поэтому
// поле tss должно быть первым в структуре TaskStruct. Плюс он заносит номер
// TSS-сегмента главной задачи в TaskPool[0].tsss, поэтому оно должно быть
// вторым.
void init_scheduler()
{
   // Устанавливаем задержку PIT-таймера
   outb(36, 0x43);
   outb(TIMER_VALUE & 0xff, 0x40);
   outb(TIMER_VALUE >> 8, 0x40);

   // Помечаем главную задачу, которая уже должна быть создана загрузочной
   // частью в head.S, как живую и текущую.
   TaskPool[0].live = 1;
   Task[0] = &TaskPool[0];
   Current = 0;
   NTasks = 1;

   // Начальная позиция для "мигающих процессов"
   // FIXME: Убрать это безобразие
   __asm__("movl $0xb8f06, 0x70000");
}


// FIXME: временная функция. Попытка переписать содержимое binload.S на С
// в культурном виде. Создает процесс для кода расположенного по адресу addr.
// Создает 3 сегмента: для кода, данных и для tss.
void scheduler_dbg(ulong addr)
{
   GDTDescriptor GDT;

   __asm__("sgdt %0":: "m" (GDT));

   ushort desc_count = (GDT.Size+1) >> 3;
   ushort cs = desc_count, ds = desc_count+1, tsss = desc_count+2;

   GDT.Addr[cs].a = (addr << 16) | 0xffff;
   GDT.Addr[cs].b = (addr&0xff000000)|0x00cf9a00|((addr>>16)&0xff);
   GDT.Addr[ds].a = GDT.Addr[cs].a;
   GDT.Addr[ds].b = (addr&0xff000000)|0x00cf9200|((addr>>16)&0xff);

   uint i;
   for (i = 0; i < CFG_SCHED_MAX_TASK; i++)
      if (! TaskPool[i].live) break;

   if (i == CFG_SCHED_MAX_TASK) panic("Too many processes!");

   TSSStruct *tss = &TaskPool[i].tss;
   TaskPool[i].live = 1;
   TaskPool[i].tsss = tsss << 3;

   tss->tl = 0;
   tss->esp0 = tss->esp1 = tss->esp2 = 0;
   tss->ss0  = tss->ss1  = tss->ss2  = 0x10;
   tss->cr3 = 0;
   tss->eip = 0;
   tss->eflags = 0x200; // IF
   tss->eax = tss->ebx = tss->ecx = tss->edx = tss->esi = tss->edi = 0;
   tss->esp = tss->ebp = 0; // FIXME!!!
   tss->cs = cs<<3;
   tss->es = tss->ss = tss->ds = tss->fs = tss->gs = ds<<3;
   tss->ldt = 0;
   tss->iomap_trace = 0;

   ulong tss_addr = (ulong)tss;
   GDT.Addr[tsss].a = (tss_addr<<16)|0x0067;
   GDT.Addr[tsss].b = (tss_addr&0xff000000)|0x00408b00|((tss_addr>>16)&0xff);

   GDT.Size += 24;
   __asm__("lgdt %0":: "m" (GDT));

   Task[NTasks] = &TaskPool[i];
   NTasks++;
}


// А вот тут, собственно, и заключен наш планировщик. Эта функция вызывается из
// обработчика irq0, который обрабатывает таймер. Планировщик переводит Current
// на новую задачу и заменяет в tss'е обработчика прерывания номер сегмента
// прерванной задачи на новый. Может быть, это не самый красивый способ
// переключения.
extern TSSStruct irq0_tss;
void scheduler()
{
   Current = (Current+1)%NTasks;
//   if (Task[Current]->tsss & 7) panic ("What a strange tsss?");
   irq0_tss.tl = Task[Current]->tsss;
}
