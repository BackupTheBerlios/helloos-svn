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


#include <helloos/types.h>

void *memcpy(void *dest, const void *src, size_t n)
{
   __asm__(
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         ::"m"(dest), "m"(src), "c"(n) : "si", "di");
   return dest;
}


// FIXME: Надо бы получше разобраться с asm-вставками...
void *memmove(uchar *dest, const uchar *src, size_t n)
{
   uint d1, d2;
   if (dest < src)
      __asm__ volatile(
            "movl %0, %%edi\n"
            "movl %1, %%esi\n"
            "cld\n"
            "rep movsb\n"
            ::"m"(dest),
              "m"(src),
              "c"(n));
   else
      __asm__ volatile(
            "movl %0, %%edi\n"
            "movl %1, %%esi\n"
            "std\n"
            "rep movsb\n"
            :"=D"(d1),
             "=S"(d2)
            :"0"(dest+n-1),
             "1"(src +n-1),
             "c"(n));
   return dest;
}


void *memset(void *s, int c, size_t n)
{
   __asm__("push %%ecx\n"
           "movl %0, %%edi\n"
           "cld\n"
           "rep stosb\n"
           "pop %%ecx\n"
           ::"m"(s), "c"(n), "a"(c) :"di", "si");
   return s;
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
