/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции работы со строками, включая аналоги функций
 *  из стандартной библиотеки
 *
 */


#ifndef __HELLO_STRING_H
#define __HELLO_STRING_H



#include <helloos/types.h>


void *memcpy(void *dest, const void *src, size_t n);
void *memmove(uchar *dest, const uchar *src, size_t n);
void *memset(void *s, int c, size_t n);
int strcmp(char *a, char *b);
int strncmp(char *a, char *b, uint n);
int strlen(char *s);



#endif // __HELLO_STRING_H
