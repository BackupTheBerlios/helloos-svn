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


#include <helloos/types.h>
#include <helloos/scrio.h>
#include <helloos/panic.h>
#include <helloos/scheduler.h>
#include <helloos/binfmt.h>
#include <helloos/io.h>


// Указатель на syscall
typedef uint (*syscall_ptr)();

// Несложно заметить, что все вызовы сделаны только в отладочных
// целях и их потом, разумеется заменим на нормальные и защищенные

uint sys_exit(uint exitcode);

uint sys_getnewcharaddr();
uint sys_incvideochar(uint addr);
uint sys_nputs_color(char *s, uint n, uchar attr);

uint sys_clear_screen();
uint sys_readline(char *cmd, uint buf_size);

uint sys_panic(char *msg);

uint sys_ps();
uint sys_kill(uint pid);
uint sys_pages(uint pid);

uint sys_bin_info(char *filename);
uint sys_bin_load(char *filename);

uint sys_dbg();

// Таблица системных вызовов
syscall_ptr syscall_table[] = {
   (syscall_ptr)sys_exit,
   (syscall_ptr)sys_getnewcharaddr,
   (syscall_ptr)sys_incvideochar,
   (syscall_ptr)sys_nputs_color,
   (syscall_ptr)sys_clear_screen,
   (syscall_ptr)sys_readline,
   (syscall_ptr)sys_panic,
   (syscall_ptr)sys_ps,
   (syscall_ptr)sys_kill,
   (syscall_ptr)sys_bin_info,
   (syscall_ptr)sys_bin_load,
   (syscall_ptr)sys_pages,
   (syscall_ptr)sys_dbg,
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
         "push %%ds\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


inline void memcpy_from_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%ds\n"
         "push %%gs\n"
         "pop %%ds\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%ds\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


inline void memcpy_to_user(void *dest, void *src, uint n)
{
   __asm__(
         "push %%es\n"
         "push %%gs\n"
         "pop %%es\n"
         "movl %0, %%edi\n"
         "movl %1, %%esi\n"
         "cld\n"
         "rep movsb\n"
         "pop %%es\n"
         ::"m"(dest), "m"(src), "c"(n):"di", "si");
}


// Системный вызов sys_exit
// Его вызывает пользовательский процесс когда завершается
uint sys_exit(uint exitcode)
{
   printf_color(0x04, "Process %d exits with code %d\n", Task[Current]->pid, exitcode);
   scheduler_kill_current();

   return 0;
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
   memcpy_from_user(localbuf, s, MIN(n, 256));
   nputs_color(localbuf, MIN(n, 256), attr);
   return MIN(n, 256);
}



uint sys_clear_screen()
{
   clear_screen();
   return 0;
}

uint sys_readline(char *cmd, uint buf_size)
{
   char localbuf[256];
   readline(localbuf, 256);
   localbuf[buf_size] = 0;
   memcpy_to_user(cmd, localbuf, buf_size);
   return 0;
}

uint sys_panic(char *msg)
{
   char localbuf[256];
   strncpy_from_user(localbuf, msg, 256);
   panic(localbuf);
   return 0;
}

uint sys_ps()
{
   scheduler_ps();
   return 0;
}

uint sys_kill(uint pid)
{
   scheduler_kill(pid);
   return 0;
}

uint sys_bin_info(char *filename)
{
   char localbuf[256];
   strncpy_from_user(localbuf, filename, 256);
   if (!bin_dump_info(localbuf))
      printf("Cannot read binary\n");
   return 0;
}

uint sys_bin_load(char *filename)
{
   char localbuf[256];
   strncpy_from_user(localbuf, filename, 256);
   if (!bin_load_bin(localbuf))
      printf("Cannot load binary\n");
   return 0;
}

uint sys_pages(uint pid)
{
   scheduler_pages(pid);
   return 0;
}

uint sys_dbg()
{
   outw(0x8A00, 0x8A00);
   outw(0x8AE0, 0x8A00);
   return 0;
}
