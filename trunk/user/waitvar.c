#include <stdio.h>
#include <helloos/user_syscalls.h>

int main()
{
   sys_waitcomvar("test_var", 5);

   return 0;
}
