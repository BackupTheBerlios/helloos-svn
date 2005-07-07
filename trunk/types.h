/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ����������� ����������� ����� ������������
 *  �����
 *
 */


#ifndef __TYPES_H
#define __TYPES_H



// �������� shortcut'�
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned int    ulong;
typedef unsigned int    size_t;
typedef uchar bool;




// ��������� ��� ������ ����������� � ���������
// �������� ���� GDT.
typedef struct _Descriptor Descriptor;
struct _Descriptor
{
   ulong a, b;
};

// ��� ��������� ������������ ��� ������ ���������� sgdt/lgdt
typedef struct _GDTDescriptor GDTDescriptor;
struct _GDTDescriptor
{
   ushort Size; // ������ GDT - 1
   Descriptor *Addr; // ����� GDT
} __attribute__((packed));


#endif // __TYPES_H
