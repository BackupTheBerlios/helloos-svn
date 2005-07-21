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


#include "types.h"
#include "config.h"

#include "hello_string.h"
#include "hello_stdio.h"
#include "io.h"
#include "panic.h"

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



// Находит первую попавшуюся страницу,
// помечает как занятую и возвращает ее адрес
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


// Освобождает страницу по указателю на любой
// (физический) адрес в этой странице
void free_page(ulong ptr)
{
   if (! page_map[(ptr>>12) - PAGE_START])
      return panic("Trying to free free page!");
   page_map[(ptr>>12) - PAGE_START] = 0;
}
