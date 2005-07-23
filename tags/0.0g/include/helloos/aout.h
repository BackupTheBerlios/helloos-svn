/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ����������� ��� ������ � �����������
 *  � ������� a.out
 *
 *  ��. FIXME � aout.c
 *
 */


#ifndef __AOUT_H
#define __AOUT_H

#include <helloos/types.h>


typedef struct
{
   uint a_midmag; // �����
   uint a_text;   // ������ ������ ����
   uint a_data;   // ������ ������ ������������������ ������
   uint a_bss;    // ������ ������ �������������������� ������
   uint a_syms;   // ������ ������ ��������
   uint a_entry;  // ����� �����
   uint a_trsize; // ������ ������ ����������� ������� ��� ����
   uint a_drsize; // ������ ������ ����������� ������� ��� ������
} Exec;

// ��������� �����
#define N_FLAG(exec) ((exec).a_midmag >> 24)

// ��������� �����������
#define N_MID(exec) (((exec).a_midmag>>16)&0xff)
#define M_OLDSUN2    0
#define M_68010      1
#define M_68020      2
#define M_SPARC      3
#define M_386        100
#define M_MIPS1      151
#define M_MIPS2      152

// ��������� ��� ���������
#define N_MAGIC(exec) ((exec).a_midmag & 0xffff)
#define OMAGIC       0407
#define NMAGIC       0410
#define ZMAGIC       0413
#define QMAGIC       0314
#define CMAGIC       0421

// ��� ��������������
#define N_TRSIZE(a)	((a).a_trsize)
#define N_DRSIZE(a)	((a).a_drsize)
#define N_SYMSIZE(a)	((a).a_syms)

// ���� �� ������������
#define N_BADMAG(x) \
  (N_MAGIC(x) != OMAGIC	&& N_MAGIC(x) != NMAGIC				\
   && N_MAGIC(x) != ZMAGIC && N_MAGIC(x) != QMAGIC)

#define _N_HDROFF(x)	(1024 - sizeof(Exec))
   /*
#define N_TXTOFF(x) \
  (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x)) + sizeof(Exec) :	\
   (N_MAGIC(x) == QMAGIC ? 0 : sizeof (Exec)))
   */

// �������� ������ � �����
// ��. FIXME #1 � aout.c
#define N_TXTOFF(x) \
   (N_MAGIC(x) == QMAGIC ? 0 : sizeof (Exec))
#define N_DATOFF(x)	(N_TXTOFF(x) + (x).a_text)
#define N_TRELOFF(x)	(N_DATOFF(x) + (x).a_data)
#define N_DRELOFF(x)	(N_TRELOFF(x) + N_TRSIZE(x))
#define N_SYMOFF(x)	(N_DRELOFF(x) + N_DRSIZE(x))
#define N_STROFF(x)	(N_SYMOFF(x) + N_SYMSIZE(x))


// ��������� ����������� ������. ���� �� ��������������.
typedef struct
{
   uint r_address;
   uint r_symbolnum  :24;
   uint r_pcrel      :1;
   uint r_length     :2;
   uint r_extern     :1;
   uint r_baserel    :1;
   uint r_jmptable   :1;
   uint r_relative   :1;
   uint r_copy       :1;
} Relocation_Info;




// ���������� ���������
void aout_info(char *Name);
// ���������
void aout_load(char *Name);



#endif // __AOUT_H
