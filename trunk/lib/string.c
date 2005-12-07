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


char *strchr(char *s, int c)
{
   while (*s && *s != c) s++;
   if (*s)
      return s;
   else
      return 0;
}


void *memcpy(void *dest, void *src, size_t size)
{
   __asm__(
         "mov %%cl, %%al\n"
         "shr $2, %%ecx\n"
         "cld\n"
         "rep movsl\n"
         "and $0x3, %%al\n"
         "mov %%al, %%cl\n"
         "rep movsb\n"
         :"=D"(dest),"=S"(src),"=c"(size):"D"(dest), "S"(src), "c"(size));
   return dest;
}

int strlen(char *s)
{
   int res;
   __asm__(
         "xor %%al, %%al\n"
         "xor %%ecx, %%ecx\n"
         "dec %%ecx\n"
         "cld\n"
         "repne scasb\n"
         "not %%ecx\n"
         "dec %%ecx\n"
         :"=D"(s),"=c"(res):"D"(s));
   return res;
}

void *memmove(void *dest, void *src, size_t size)
{
   __asm__(
         "mov %%cl, %%al\n"
         "shr $2, %%ecx\n"
         "cmp %%esi, %%edi\n"
         "ja n1\n"
         "  cld\n"
         "  rep movsl\n"
         "  and $0x3, %%al\n"
         "  mov %%al, %%cl\n"
         "  rep movsb\n"
         "  jmp goend\n"
         "n1:\n"
         "  add %%ebx, %%edi\n"
         "  add %%ebx, %%esi\n"
         "  sub $4, %%edi\n"
         "  sub $4, %%esi\n"
         "  std\n"
         "  rep movsl\n"
         "  and $0x3, %%al\n"
         "  add $0x3, %%edi\n"
         "  add $0x3, %%esi\n"
         "  mov %%al, %%cl\n"
         "  rep movsb\n"
         "goend:\n"
         :"=D"(dest),"=S"(src),"=c"(size):"D"(dest),"S"(src),"c"(size),
               "b"(size));
   return dest;
}
