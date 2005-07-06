/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ������� �����/������, � ��� ����� ���������� �����������
 *  �������.
 *
 */


#ifndef __HELLO_STDIO
#define __HELLO_STDIO


#include "types.h"

extern int lines, cols;                // ���������� ����� � ����� �� ������
extern int curr_x,curr_y;              // ������� ��������� �������

// ������������
void scrio_init(int cur_x, int cur_y);

// ����� ������
void puts(const char *s);

// ����� ������, �� ����� n ��������
void nputs(const char *s, uint n);

// ����� ������ � ����������
void puts_color(const char *s, uchar attr);

// ����� ������ � ����������, �� ����� n ��������
void nputs_color(const char *s, uint n, uchar attr);

// ����������� �������
void gotoxy(int x, int y);

// ������������� ������ �� ���� ������ �����
void scroll();

// ������� ������
void clear_screen();

// ����� ������ � hex-����. Size - ������ ������ � ������.
void PrintHex(void *val, uchar size);

// ������� ���� ������
char get_char();

// ������� ������ � ������� ���������� �� �����
void readline(char *s, uint buf_size);


#endif // __HELLO_STDIO
