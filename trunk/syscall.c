/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Системные вызовы
 *
 *  Пока только тестовые вызовы
 *
 */


#include "types.h"
#include "hello_stdio.h"


// Указатель на syscall
typedef uint (*syscall_ptr)();


uint sys_getnewcharaddr();
uint sys_incvideochar(uint addr);
uint sys_nputs_color(char *s, uint n, uchar attr);

// Таблица системных вызовов
syscall_ptr syscall_table[] = {
   (syscall_ptr)sys_getnewcharaddr,
   (syscall_ptr)sys_incvideochar,
   (syscall_ptr)sys_nputs_color,
};
// Вычисляемое количество вызовов. Сделано в виде переменной,
// чтобы было возможно обращение из ассемблера.
uint syscall_nr = (sizeof(syscall_table) / sizeof(syscall_ptr));


// Во время системного вызова все сегментные регистры кроме GS
// переключаются на сегменты ядра. Эта функция копирует строку
// из пользовательской памяти в системную через GS.
//
// FIXME: я забыл про ограничение строки нулем. Переделывать
// лень - для демонстрации и так пойдет
inline void strncpy_from_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%es\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


// Системный вызов sys_getnewcharaddr
// Выделяет процессу один символ в видеопамяти и возвращает
// его адрес
uint sys_getnewcharaddr()
{
   static uint addr = 0xf06; // Выделяем начиная отсюда
   uint res = addr;
   addr += 2;  // Переходим на следующий
   return res;
}

// Системный вызов sys_incvideochar
// Увеличивает на 1 значения байта в видеопамяти
// Возвращает 0
uint sys_incvideochar(uint addr)
{
   (*(uchar*)(0xb8000+addr))++;
   return 0;
}

// Системный вызов sys_nputs_color
// Предоставляет процессам функцию nputs_color
// Параметры те же, что и обычно. Строка не длиннее 255 символов.
// Возвращает количество выведенных символов.
//
// FIXME: Нужно проверять наличие доступа к пользовательской
// памяти. Предупреждать GP, PF (в смысле выхода за выделенную память.
uint sys_nputs_color(char *s, uint n, uchar attr)
{
   uchar localbuf[256];
   strncpy_from_user(localbuf, s, MIN(n, 256));
   nputs_color(localbuf, MIN(n, 256), attr);
   return MIN(n, 256);
}
