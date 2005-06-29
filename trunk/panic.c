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

#include "types.h"
#include "hello_stdio.h"

void panic(char *msg)
{
   puts("\nKernel panic!\nOops: ");
   puts(msg);
   asm("cli; hlt");
}
