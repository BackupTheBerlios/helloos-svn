#include <helloos/types.h>
#include <helloos/user_syscalls.h>

int bss;
int data = 10000;

int main()
{
   uint myaddr = mysys_getnewcharaddr();

   while (1)
   {
      mysys_incvideochar(myaddr);
   }

   return 17;
}
