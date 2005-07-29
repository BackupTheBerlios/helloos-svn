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


// Макросы для объявления клиентских заглушек к
// системным вызовам

#define SYSCALL0(name, idx, restype)   \
extern inline restype mysys_##name()    \
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx));         \
   return res;                         \
}

#define SYSCALL1(name, idx, restype, t1)\
extern inline restype mysys_##name(t1 a1)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1));\
   return res;                         \
}

#define SYSCALL2(name, idx, restype, t1, t2)\
extern inline restype mysys_##name(t1 a1, t2 a2)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1), "c"(a2));\
   return res;                         \
}


#define SYSCALL3(name, idx, restype, t1, t2, t3)\
extern inline restype mysys_##name(t1 a1, t2 a2, t3 a3)\
{                                      \
   restype res;                        \
   __asm__ volatile("int $0x80\n"      \
         :"=a"(res):"0"(idx), "b"(a1), "c"(a2), "d"(a3));\
   return res;                         \
}

// Несложно заметить, что все вызовы сделаны только в отладочных
// целях и их потом, разумеется заменим на нормальные и защищенные

SYSCALL1(exit,             0,    uint, uint)
SYSCALL0(getnewcharaddr,   1,    uint)
SYSCALL1(incvideochar,     2,    uint, uint)
SYSCALL3(nputs_color,      3,    uint, const char*, uint, uchar)
SYSCALL0(clear_screen,     4,    uint)
SYSCALL2(readline,         5,    uint, char*, uint)
SYSCALL1(panic,            6,    uint, char*)
SYSCALL0(ps,               7,    uint)
SYSCALL1(kill,             8,    uint, uint)
SYSCALL1(bin_info,         9,    uint, char*)
SYSCALL1(bin_load,         10,    uint, char*)
SYSCALL1(pages,            11,   uint, uint)
SYSCALL0(dbg,              12,   uint)

#endif // __USER_SYSCALLS_H
