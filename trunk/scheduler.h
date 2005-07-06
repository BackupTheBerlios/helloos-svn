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


#include "types.h"

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
   // ���������� TSS-�������� � GDT, *������� CPL � TI*. �.�. ����� ��������
   // ����� ��������, ���� ������� tsss>>3.
   uint tsss;

   // ������������� �� ���� �������� ������������ ������
   bool live;

   // ���� ��� ������ ������ ���. FIXME: �������� �������� ����
};



// ������������ ���������������
void init_scheduler();

// ����� �������� ��������. ������ �� ���.
void scheduler_dbg(ulong addr);

#endif // __SCHEDULER_H
