/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Библиотека стандартных функций ввода-вывода
 *  для пользовательских программ
 *
 */


#include <helloos/types.h>
#include <helloos/user_syscalls.h>
#include <stdarg.h>
#include <string.h>

// Включаем код vsnprintf и др.
#include "stdio.inc"

// Тупая реализация стандартного vprintf с поддержкой цвета
// Используется vsnprintf, а затем puts
// Может выводить НЕ БОЛЕЕ 256(-1) символов
// Цвет просто передается в puts_color
int vprintf_color(uchar attr, char *format, va_list curp)
{
   char Buffer[256];
   uint res = vsnprintf(Buffer, 256, format, curp);
   sys_nputs_color(Buffer, 256, attr);
   return res;
}

// Реализация стандартного vprintf через vprintf_color
int vprintf(char *format, va_list curp)
{
   return vprintf_color(0x0e, format, curp);
}

// Цветной printf через vprintf_color
int printf_color(uchar attr, char *format, ...)
{
   va_list curp;
   uint res;
   va_start(curp, format);
   res = vprintf_color(attr, format, curp);
   va_end(curp);
   return res;
}

// Реализация стандартного printf через vprintf_color
int printf(char *format, ...)
{
   va_list curp;
   uint res;
   va_start(curp, format);
   res = vprintf_color(0x0e, format, curp);
   va_end(curp);
   return res;
}



// Вывод с цветом без ограничения длины
void puts_color(const char *s, uchar attr)
{
   sys_nputs_color(s, -1, attr);
}


// Вывод без цвета без длины
void puts(const char *s)
{
   sys_nputs_color(s, -1, 0x0e);
}


// Вывод без цвета, с ограничением длины
void nputs(const char *s, uint n)
{
   sys_nputs_color(s, n, 0x0e);
}

uchar CharTo83(uchar c)
{
   // Заменяем запрещенные символы на подчеркивание
   // Запрещенными являются:
   //  - меньшие 0x20
   //  - " * + , / : ; < = > ? [ \ ] |
   //  Заодно я запрещаю использование второй половины таблицы ascii (пока ;)
   if (c<0x20 || c==0x22 || c==0x2a || c==0x2b || c==0x2c || c==0x2e || c==0x2f || (c>=0x3a && c<=0x3f) || c==0x5b || c==0x5c || c==0x5d || c >= 0x80)
      c = '_';

   // Приводим к верхнему регистру
   if (c >= 0x61 && c <= 0x7a) c -= 0x20;

   return c;
}

void Make83Name(char *fullname, char *name83)
{
   int len = 0, i, j;

   // Предварительно заполняем пробелами
   for (i = 0; i < 11; i++) name83[i] = 0x20;

   // Отдельно обрабатываем . и ..
   if (strcmp(fullname, ".") == 0)
   {
      name83[0] = '.';
      return;
   }
   if (strcmp(fullname, "..") == 0)
   {
      name83[0] = '.'; name83[1] = '.';
      return;
   }

   // Выбираем 8 символов для имени
   i = 0;
   while ((len <= 8)  &&  (i < strlen(fullname))  &&  (fullname[i] != '.'))
      name83[len++] = CharTo83(fullname[i++]);

   // Ищем точку
   for (i = 0; i < strlen(fullname); i++) if (fullname[i] == '.') break;

   // Если нашли - набираем расширение
   if (i < strlen(fullname))
   {
      j = 8;
      i++;
      while ((j < 11)  &&  (i < strlen(fullname)))
         name83[j++] = CharTo83(fullname[i++]);
   }

   // Первый символ не может быть пробелом (например, для ".foo")
   if (name83[0] == 0x20)
      name83[0] = '_';
}
