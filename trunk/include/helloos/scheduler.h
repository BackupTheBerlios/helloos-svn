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


#include <helloos/types.h>
#include <helloos/io.h>
#include <helloos/head.h>
#include <helloos/fat.h>
#include <helloos/elf.h>
#include <helloos/binfmt.h>
#include <helloos/ipc.h>
#include <config.h>

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


// Частота процессора в килогерцах
extern uint cpu_khz;


// Константы состояний процесса

// Выполняется
#define PS_RUNNING      0
// Ожидает завершения другого процесса
#define PS_WAITPID      1
// Ожидает значения общей переменной
#define PS_WAITCOMVAR   2
//// Спит
//#define PS_WAITSLEEP    3


// Эта структура будет хранить контекст задачи. То что
// по науке называется дескриптором процесса будет просто
// указателем на контекст.
typedef struct
{
   // TSS задачи
   TSSStruct tss;
   // Селектор TSS-сегмента в GDT, *включая CPL и TI*. Т.е. чтобы получить
   // номер сегмента, надо сделать tsss>>3.
   uint tsss;

   // Идентификатор
   ulong pid;

   // Состояние процесса
   ulong state;

   // Данные для ожидания
   union
   {
      // Если state==PS_WAITPID, то здесь хранится
      // pid интересующего процесса
      ulong pid;
      struct
      {
         char name[MAX_VAR_NAME];
         uint value;
      } comvar;
   } waitfor;

   // Дескриптор файла. Нужен для demand-loading.
   DirEntry file;

   uchar BinFormat;

   // Заголовки для elf
   Elf32_Ehdr elf_header;
   Elf32_Phdr pheaders[5];

   // Этот массив будет служить стеком для системных вызовов
   uchar syscall_stack[3024];
} TaskStruct;


// Вызывает прерывание таймера. Используется для принудительного
// вызова планировщика
#define CALL_SCHEDULER     call_int(IRQ0_INT)


// Иницилизация многозадачности
void init_scheduler();



// Переменные, полезные другим модулям
extern TaskStruct *Task[CFG_SCHED_MAX_TASK];
extern ulong NTasks;    // Текущее количество живых процессов
extern ulong Current;   // Номер текущего процесса в массиве Task
extern ulong CurPID;     // PID, который будет присвоен очередному


// ТЕСТОВЫЕ ФУНКЦИИ
void scheduler_ps();
void scheduler_kill(ulong pid);
void scheduler_kill_current();
void scheduler_pages(ulong pid);



#endif // __SCHEDULER_H
