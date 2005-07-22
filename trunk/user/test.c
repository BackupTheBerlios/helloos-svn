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


#include <helloos/types.h>
#include <helloos/user_syscalls.h>

// Заглушки системных вызовов. Разумеется,
// потом они будут в пользовательских заголовках.

int main()
{
   // Просим себе символ на экране
   uint myaddr = mysys_getnewcharaddr();
//   mysys_nputs_color("I'm alive!\n", -1, 0xc);

   // И мигаем им до посинения
#define INVOKE_PF
#ifdef INVOKE_GP
   uint gp_counter = 10000;
#endif
#ifdef INVOKE_PF
   uint pf_counter = 10000;
#endif
   while (1)
   {
      mysys_incvideochar(myaddr);
#ifdef INVOKE_GP
      if (! gp_counter--)
         *(uchar*)(0xc0000000) = 5;
#endif
#ifdef INVOKE_PF
      if (! pf_counter--)
         *(uchar*)(0x70000000) = 5;
#endif
   }

   // FIXME: Научится отслеживать завершение процесса.
   // Как я понял, для него просто нужно создать стековый
   // фрейм, как для вызова фунции.
   return 17;
}
