/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции ввода/вывода, в том числе реализации стандартных
 *  функций.
 *
 */


#ifndef __HELLO_STDIO_H
#define __HELLO_STDIO_H


#include "types.h"
#include "hello_stdarg.h"

#include "colors.h"


// Иницилизация
void scrio_init(int cur_x, int cur_y);

// Вывод строки
void puts(const char *s);

// Вывод строки, не более n символов
void nputs(const char *s, uint n);

// Вывод строки с аттрибутом
void puts_color(const char *s, uchar attr);

// Вывод строки с аттрибутом, не более n символов
void nputs_color(const char *s, uint n, uchar attr);

// Перемещение курсора
void gotoxy(int x, int y);

// Прокручивание экрана на одну строку вверх
void scroll();

// Очистка экрана
void clear_screen();

// Вывод байтов в hex-виде. Size - размер данных в байтах.
void PrintHex(void *val, uchar size);

// Считать строку с выводом введенного на экран
void readline(char *s, uint buf_size);

// Реализация стандартного vsnprintf'а. За подробностями см. hello_stdio.c 
int vsnprintf(char *str, size_t maxlen, const char *format, va_list curp);

// Реализация стандартного snprintf'а. За подробностями см. hello_stdio.c
int snprintf(char *str, size_t maxlen, const char *format, ...);

// Реализация стандартного printf'а. За подробностями см. hello_stdio.c
int printf(char *format, ...);

// Цветной printf
int printf_color(uchar attr, char *format, ...);

#endif // __HELLO_STDIO_H
