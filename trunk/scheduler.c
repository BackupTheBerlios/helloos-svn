/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ����������� ���������
 *
 *  �������, ����������� � ���������� ����������
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


// ��� ��� PIT-�������. ��. ����.
#define TIMER_VALUE  (1193180L/CFG_SCHED_HZ)


// ������ Task �������� ��������� �� ��������, ����������
// ���������, ����������� �������.
TaskStruct *Task[CFG_SCHED_MAX_TASK];

ushort NTasks;    // ������� ���������� ����� ���������
ushort Current;   // ����� �������� �������� � ������� Task
ulong CurPID;     // PID, ������� ����� �������� ����������
                  // ���������� ��������



// ������������� ������������
// ��� �������� ������ ������, head.S �������� �������� ��� ���������
// TaskStruct � ��������� �� (������������ ��, ��� ���� tss ��������
// ������, � tsss - ������). ����� �������� ��������� � Task[0] ���
// �� ������ ���� �������.
void init_scheduler()
{
   // ������������� �������� PIT-�������
   outb(36, 0x43);
   outb(TIMER_VALUE & 0xff, 0x40);
   outb(TIMER_VALUE >> 8, 0x40);

   // ��� ��������� ��� ������ ������ ��� �������
   Task[0]->pid = 0;
   CurPID = 1;
   Current = 0;
   NTasks = 1;
}


// ������� ������� �������, �������� ���������������
// ��� ���� ������������� ��� ������� �� ��������, � �.�. ��������,
// ���������� ��� ������� ������� � TSS
void scheduler_kill(ulong pid)
{
      ulong i;
      for (i = 0; i < NTasks; i++)
      {
         if (Task[i]->pid == pid)
         {
            if (pid == 0)
               // ������� ������ ���� �������������� ��������� �������: � ��� CPL=0,
               // ��� ��� ������������ ��� 8Mb ������, � �.�. � ����������������, � ����.
               // ������� �� �� ������� ���� �� �����.
               printf_color(0x4, "Freeing of main process is not implemented yet\n");
            else
            {
               uint pagecount = 0;
               TaskStruct* task = Task[i];
               int j, k;
               // �������� �� �������� ������� ������ � ���� �������� ������
               // ��� ������ ������ �������� �� ��������������� ������� �������
               // � ����������� ���������� ��������. �� ���� ������� �� ����������.
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

            if (NTasks <= 1) // ������� �������� ��� ��������� :(
               return panic("Heh... Last process has died...\n");
            Task[i] = Task[NTasks-1];
            NTasks--;
            if (Current >= NTasks) Current = 0;
            // �������� ����� ������, �� ������, ���� ����� ������� �������
            __asm__("int $0x8"); // ��� ���� ���� ��� ��������
            return;
         }
      }
}

// ����� ������� ������. ���������� �� ������������ #GP, #PF � ��.
void scheduler_kill_current()
{
   scheduler_kill(Task[Current]->pid);
}




// �������� ���������� � ���, ������� ������� �������� �������
// � ���������������� ��������� ��������� ��, ������� ������
// ����� � ������� ��������, � � ��������� - ���������� �������
// � ������� �������, TSS
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


// � ��� ���, ����������, � �������� ��� �����������. ��� ������� ���������� ��
// ����������� irq0, ������� ������������ ������. ����������� ��������� Current
// �� ����� ������ � �������� � tss'� ����������� ���������� ����� ��������
// ���������� ������ �� �����. ����� ����, ��� �� ����� �������� ������
// ������������.
extern TSSStruct irq0_tss;
void scheduler()
{
   if (NTasks == 0)
      return panic("No processes!");
   Current = (Current+1)%NTasks;

   irq0_tss.tl = Task[Current]->tsss;

}


// ������� ������� � ���������� ���������. ��� ������� �������� ���������� ���
// �������������, ����� �������� TSS � ����� �������� �������.
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
