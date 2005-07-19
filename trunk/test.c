/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ���������������� ��������. �������������
 *  � ������ a.out � ����� ���� ������� �����.
 *
 */


#include "types.h"

// �������� ��������� �������. ����������,
// ����� ��� ����� � ���������������� ����������.

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
   // ������ ���� ������ �� ������
   uint myaddr = mysys_getnewcharaddr();
//   mysys_nputs_color("I'm alive!\n", -1, 0xc);

   // � ������ �� �� ���������
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

   // FIXME: �������� ����������� ���������� ��������.
   // ��� � �����, ��� ���� ������ ����� ������� ��������
   // �����, ��� ��� ������ ������.
   return 17;
}
