#include <helloos/types.h>
#include <helloos/user_syscalls.h>

int bss;
int data = 10000;

int main()
{
   uint myaddr = mysys_getnewcharaddr();

   uint counter = 100000;
   while (counter--)
   {
      mysys_incvideochar(myaddr);
   }

   return 17;
}
