/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  �������, ���������� ��� ��������� ������
 *
 *  ���� ����� �� [5]. ���, �������, ������ �� �����
 *  ������ ������ ������ ��������, �� �� ������ �����
 *  � � ����� ������� ������.
 *
 *  ������� ������� �� ����� ��������� �� ������ �
 *  ��������� ���������� � ���������.
 *
 */

#include <helloos/types.h>
#include <helloos/scrio.h>

void panic(char *msg)
{
   puts_color("\nKernel panic!\nOops: ", 0xc);
   puts_color(msg, 0x0c);
   asm("cli; hlt");
   while(1);
}
