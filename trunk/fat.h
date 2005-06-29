/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ��������� �������� FAT
 *
 */


#ifndef __FAT_H
#define __FAT_H


#include "types.h"


// ���������� �������� ���������� �� �����
// ��������.
struct _DirEntry
{
   uchar    Name[11];      // 11
   uchar    Attr;          // 1
   uchar    NTRes;         // 1
   uchar    CrtTimeTenth;  // 1
   ushort   CrtTime;       // 2
   ushort   CrtDate;       // 2
   ushort   LstAccDate;    // 2
   ushort   FstClusHI;     // 2
   ushort   WrtTime;       // 2
   ushort   WrtDate;       // 2
   ushort   FstClusLO;     // 2
   ulong    FileSize;      // 4
} __attribute__((packed));
typedef struct _DirEntry DirEntry;


// �������� ������ � ���������
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0x0f


// ��� callback-������� ��� ������������� ��������
// �� ���������� DirEntry � ���������������� ������
// ���� callback-������� ���������� 0, �������������
// ������������.
typedef bool (*DirCallback)(DirEntry* Entry, void* Data);

// ��� callback-������� ��� ������������� �����
// ���������� ���� ������, ��� ����� � ���������������� ������
// ���� callback-������� ���������� 0, �������������
// ������������.
typedef bool (*FileCallback)(uchar *Block, ulong len, void *Data);



// ���������� ������� � ������� �������� ���������
void fat_main();


// ������������ ��������
void fat_init();


// ������������� ��������� ��������.
// �� �� �����, ��� DirIterate � ������ ���������� 0.
void RootDirIterate(DirCallback, void *Data);


// ������������� ��������
// ������� �������� ����� ������ ��������� ;)
// �� ������ ������ � �������� ���������� ������� Callback,
// ������� ���������� ��������� �� ������ � �������� Data.
// ����� �������� �������� Data �� ������������.
// �� ��������� �������� �������� ������ �� callback-�������,
// ��������� �� ����������.
void DirIterate(ulong Cluster, DirCallback Callback, void *Data);


// ����� �� ����� ����������� ��������. ���������� DirIterate.
void ListDir(ulong Cluster);


// ������������� �����. ���� �������� ����� ������� � ��������.
// ��� ������ ������ Callback �� ���������� ��������� �� ���������
// ����� ������ (������ - ��������� �������) � ��� ������.
// �� ��������� �������� ������ �� callback-�������.
void FileIterate(DirEntry *Entry, FileCallback Callback, void *Data);


// �������� ���� �� �����
void PrintFile(DirEntry *Entry);


// ������� �������� ����� � ��� ���� 8.3 � ����������� ���������,
// ��� ��� ������� � [4] ��� ������� ����� � ������� ���������.
// ���� �� ��� ������� ����-�� �������, ����� ������ ��������.
// ����� name83 ������ ���� �� ������ 11 ����.
void Make83Name(char *fullname, char *name83);


// ���� ���� � ������ ��������
// ���������� ������ ������� ���������� �����/��������
// ��� ������� ���������� -1
// ���� �������� EntryBuf != 0, �� � ���� ������������ DirEntry
// ���������� �����/��������
ulong FindEntry(ulong DirCluster, char *Name83, DirEntry *EntryBuf);


#endif // __FAT_H
