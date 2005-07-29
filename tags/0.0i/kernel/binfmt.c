/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции и структуры для работы с бинарными
 *  файлами
 *
 *
 *  Общая структура такова. При запуске бинарника на самом
 *  деле содержимое файла в память не загружается.  Части
 *  файла реально загружаются в память когда (и если)
 *  программа обратится *  по соответствующему виртуальному
 *  адресу. При этом вызывается исключение Page Fault, и
 *  функция load_page модуля, соответствующего формату
 *  файла, должна загрузить нужную страницу.  Для поддержки
 *  этого процесса в TaskStruct сохраняются заголовки
 *  бинарника, различные для различный форматов.
 *
 */


#include <helloos/binfmt.h>

#include <helloos/aout.h>
#include <helloos/elf.h>
#include <helloos/scheduler.h>
#include <helloos/scrio.h>
#include <helloos/pager.h>
#include <string.h>


// Структура описывает форматы бинарных файлов
BinFmt BinFormats[BIN_N] = 
{
   {  // BIN_AOUT
      .FormatName = "A.OUT",
      .is = aout_is,
      .dump_info = aout_info,
      .load_bin = aout_load,
      .load_page = aout_pf,
   },
   {  // BIN_ELF
      .FormatName = "ELF",
      .is = elf_is,
      .dump_info = elf_info,
      .load_bin = elf_load,
      .load_page = elf_pf,
   }
};


// Определить тип бинарника
int bin_type(char *name)
{
   uint i;
   for (i = 0; i < BIN_N; i++)
      if (BinFormats[i].is(name))
         return i;
   return -1;
}

// Напечатать заголовки бинарника
bool bin_dump_info(char *name)
{
   int type = bin_type(name);
   if (type != -1)
   {
      BinFormats[type].dump_info(name);
      return 1;
   }
   else
      return 0;
}

// Запустить бинарник
bool bin_load_bin(char *name)
{
   int type = bin_type(name);
   if (type != -1)
   {
      BinFormats[type].load_bin(name);
      return 1;
   }
   else
      return 0;
}


// Обработчик #PF
void pf_handler(uint address, uint errcode)
{
//   printf_color(0x04, "Page Fault: addr=0x%x, errcode=0x%x\n", address, errcode);

   ulong ok = 0;
   TaskStruct *task = Task[Current];


   // Младшый бит errcode определяет, было ли вызвано
   // исключение отсутствующей страницей (0) или нарушением
   // привелегий
   if ((errcode & 1) == 0)
   {
      if (BinFormats[task->BinFormat].load_page)
         // Функция возвратит физический адрес новой страницы, А ТАКЖЕ ФЛАГИ для этой записи
         // в таблице страниц
         ok = BinFormats[task->BinFormat].load_page(address);
   }
   else
   {
      printf_color(0x04, "Strange #PF errcode=0x%x (pid=%d)\n", errcode, Task[Current]->pid);
   }

   if (! ok)
   {
      printf_color(0x04, "Process requested an invalid page (req addr=0x%x)! Killing him...\n", address);
      scheduler_kill_current();
   }
   else
      map_page(ok, Task[Current], address, ok & PA_MASK);
}
