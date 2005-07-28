/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ��������� ������������ ���������
 *
 */


#ifndef __SCHEDULER_H
#define __SCHEDULER_H


#include <helloos/types.h>
#include <helloos/io.h>
#include <helloos/head.h>
#include <helloos/fat.h>
#include <helloos/aout.h>
#include <helloos/elf.h>
#include <helloos/binfmt.h>
#include <config.h>

// ��� ��������� ����� ������� TSS ������. ��� ����� ���
// uchar TSS[104], �.�. �� ������ �� �-���� ������� �����
// ��������.
typedef struct _TSSStruct TSSStruct;
struct _TSSStruct
{
   ulong tl;   // ������������ ������ ������� �����
   ulong esp0;
   ulong ss0;  // ������������ ������ ������� �����
   ulong esp1;
   ulong ss1;  // ������������ ������ ������� �����
   ulong esp2;
   ulong ss2;  // ������������ ������ ������� �����
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
   ulong es;   // ������������ ������ ������� �����
   ulong cs;   // ������������ ������ ������� �����
   ulong ss;   // ������������ ������ ������� �����
   ulong ds;   // ������������ ������ ������� �����
   ulong fs;   // ������������ ������ ������� �����
   ulong gs;   // ������������ ������ ������� �����
   ulong ldt;
   ulong iomap_trace;
};


// ��� ��������� ����� ������� �������� ������. �� ���
// �� ����� ���������� ������������ �������� ����� ������
// ���������� �� ��������.
typedef struct _TaskStruct TaskStruct;
struct _TaskStruct
{
   // TSS ������
   TSSStruct tss;
   // �������� TSS-�������� � GDT, *������� CPL � TI*. �.�. ����� ��������
   // ����� ��������, ���� ������� tsss>>3.
   uint tsss;

   // �������������
   ulong pid;

   // ���������� �����. ����� ��� demand-loading.
   DirEntry file;

   uchar BinFormat;
   // ��������� a.out
   Exec header;

   Elf32_Ehdr elf_header;
   Elf32_Phdr pheaders[5];

   // ���� ������ ����� ������� ������ ��� ��������� �������
   uchar syscall_stack[3024];
};


// �������� ���������� �������. ������������ ��� ���������������
// ������ ������������
#define CALL_SCHEDULER     call_int(IRQ0_INT)


// ������������ ���������������
void init_scheduler();

// ����� �������� ��������. ������ �� ���.
void scheduler_dbg(ulong addr);


// ����������, �������� ������ �������
extern TaskStruct *Task[CFG_SCHED_MAX_TASK];
extern ulong NTasks;    // ������� ���������� ����� ���������
extern ulong Current;   // ����� �������� �������� � ������� Task
extern ulong CurPID;     // PID, ������� ����� �������� ����������


// �������� �������
void scheduler_ps();
void scheduler_kill(ulong pid);
void scheduler_kill_current();
void scheduler_pages(ulong pid);



#endif // __SCHEDULER_H
