/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Заголовки для средств межпроцессорного
 *  взаимодействия
 *
 */


#ifndef __IPC_H
#define __IPC_H


#include <helloos/types.h>


#define MAX_VAR_NAME    24


void ipc_init();

uint syscall_comvar_init(char *name, uint val);
uint syscall_comvar_get(char *name);
uint syscall_comvar_set(char *name, uint val);
uint syscall_comvar_add(char *name, uint incr);

uint syscall_waitcomvar(char *name, uint val);

uint syscall_ipc_info();

#endif // __IPC_H
