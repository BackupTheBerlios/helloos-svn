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


#include <helloos/types.h>
#include <helloos/scrio.h>
#include <helloos/panic.h>
#include <helloos/scheduler.h>
#include <helloos/binfmt.h>
#include <helloos/io.h>


// ��������� �� syscall
typedef uint (*syscall_ptr)();

// �������� ��������, ��� ��� ������ ������� ������ � ����������
// ����� � �� �����, ���������� ������� �� ���������� � ����������

uint sys_exit(uint exitcode);

uint sys_getnewcharaddr();
uint sys_incvideochar(uint addr);
uint sys_nputs_color(char *s, uint n, uchar attr);

uint sys_clear_screen();
uint sys_readline(char *cmd, uint buf_size);

uint sys_panic(char *msg);

uint sys_ps();
uint sys_kill(uint pid);
uint sys_pages(uint pid);

uint sys_bin_info(char *filename);
uint sys_bin_load(char *filename);

uint sys_dbg();

// ������� ��������� �������
syscall_ptr syscall_table[] = {
   (syscall_ptr)sys_exit,
   (syscall_ptr)sys_getnewcharaddr,
   (syscall_ptr)sys_incvideochar,
   (syscall_ptr)sys_nputs_color,
   (syscall_ptr)sys_clear_screen,
   (syscall_ptr)sys_readline,
   (syscall_ptr)sys_panic,
   (syscall_ptr)sys_ps,
   (syscall_ptr)sys_kill,
   (syscall_ptr)sys_bin_info,
   (syscall_ptr)sys_bin_load,
   (syscall_ptr)sys_pages,
   (syscall_ptr)sys_dbg,
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
         "push %%ds\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


inline void memcpy_from_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%ds\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


inline void memcpy_to_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%es\n"
         "push %%gs\n"
         "pop %%es\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%es\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


// ��������� ����� sys_exit
// ��� �������� ���������������� ������� ����� �����������
uint sys_exit(uint exitcode)
{
   printf_color(0x04, "Process %d exits with code %d\n", Task[Current]->pid, exitcode);
   scheduler_kill_current();

   return 0;
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
   memcpy_from_user(localbuf, s, MIN(n, 256));
   nputs_color(localbuf, MIN(n, 256), attr);
   return MIN(n, 256);
}



uint sys_clear_screen()
{
   clear_screen();
   return 0;
}

uint sys_readline(char *cmd, uint buf_size)
{
   char localbuf[256];
   readline(localbuf, 256);
   localbuf[buf_size] = 0;
   memcpy_to_user(cmd, localbuf, buf_size);
   return 0;
}

uint sys_panic(char *msg)
{
   char localbuf[256];
   strncpy_from_user(localbuf, msg, 256);
   panic(localbuf);
   return 0;
}

uint sys_ps()
{
   scheduler_ps();
   return 0;
}

uint sys_kill(uint pid)
{
   scheduler_kill(pid);
   return 0;
}

uint sys_bin_info(char *filename)
{
   char localbuf[256];
   strncpy_from_user(localbuf, filename, 256);
   if (!bin_dump_info(localbuf))
      printf("Cannot read binary\n");
   return 0;
}

uint sys_bin_load(char *filename)
{
   char localbuf[256];
   strncpy_from_user(localbuf, filename, 256);
   if (!bin_load_bin(localbuf))
      printf("Cannot load binary\n");
   return 0;
}

uint sys_pages(uint pid)
{
   scheduler_pages(pid);
   return 0;
}

uint sys_dbg()
{
   outw(0x8A00, 0x8A00);
   outw(0x8AE0, 0x8A00);
   return 0;
}
