/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ������� � ��������� ��� ������ � ���������
 *  �������
 *
 *
 *  ����� ��������� ������. ��� ������� ��������� �� �����
 *  ���� ���������� ����� � ������ �� �����������.  �����
 *  ����� ������� ����������� � ������ ����� (� ����)
 *  ��������� ��������� *  �� ���������������� ������������
 *  ������. ��� ���� ���������� ���������� Page Fault, �
 *  ������� load_page ������, ���������������� �������
 *  �����, ������ ��������� ������ ��������.  ��� ���������
 *  ����� �������� � TaskStruct ����������� ���������
 *  ���������, ��������� ��� ��������� ��������.
 *
 */


#include <helloos/binfmt.h>

#include <helloos/aout.h>
#include <helloos/elf.h>
#include <helloos/scheduler.h>
#include <helloos/scrio.h>
#include <helloos/pager.h>
#include <string.h>


// ��������� ��������� ������� �������� ������
BinFmt BinFormats[BIN_N] = 
{
   {  // BIN_AOUT
      .FormatName = "A.OUT",
      .is = aout_is,
      .dump_info = aout_info,
      .load_bin = aout_load,
      .load_page = aout_pf,
   },
   {  // BIN_ELF
      .FormatName = "ELF",
      .is = elf_is,
      .dump_info = elf_info,
      .load_bin = elf_load,
      .load_page = elf_pf,
   }
};


// ���������� ��� ���������
int bin_type(char *name)
{
   uint i;
   for (i = 0; i < BIN_N; i++)
      if (BinFormats[i].is(name))
         return i;
   return -1;
}

// ���������� ��������� ���������
bool bin_dump_info(char *name)
{
   int type = bin_type(name);
   if (type != -1)
   {
      BinFormats[type].dump_info(name);
      return 1;
   }
   else
      return 0;
}

// ��������� ��������
bool bin_load_bin(char *name)
{
   int type = bin_type(name);
   if (type != -1)
   {
      BinFormats[type].load_bin(name);
      return 1;
   }
   else
      return 0;
}


// ���������� #PF
void pf_handler(uint address, uint errcode)
{
//   printf_color(0x04, "Page Fault: addr=0x%x, errcode=0x%x\n", address, errcode);

   ulong ok = 0;
   TaskStruct *task = Task[Current];


   // ������� ��� errcode ����������, ���� �� �������
   // ���������� ������������� ��������� (0) ��� ����������
   // ����������
   if ((errcode & 1) == 0)
   {
      if (BinFormats[task->BinFormat].load_page)
         ok = BinFormats[task->BinFormat].load_page(address);
   }

   if (! ok)
   {
      printf_color(0x04, "Process requested an invalid page (req addr=0x%x)! Killing him...\n", address);
      scheduler_kill_current();
   }
   else
   {
      // #PF ���������, �������� ���������. ������ ���� �� ���������
      // � �������� � ������� �������
      ulong *pg_dir = (ulong*)task->tss.cr3; // ����� �������� �������
      ulong *pg;  // ����� ������� �������

      // ������� �� ��������������� ������� �������?...
      if ((pg_dir[address>>22] & 1) == 0)
      {
         // ... ���. ������� ��...
         pg = (ulong*)alloc_first_page();
//         printf("New page table allocated (0x%x)\n", pg);
         memset(pg, 0, 0x1000);
         // ... � ������������ � ��������
         pg_dir[address>>22] = (ulong)pg + 0x7;
      }
      else
         // ... ��. ������ ����� �� �����.
         pg = (ulong*)(pg_dir[address>>22] & 0xfffff000);

      // ����������� ����� �������� � ������� �������
      pg[(address>>12) & 0x3ff] = ok + 0x7;

      // ����������� cr3 ����� �������� ��� �������
      __asm__("mov %%eax, %%cr3\n"
            ::"a"(task->tss.cr3));
   }
}
