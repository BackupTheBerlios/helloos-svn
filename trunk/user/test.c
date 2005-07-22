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


#include <helloos/types.h>
#include <helloos/user_syscalls.h>

// �������� ��������� �������. ����������,
// ����� ��� ����� � ���������������� ����������.

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
