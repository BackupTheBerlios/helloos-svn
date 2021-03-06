/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  �������� ������
 *
 *  ���������� ���, ��� �������� � �����������
 *  �������� �� ������� �� ����.
 *
 *  ����� ����� ���� �������� ������� ������
 *  (������������ ����) � �������� ��������� �����
 *
 */


#include <helloos/types.h>
#include <config.h>
#include <helloos/pager.h>

#include <string.h>
#include <helloos/scrio.h>
#include <helloos/io.h>
#include <helloos/panic.h>
#include <helloos/scheduler.h>

// ���������� �������������� �������
#define PAGES_NR     (CFG_MEM_SIZE-CFG_LOW_MEM)

// ������ �������������� ��������
#define PAGE_START   CFG_LOW_MEM

// ��� ����� ������������ � uchar � ���� �� 1 ����
// �� ��������. �� ��� �������, � ������ ��� �� ����� ;)
ushort page_map[PAGES_NR];


// �������������
void pager_init()
{
   puts_color("Starting memory manager...", 0x0b);
// ������� ���� ���������!
   memset(page_map, 0, sizeof(page_map));
   printf_color(0x0a, "\t%dKb low, %dKb managed memory\n", PAGE_START*4, PAGES_NR*4);
}



// ������� ������ ���������� ���������� ��������,
// �������� ��� ������� � ���������� �� ���������� �����
addr_t alloc_first_page()
{
   ulong res, dumb;
   __asm__ volatile (
         "cld\n"
         "repne scasw\n"
         :"=c"(res),"=D"(dumb):"1"(page_map), "0"(PAGES_NR+1), "a"(0));
   res = PAGES_NR - res;
   if (res == PAGES_NR)
   {
      panic("Cannot allocate first page! Out of memory?!");
      return 0;
   }
   page_map[res] = 1;
   //printf("alloc_first_page=0x%x\n", (res + PAGE_START) * PAGE_SIZE);
   return (res + PAGE_START) * PAGE_SIZE;
}


// ����������� ���������� �������� �� ��������� �� �����
// ���������� ����� � ���� ��������
void free_page(addr_t ptr)
{
   if ((ptr>>12) < PAGE_START)
      return panic("Woow!... Trying to free low-mem!\n");
   if (! page_map[(ptr>>12) - PAGE_START])
      return panic("Trying to free free page!");
   page_map[(ptr>>12) - PAGE_START] = 0;
}


// ��������� ���������� �������� � �������� �� ��������
void map_page(addr_t phaddr, TaskStruct *task, addr_t vaddr, uint flags)
{
   ulong *pg_dir = (ulong*)task->tss.cr3; // ����� �������� ������� ��������
   ulong *pg;  // ����� ������� ������� (���� ����������, ���� ����� �������)

   // ������� �� ��� ��������������� ������� �������? (����������� ��� P)
   if ((pg_dir[vaddr >> 22] & 1) == 0)
   {
      // ���. �������� ��
      pg = (ulong*)alloc_first_page();
      // printf("New page talbe allocated (0x%x)\n", pg);
      memset(pg, 0, PAGE_SIZE);
      // ����������� �� � ��������
      pg_dir[vaddr >> 22] = (ulong)pg + PAGE_ATTR;
   }
   else
      // �������. ������ ������� �� �����
      pg = (ulong*)PAGE_ADDR(pg_dir[vaddr >> 22]);

   // ����������� ����� �������� � ������� �������
   pg[(vaddr >> 12) & 0x3ff] = PAGE_ADDR(phaddr) + flags;

   reload_cr3();
}
