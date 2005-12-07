/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Менеджер памяти
 *
 *  Занимается тем, что выделяет и освобождает
 *  страницы по запросу от ядра.
 *
 *  Потом здесь надо изменить принцип работы
 *  (использовать стек) и добавить поддержку свопа
 *
 */


#include <helloos/types.h>
#include <config.h>
#include <helloos/pager.h>

#include <string.h>
#include <helloos/scrio.h>
#include <helloos/io.h>
#include <helloos/scheduler.h>

// Количество распределяемых страниц
#define PAGES_NR     (CFG_MEM_SIZE-CFG_LOW_MEM)

// Первая распределяемая страница
#define PAGE_START   CFG_LOW_MEM

// Тут можно использовать и uchar и даже по 1 биту
// на страницу. Но так быстрее, а памяти нам не жалко ;)
ushort page_map[PAGES_NR];


// Инициализация
void pager_init()
{
   puts_color("Starting memory manager...", 0x0b);
// Свободу всем страницам!
   memset(page_map, 0, sizeof(page_map));
   printf_color(0x0a, "\t%dKb low, %dKb managed memory\n", PAGE_START*4, PAGES_NR*4);
}



// Находит первую попавшуюся физическую страницу,
// помечает как занятую и возвращает ее физический адрес
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
      printf_color(0x0c, "Cannot allocate first page! Out of memory?!");
      scheduler_kill_current();
      return 0;
   }
   page_map[res] = 1;
   //printf("alloc_first_page=0x%x\n", (res + PAGE_START) * PAGE_SIZE);
   return (res + PAGE_START) * PAGE_SIZE;
}


// Освобождает физическую страницу по указателю на любой
// физический адрес в этой странице
void free_page(addr_t ptr)
{
   if ((ptr>>12) < PAGE_START)
   {
      printf_color(0x0c, "Woow!... Trying to free low-mem!\n");
      return scheduler_kill_current();
   }
   if (! page_map[(ptr>>12) - PAGE_START])
   {
      printf_color(0x0c, "Trying to free free page!");
      return scheduler_kill_current();
   }
   page_map[(ptr>>12) - PAGE_START] = 0;
}


// Маппирует физическую страницу в линейное АП процесса
void map_page(addr_t phaddr, TaskStruct *task, addr_t vaddr, uint flags)
{
   ulong *pg_dir = (ulong*)task->tss.cr3; // Адрес каталога страниц процесса
   ulong *pg;  // Адрес таблицы страниц (либо существует, либо будет создана)

   // Создана ли уже соответствующая таблица страниц? (проверяется бит P)
   if ((pg_dir[vaddr >> 22] & 1) == 0)
   {
      // Нет. Создадим ее
      pg = (ulong*)alloc_first_page();
      // printf("New page talbe allocated (0x%x)\n", pg);
      memset(pg, 0, PAGE_SIZE);
      // Прописываем ее в каталоге
      pg_dir[vaddr >> 22] = (ulong)pg + PAGE_ATTR;
   }
   else
      // Создана. Просто возьмем ее адрес
      pg = (ulong*)PAGE_ADDR(pg_dir[vaddr >> 22]);

   // Прописываем новую страницу в таблице страниц
   pg[(vaddr >> 12) & 0x3ff] = PAGE_ADDR(phaddr) + flags;

   reload_cr3();
}
