#include <helloos/user_syscalls.h>

int main()
{
   sys_comvar_set("test_var", 5);

   return 0;
}
