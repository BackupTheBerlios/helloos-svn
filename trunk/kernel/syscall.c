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
#include <helloos/scheduler.h>
#include <helloos/binfmt.h>
#include <helloos/io.h>
#include <helloos/syscall.h>
#include <helloos/ipc.h>
#include <helloos/fat.h>


// Указатель на syscall
typedef uint (*syscall_ptr)();

// Несложно заметить, что все вызовы сделаны только в отладочных
// целях и их потом, разумеется заменим на нормальные и защищенные

uint syscall_exit(uint exitcode);

uint syscall_getpid();

char syscall_getchar(uint x, uint y);
uint syscall_setchar(uint x, uint y, char c);
uint syscall_setattr(uint x, uint y, byte a);

char syscall_getch();

uint syscall_nputs_color(char *s, uint n, uchar attr);

uint syscall_clear_screen();
uint syscall_readline(char *cmd, uint buf_size);

uint syscall_ps();
uint syscall_kill(uint pid);
uint syscall_pages_info(uint pid);

uint syscall_bin_info(char *filename);
uint syscall_bin_load(char *filename, char *arg);

uint syscall_dbg();

uint syscall_screen_info(uint *w, uint *h);

uint syscall_waitpid(uint pid);

// Таблица системных вызовов
syscall_ptr syscall_table[] = {
   (syscall_ptr)syscall_exit,
   (syscall_ptr)syscall_getpid,

   (syscall_ptr)syscall_getchar,
   (syscall_ptr)syscall_setchar,
   (syscall_ptr)syscall_setattr,
   (syscall_ptr)syscall_getch,
   (syscall_ptr)syscall_nputs_color,
   (syscall_ptr)syscall_clear_screen,
   (syscall_ptr)syscall_readline,

   (syscall_ptr)syscall_ps,
   (syscall_ptr)syscall_kill,
   (syscall_ptr)syscall_bin_info,
   (syscall_ptr)syscall_bin_load,
   (syscall_ptr)syscall_pages_info,
   (syscall_ptr)syscall_dbg,

   (syscall_ptr)syscall_screen_info,

   (syscall_ptr)syscall_comvar_init,
   (syscall_ptr)syscall_comvar_get,
   (syscall_ptr)syscall_comvar_set,
   (syscall_ptr)syscall_comvar_add,
   (syscall_ptr)syscall_waitcomvar,
   (syscall_ptr)syscall_ipc_info,

   (syscall_ptr)syscall_waitpid,

   (syscall_ptr)syscall_find_file,
   (syscall_ptr)syscall_file_load,
   (syscall_ptr)syscall_dir_load,
};
// Вычисляемое количество вызовов. Сделано в виде переменной,
// чтобы было возможно обращение из ассемблера.
uint syscall_nr = (sizeof(syscall_table) / sizeof(syscall_ptr));


// Системный вызов syscall_exit
// Его вызывает пользовательский процесс когда завершается
uint syscall_exit(uint exitcode)
{
   printf_color(0x04, "Process %d exits with code %d\n", Task[Current]->pid, exitcode);
   scheduler_kill_current();

   return 0;
}


// Системный вызов getpid
// Возвращает pid текущеко процесса
uint syscall_getpid()
{
   return Task[Current]->pid;
}



// Системный вызов syscall_getnewcharaddr
// Выделяет процессу один символ в видеопамяти и возвращает
// его адрес
uint syscall_getnewcharaddr()
{
   static uint addr = 0xf06; // Выделяем начиная отсюда
   uint res = addr;
   addr += 2;  // Переходим на следующий
   return res;
}

// Системный вызов syscall_incvideochar
// Увеличивает на 1 значения байта в видеопамяти
// Возвращает 0
uint syscall_incvideochar(uint addr)
{
   (*(uchar*)(0xb8000+addr))++;
   return 0;
}

// Системный вызов syscall_nputs_color
// Предоставляет процессам функцию nputs_color
// Параметры те же, что и обычно. Строка не длиннее 255 символов.
// Возвращает количество выведенных символов.
//
// FIXME: Нужно проверять наличие доступа к пользовательской
// памяти. Предупреждать GP, PF (в смысле выхода за выделенную память.
uint syscall_nputs_color(char *s, uint n, uchar attr)
{
   uchar localbuf[256];
   memcpy_from_user(localbuf, s, MIN(n, 256));
   nputs_color(localbuf, MIN(n, 256), attr);
   return MIN(n, 256);
}


// Системный вызов syscall_getch
// Ожидает нажатия клавиши и возвращает ее ascii-код
char syscall_getch()
{
   uchar cp, c = inb(0x60);
   char ascii;
   do
   {
      cp = c;
      c = inb(0x60);
   } while ((c == '\0') || (c == cp) || ((ascii = scan2ascii(c)) == '\0'));
   return scan2ascii(c);
}



uint syscall_clear_screen()
{
   clear_screen();
   return 0;
}

uint syscall_readline(char *cmd, uint buf_size)
{
   char localbuf[256];
   readline(localbuf, 256);
   localbuf[buf_size] = 0;
   memcpy_to_user(cmd, localbuf, buf_size);
   return 0;
}

uint syscall_ps()
{
   scheduler_ps();
   return 0;
}

uint syscall_kill(uint pid)
{
   scheduler_kill(pid);
   return 0;
}

uint syscall_bin_info(char *filename)
{
   char localbuf[256];
   strncpy_from_user(localbuf, filename, 256);
   if (!bin_dump_info(localbuf))
      printf("Cannot read binary\n");
   return 0;
}

uint syscall_bin_load(char *filename, char *arg)
{
   char localbuf[256];
   char locarg[256];
   strncpy_from_user(localbuf, filename, 256);
   strncpy_from_user(locarg,   arg,      256);
   uint pid = bin_load_bin(localbuf, locarg);
   if (pid == (uint)-1)
      printf_color(0x04, "Cannot load binary\n");
   return pid;
}

uint syscall_pages_info(uint pid)
{
   scheduler_pages(pid);
   return 0;
}

uint syscall_dbg()
{
   outw(0x8A00, 0x8A00);
   outw(0x8AE0, 0x8A00);
   return 0;
}

uint syscall_waitpid(uint pid)
{
   uint p;
   for (p = 0; p < NTasks; p++)
      if (Task[p]->pid == pid)
         goto ok;
   return 0;

ok:
   Task[Current]->waitfor.pid = pid;
   Task[Current]->state = PS_WAITPID;
   CALL_SCHEDULER;
   return 1;
}
