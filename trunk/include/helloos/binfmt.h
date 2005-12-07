/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Структуры для работы с бинарными файлами
 *
 */


#ifndef __BINFMT_H
#define __BINFMT_H

#include <helloos/types.h>


// Структура описывает форматы бинарных файлов
typedef struct
{
   char *FormatName;    // Название формата
   bool (*is)(char*);   // Функция, определяющая относится ли данный файл
                        // к этому формату
   void (*dump_info)(char*);  // Функция печати заголовков этого формата
   uint (*load_bin)(char*, char*);   // Функция запуска исполняемого файла
   addr_t (*load_page)(uint); // Функция, загружающая отсутствующую
                              // страницу (вызывается из #PF)
} BinFmt;


// Константы, определяющие двоичные форматы
// Для каждой константы соответствующий элемент массива BinFormats
// описывает соответствующий формат
#define BIN_ELF      0

// Количество форматов / записей в массиве BinFormats
#define BIN_N        1

extern BinFmt BinFormats[BIN_N];


// Определить формат файла (-1 при неудаче)
int bin_type(char *name);
// Напечатать заголовки файла (0 при неудаче)
bool bin_dump_info(char *name);
// Запустить файл
// Возвращает его PID или (uint)-1 в случае неудачи
uint bin_load_bin(char *name, char* args);


typedef struct
{
   ushort index_in_catalog;
   ulong  table_address;
} SysPageTableRef;

extern SysPageTableRef sys_pagerefs[];
extern ushort sys_pagerefs_n;

void add_sys_pageref(ushort index, ulong address);



#endif // __BINFMT_H
