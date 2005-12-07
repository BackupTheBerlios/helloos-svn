/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Демонстрационный бинарник. Компилируется
 *  в формат ELF и может быть запущен ядром.
 *
 */


#include <helloos/types.h>
#include <helloos/user_syscalls.h>

int main()
{
   uint myaddr = sys_comvar_init("char pos", 3);
   sys_comvar_add("char pos", +1);
   uint w, h;
   sys_screen_info(&w, &h);

   sys_setattr(myaddr, h, 0x0a);

   // И мигаем им до посинения
   while (1)
   {
      char q = sys_getchar(myaddr, h);
      sys_setchar(myaddr, h, q+1);
   }

   return 17;
}
