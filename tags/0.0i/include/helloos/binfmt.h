/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ��������� ��� ������ � ��������� �������
 *
 */


#ifndef __BINFMT_H
#define __BINFMT_H

#include <helloos/types.h>


// ��������� ��������� ������� �������� ������
typedef struct
{
   char *FormatName;    // �������� �������
   bool (*is)(char*);   // �������, ������������ ��������� �� ������ ����
                        // � ����� �������
   void (*dump_info)(char*);  // ������� ������ ���������� ����� �������
   void (*load_bin)(char*);   // ������� ������� ������������ �����
   addr_t (*load_page)(uint); // �������, ����������� �������������
                              // �������� (���������� �� #PF)
} BinFmt;


// ���������, ������������ �������� �������
// ��� ������ ��������� ��������������� ������� ������� BinFormats
// ��������� ��������������� ������
#define BIN_AOUT     0
#define BIN_ELF      1

// ���������� �������� / ������� � ������� BinFormats
#define BIN_N        2

extern BinFmt BinFormats[BIN_N];


// ���������� ������ ����� (-1 ��� �������)
int bin_type(char *name);
// ���������� ��������� ����� (0 ��� �������)
bool bin_dump_info(char *name);
// ��������� ���� (0 ��� �������)
bool bin_load_bin(char *name);



#endif // __BINFMT_H
