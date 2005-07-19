/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ��������� ������
 *
 *  ���� ������ �������� ������
 *
 */


#include "types.h"
#include "hello_stdio.h"


// ��������� �� syscall
typedef uint (*syscall_ptr)();


uint sys_getnewcharaddr();
uint sys_incvideochar(uint addr);
uint sys_nputs_color(char *s, uint n, uchar attr);

// ������� ��������� �������
syscall_ptr syscall_table[] = {
   (syscall_ptr)sys_getnewcharaddr,
   (syscall_ptr)sys_incvideochar,
   (syscall_ptr)sys_nputs_color,
};
// ����������� ���������� �������. ������� � ���� ����������,
// ����� ���� �������� ��������� �� ����������.
uint syscall_nr = (sizeof(syscall_table) / sizeof(syscall_ptr));


// �� ����� ���������� ������ ��� ���������� �������� ����� GS
// ������������� �� �������� ����. ��� ������� �������� ������
// �� ���������������� ������ � ��������� ����� GS.
//
// FIXME: � ����� ��� ����������� ������ �����. ������������
// ���� - ��� ������������ � ��� ������
inline void strncpy_from_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%es\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


// ��������� ����� sys_getnewcharaddr
// �������� �������� ���� ������ � ����������� � ����������
// ��� �����
uint sys_getnewcharaddr()
{
   static uint addr = 0xf06; // �������� ������� ������
   uint res = addr;
   addr += 2;  // ��������� �� ���������
   return res;
}

// ��������� ����� sys_incvideochar
// ����������� �� 1 �������� ����� � �����������
// ���������� 0
uint sys_incvideochar(uint addr)
{
   (*(uchar*)(0xb8000+addr))++;
   return 0;
}

// ��������� ����� sys_nputs_color
// ������������� ��������� ������� nputs_color
// ��������� �� ��, ��� � ������. ������ �� ������� 255 ��������.
// ���������� ���������� ���������� ��������.
//
// FIXME: ����� ��������� ������� ������� � ����������������
// ������. ������������� GP, PF (� ������ ������ �� ���������� ������.
uint sys_nputs_color(char *s, uint n, uchar attr)
{
   uchar localbuf[256];
   strncpy_from_user(localbuf, s, MIN(n, 256));
   nputs_color(localbuf, MIN(n, 256), attr);
   return MIN(n, 256);
}
