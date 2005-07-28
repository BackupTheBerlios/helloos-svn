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

#include <helloos/types.h>
#include <helloos/io.h>
#include <helloos/panic.h>
#include <config.h>
#include <helloos/scheduler.h>
#include <helloos/head.h>

#include <helloos/scrio.h>
#include <string.h>

#include <helloos/fat.h>
#include <helloos/pager.h>


// ��� ��� PIT-�������. ��. ����.
#define TIMER_VALUE  (1193180L/CFG_SCHED_HZ)


// ������ Task �������� ��������� �� ��������, ����������
// ���������, ����������� �������.
TaskStruct *Task[CFG_SCHED_MAX_TASK];

ulong NTasks;    // ������� ���������� ����� ���������
ulong Current;   // ����� �������� �������� � ������� Task
ulong CurPID;     // PID, ������� ����� �������� ����������
                  // ���������� ��������



// ������������� ������������
// ��� �������� ������ ������, head.S �������� �������� ��� ���������
// TaskStruct � ��������� �� (������������ ��, ��� ���� tss ��������
// ������, � tsss - ������). ����� �������� ��������� � Task[0] ���
// �� ������ ���� �������.
void init_scheduler()
{
   printf_color(0x0b, "Starting scheduler...\t\t"FLGREEN"%dHz switching frequency\n", CFG_SCHED_HZ);

   // ������������� �������� PIT-�������
   outb(36, 0x43);
   outb(TIMER_VALUE & 0xff, 0x40);
   outb(TIMER_VALUE >> 8, 0x40);

   // ��� ��������� ��� ������ ������ ��� �������
//   Task[0]->pid = 0;
//   CurPID = 1;
   CurPID = 0;
   Current = 0;
//   NTasks = 1;
   NTasks = 0;
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

         if (NTasks <= 1) // ������� �������� ��� ��������� :(
            return panic("Heh... Last process has died...\n");
         Task[i] = Task[NTasks-1];
         NTasks--;
         if (Current >= NTasks) Current = 0;
         // �������� ����� ������, �� ������, ���� ����� ������� �������
         CALL_SCHEDULER; // ��� ���� ���� ��� ��������
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
   printf("PID\tFILENAME\tTSSs\tPG_DIR\n");

   uint i;
   uchar name83[12];
   name83[11] = 0;
   for (i = 0; i < NTasks; i++)
   {
      memcpy(&name83, &Task[i]->file.Name, 11);
      int j;
      for (j = 0; j < 11; j++)
         if (name83[j] == 0)
            name83[j] = ' ';
      printf("%d\t%s\t%xh\t%xh\n",
            Task[i]->pid,
            &name83,
            Task[i]->tsss,
            Task[i]->tss.cr3&0xfffff000);
   }
}
