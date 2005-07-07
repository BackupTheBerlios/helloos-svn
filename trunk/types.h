/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Сокращенные определения часто используемых
 *  типов
 *
 */


#ifndef __TYPES_H
#define __TYPES_H



// Полезные shortcut'ы
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned int    ulong;
typedef unsigned int    size_t;
typedef uchar bool;




// Структура для одного дескриптора в системных
// таблицах типа GDT.
typedef struct _Descriptor Descriptor;
struct _Descriptor
{
   ulong a, b;
};

// Эта структура используется для вызова инструкций sgdt/lgdt
typedef struct _GDTDescriptor GDTDescriptor;
struct _GDTDescriptor
{
   ushort Size; // Размер GDT - 1
   Descriptor *Addr; // Адрес GDT
} __attribute__((packed));


#endif // __TYPES_H
