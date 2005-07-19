/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Демонстрационный бинарник. Компилируется
 *  в формат a.out и может быть запущен ядром.
 *
 */


#include "types.h"

// Заглушки системных вызовов. Разумеется,
// потом они будут в пользовательских заголовках.

uint mysys_getnewcharaddr()
{
   uint res;
   __asm__("int $0x80\n"
         :"=a"(res):"0"(0));
   return res;
}

uint mysys_incvideochar(uint addr)
{
   uint res;
   __asm__("int $0x80\n"
         :"=a"(res):"0"(1),"b"(addr));
   return res;
}
uint mysys_nputs_color(char *s, uint n, uchar attr)
{
   uint res;
   __asm__("int $0x80\n"
         :"=a"(res):"0"(2),"b"(s),"c"(n),"d"(attr));
   return res;
}

int main()
{
   // Просим себе символ на экране
   uint myaddr = mysys_getnewcharaddr();
//   mysys_nputs_color("I'm alive!\n", -1, 0xc);

   // И мигаем им до посинения
#ifdef INVOKE_GP
   uint counter = 10000;
#endif
#ifdef INVOKE_PF
   uint counter = 10000;
#endif
   while (1)
   {
      mysys_incvideochar(myaddr);
#ifdef INVOKE_GP
      if (! counter--)
         *(uchar*)(0xc0000000) = 5;
#endif
#ifdef INVOKE_PF
      if (! counter--)
         *(uchar*)(0x70000000) = 5;
#endif
   }

   // FIXME: Научится отслеживать завершение процесса.
   // Как я понял, для него просто нужно создать стековый
   // фрейм, как для вызова фунции.
   return 17;
}
