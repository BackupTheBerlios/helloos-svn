#include <helloos/user_syscalls.h>
#include <stdio.h>

#define n   50

DirEntry files[n];

int main()
{
   uint total = sys_dir_load(0, files, n);
   uint count = n < total ? n : total;
   uint i;
   for (i = 0; i < count; i++)
   {
      nputs(files[i].Name, 11);
      if (files[i].Attr & ATTR_HIDDEN)
         puts(" <hid>");
      if (files[i].Attr & ATTR_SYSTEM)
         puts(" <sys>");
      if (files[i].Attr & ATTR_ARCHIVE)
         puts(" <arc>");
      if (files[i].Attr & ATTR_READ_ONLY)
         puts(" <ro>");
      if (files[i].Attr & ATTR_DIRECTORY)
         puts(" <dir>");
      puts("\n");
   }
   if (total > n)
      puts("...\n");

   return 0;
}
