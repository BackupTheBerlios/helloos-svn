/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Заголовок драйвера флоппи-дисковода
 *
 */


#ifndef __FD_H
#define __FD_H


#include "types.h"


// Константы, определяющие адреса портов флоппи-контроллера
// см. [6] и [7].

// Main Status Register
#define FD_MSR   0x3f4

// Digital Output Register
#define FD_DOR   0x3f2

// FIFO, очередь команд и данных
#define FD_FIFO  0x3f5

extern uint fd_cache_pos;
// Иницилизация драйвера и контроллера
void fd_init();

// Запись сектора
void fd_write_sector(uint lba, uchar *buf);

// Чтение сектора
void fd_read_sector(uint lba, uchar *buf);


#endif // __FD_H
