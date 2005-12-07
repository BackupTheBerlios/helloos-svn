/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Заголовок для стандартных пользовательских
 *  функций ввода-вывода
 *
 */

#ifndef __STDIO_H
#define __STDIO_H


#include <helloos/types.h>
#include <stdarg.h>
#include <string.h>


// Terminal I/O
int vsnprintf(char *str, size_t maxlen, const char *format, va_list curp);
int snprintf(char *str, size_t maxlen, const char *format, ...)
   __attribute__((format(printf, 3, 4)));
int vprintf_color(uchar attr, char *format, va_list curp);
int vprintf(char *format, va_list curp);
int printf_color(uchar attr, char *format, ...)
   __attribute__((format(printf, 2, 3)));
int printf(char *format, ...)
   __attribute__((format(printf, 1, 2)));
void puts_color(const char *s, uchar attr);
void puts(const char *s);
void nputs(const char *s, uint n);


// File I/O
// Атрибуты файлов и каталогов
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0x0f


typedef struct
{
   uint start;
   uint length;
} FileChunk;


void Make83Name(char *fullname, char *name83);

#endif // __STDIO_H
