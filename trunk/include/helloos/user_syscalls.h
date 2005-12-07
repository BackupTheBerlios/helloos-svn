/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Пользовательские заглушки для системных
 *  вызовов
 *
 */


#ifndef __USER_SYSCALLS_H
#define __USER_SYSCALLS_H

#include <helloos/types.h>
#include <helloos/fat.h>


// Макросы для объявления клиентских заглушек к
// системным вызовам

#define SYSCALL0(name, idx, restype)   \
extern inline restype sys_##name()    \
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx));         \
   return res;                         \
}

#define SYSCALL1(name, idx, restype, t1)\
extern inline restype sys_##name(t1 a1)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1));\
   return res;                         \
}

#define SYSCALL2(name, idx, restype, t1, t2)\
extern inline restype sys_##name(t1 a1, t2 a2)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1), "c"(a2));\
   return res;                         \
}


#define SYSCALL3(name, idx, restype, t1, t2, t3)\
extern inline restype sys_##name(t1 a1, t2 a2, t3 a3)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1), "c"(a2), "d"(a3));\
   return res;                         \
}

// Несложно заметить, что все вызовы сделаны только в отладочных
// целях и их потом, разумеется заменим на нормальные и защищенные

SYSCALL1(exit,             0,    uint, uint)
SYSCALL0(getpid,           1,    uint)
SYSCALL2(getchar,          2,    char, uint, uint)
SYSCALL3(setchar,          3,    uint, uint, uint, char)
SYSCALL3(setattr,          4,    uint, uint, uint, byte)
SYSCALL0(getch,            5,    char)
SYSCALL3(nputs_color,      6,    uint, const char*, uint, uchar)
SYSCALL0(clear_screen,     7,    uint)
SYSCALL2(readline,         8,    uint, char*, uint)
SYSCALL0(ps,               9,    uint)
SYSCALL1(kill,             10,    uint, uint)
SYSCALL1(bin_info,         11,   uint, char*)
SYSCALL2(bin_load,         12,   uint, char*, char*)
SYSCALL1(pages_info,       13,   uint, uint)
SYSCALL0(dbg,              14,   uint)
SYSCALL2(screen_info,      15,   uint, uint*, uint*)
SYSCALL2(comvar_init,      16,   uint, char*, uint)
SYSCALL1(comvar_get,       17,   uint, char*)
SYSCALL2(comvar_set,       18,   uint, char*, uint)
SYSCALL2(comvar_add,       19,   uint, char*, uint)
SYSCALL2(waitcomvar,       20,   uint, char*, uint)
SYSCALL0(ipc_info,         21,   uint)
SYSCALL1(waitpid,          22,   uint, uint)
SYSCALL3(find_file,        23,   uint, uint, char*, DirEntry*)
SYSCALL3(file_load,        24,   uint, DirEntry*, byte*, FileChunk*)
SYSCALL3(dir_load,         25,   uint, uint, DirEntry*, uint)

#endif // __USER_SYSCALLS_H
