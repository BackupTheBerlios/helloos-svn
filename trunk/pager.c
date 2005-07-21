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


#include "types.h"
#include "config.h"

#include "hello_string.h"
#include "hello_stdio.h"
#include "io.h"
#include "panic.h"

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



// ������� ������ ���������� ��������,
// �������� ��� ������� � ���������� �� �����
ulong alloc_first_page()
{
   ulong res, dumb;
   __asm__ volatile (
         "cld\n"
         "repne scasw\n"
         :"=c"(res),"=D"(dumb):"1"(page_map), "0"(PAGES_NR+1), "a"(0));
   res = PAGES_NR - res;
   if (res == PAGES_NR)
   {
      panic("Cannot allocate first page! Out of memory!");
      return 0;
   }
   page_map[res] = 1;
   //printf("alloc_first_page=0x%x\n", (res + PAGE_START)*0x1000);
   return (res + PAGE_START)*0x1000;
}


// ����������� �������� �� ��������� �� �����
// (����������) ����� � ���� ��������
void free_page(ulong ptr)
{
   if (! page_map[(ptr>>12) - PAGE_START])
      return panic("Trying to free free page!");
   page_map[(ptr>>12) - PAGE_START] = 0;
}
