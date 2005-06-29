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


#include "types.h"



void* memcpy(void* dest, const void* src, uint n)
{
   unsigned int i;
   char *d = (char *)dest, *s = (char *)src;
   for (i = 0; i < n; i++) d[i] = s[i];
   return dest;
}



int strcmp(char *a, char *b)
{
   while (*a && *a == *b) a++, b++;
   return *a - *b;
}



int strncmp(char *a, char *b, uint n)
{
   while (*a && (*a == *b) && n) a++, b++, n--;
   if (n)
      return *a - *b;
   else
      return 0;
}


int strlen(char *s)
{
   uint n = 0;
   while (*(s++) != '\0') n++;
   return n;
}
