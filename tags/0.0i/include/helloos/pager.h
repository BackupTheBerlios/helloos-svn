/*
 * -= HelloOS Educational Project =-
 *
 *  $Id$
 *
 *  Заголовок менеджера памяти
 *
 */


#ifndef __PAGER_H
#define __PAGER_H


#include <helloos/types.h>
#include <helloos/scheduler.h>

void pager_init();

ulong alloc_first_page();
void free_page(ulong ptr);

void map_page(addr_t phaddr, TaskStruct *task, addr_t vaddr, uint flags);


// Перегружает CR3 сбрасывая кэш страниц
#define reload_cr3()  { __asm__("mov %cr3, %eax ; mov %eax, %cr3"); }

// Возвращает указатель на начало страницы, которой принадлежит
// аргумент-указатель
#define PAGE_ADDR(addr) ((addr) & 0xfffff000)

// Размер страницы
#define PAGE_SIZE    0x1000


// Атрибуты страниц и таблиц страниц
// Присутствие
#define PA_P      1
// Доступность на запись
#define PA_W      2
// Доступность для CPL=3
#define PA_USER   4
// К странице было обращение (accessed)
#define PA_A      32
// В страницу была запись
#define PA_D      64

// Флаги, определяемые HelloOS
// Страницу не следует освобождать
#define PA_NONFREE   512


// Маска флагов страницы
#define PA_MASK      0xfff


// Атрибуты (права) создаваемых страниц. FIXME: Они должны быть
// разными, и такой константы не должно быть.
#define PAGE_ATTR    0x7

// Права для системных таблиц страниц
#define SYS_PAGE_ATTR    0x5


#endif // __PAGER_H
