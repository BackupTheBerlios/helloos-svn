/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Заголовок планировщика процессов
 *
 */


#ifndef __SCHEDULER_H
#define __SCHEDULER_H


#include "types.h"

// Эта структура будет хранить TSS задачи. Это лучше чем
// uchar TSS[104], т.к. мы сможем из С-кода трогать чужие
// регистры.
typedef struct _TSSStruct TSSStruct;
struct _TSSStruct
{
   ulong tl;   // Используется только младшее слово
   ulong esp0;
   ulong ss0;  // Используется только младшее слово
   ulong esp1;
   ulong ss1;  // Используется только младшее слово
   ulong esp2;
   ulong ss2;  // Используется только младшее слово
   ulong cr3;
   ulong eip;
   ulong eflags;
   ulong eax;
   ulong ecx;
   ulong edx;
   ulong ebx;
   ulong esp;
   ulong ebp;
   ulong esi;
   ulong edi;
   ulong es;   // Используется только младшее слово
   ulong cs;   // Используется только младшее слово
   ulong ss;   // Используется только младшее слово
   ulong ds;   // Используется только младшее слово
   ulong fs;   // Используется только младшее слово
   ulong gs;   // Используется только младшее слово
   ulong ldt;
   ulong iomap_trace;
};


// Эта структура будет хранить контекст задачи. То что
// по науке называется дескриптором процесса будет просто
// указателем на контекст.
typedef struct _TaskStruct TaskStruct;
struct _TaskStruct
{
   // TSS задачи
   TSSStruct tss;
   // Дескриптор TSS-сегмента в GDT, *включая CPL и TI*. Т.е. чтобы получить
   // номер сегмента, надо сделать tsss>>3.
   uint tsss;

   // Соответствует ли этот контекст существующей задаче
   bool live;

   // Пока тут больше ничего нет. FIXME: добавить полезную инфу
};



// Иницилизация многозадачности
void init_scheduler();

// Тупое создание процесса. Убрать на фиг.
void scheduler_dbg(ulong addr);

#endif // __SCHEDULER_H
