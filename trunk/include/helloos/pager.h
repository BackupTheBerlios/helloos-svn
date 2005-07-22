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

void pager_init();

ulong alloc_first_page();
void free_page(ulong ptr);


#endif // __PAGER_H
