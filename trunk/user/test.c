/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ���������������� ��������. �������������
 *  � ������ ELF � ����� ���� ������� �����.
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

   // FIXME: �������� ����������� ���������� ��������.
   // ��� � �����, ��� ���� ������ ����� ������� ��������
   // �����, ��� ��� ������ ������.
   return 17;
}
