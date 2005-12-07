#include <helloos/user_syscalls.h>
#include <helloos/fat.h>
#include <stdio.h>
#include <string.h>

#define size   4096

char BUF[size];

int main(int argc, char *argv[])
{
   if (argc < 1)
   {
      printf("arg required\n");
      return 1;
   }

   char name83[11];
   Make83Name(argv[0], name83);

   DirEntry file;
   if (sys_find_file(0, name83, &file) != (uint)-1)
   {
      uint l;
      FileChunk chunk = {0, size};
      do
      {
         l = sys_file_load(&file, BUF, &chunk);
         nputs(BUF, l);
         chunk.start += size;
      } while (l == size);
   }
   else
      printf("File '%s' not found\n", argv[0]);

   return 0;
}
