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


#ifndef __HELLO_STDIO
#define __HELLO_STDIO


#include "types.h"

extern int lines, cols;                // количество линий и строк на экране
extern int curr_x,curr_y;              // текущее положение курсора

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

// Считать один символ
char get_char();

// Считать строку с выводом введенного на экран
void readline(char *s, uint buf_size);


#endif // __HELLO_STDIO
