/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Простой драйвер флоппи-дисковода
 *
 *  Пока что я не заботился о возможном эффекте
 *  гонок и других аномалиях, связанных с
 *  многозадачностью, ибо таковой еще не было в
 *  момент написания этого кода.
 *
 *  Описание работы с контроллером дисковода можно
 *  найти в [6] и [7].
 *
 *
 *  Кэш чтения
 *    Для оптимизации работы с диском введен кэш секторов,
 *    который работает только для чтения. Я не уверен в том,
 *    что эта фича вообще нужна, но пока она неплохо помогает
 *    тупому драйверу FAT.
 *
 *
 *  FIXME: Опция, включающая использование кэша не должна
 *    задаваться при компиляции. Она должна меняться
 *    динамически!
 */


#include "config.h"
#include "types.h"
#include "io.h"
#include "fd.h"
#include "panic.h"
#include "hello_stdio.h"
#include "hello_string.h"




#define FD_SECTOR_SIZE     512
#define FD_SECTOR_SIZE_CODE   0x02



// Если включен кэш секторов
#if (CFG_FD_READ_CACHE == 1)
struct _fd_cache_item
{
   uint LBA;
   uchar Data[FD_SECTOR_SIZE];
};
typedef struct _fd_cache_item fd_cache_item;


// Кэш организовывается в виде кольцевой очереди
// Для упрощения очередь всегда имеет размер
// равный CFG_FD_READ_CACHE_SIZE. Сначала заполняется
// несуществующими секторами. При загрузке нового сектора
// из кеша удаляется элемент с номером fd_cache_pos.
fd_cache_item fd_cache[CFG_FD_READ_CACHE_SIZE];
uint fd_cache_pos;

#endif



// Почти после каждого ввода/вывода в порты контроллера
// нам нужно дождаться бита RQM в главном регистре в знак
// того, что контроллер готов слушать.
void fd_wait_for_RQM()
{
   while ((inb_p(FD_MSR) & 0x80) == 0);
}

// Вывести байт в регистр DOR
void out_DOR(uchar value)
{
   outb_p(value, FD_DOR);
}

// Вывести в FIFO
void out_FIFO(uchar value)
{
   fd_wait_for_RQM();
   outb_p(value, FD_FIFO);
}

// Считать байт из FIFO
uchar in_FIFO()
{
   fd_wait_for_RQM();
   return inb_p(FD_FIFO);
}



// После отправки контроллеру команд нам нужно дождаться
// ответного прерывания. Для этого используем самый тупой
// способ: останавливаемся и ждем, пока обработчик прерывания
// не поднимет флаг.

// Флаг наличия прерывания
volatile uchar __FD_INT_FLAG = 0;

// Очистка флага
void fd_clear_int_flag()
{
   __FD_INT_FLAG = 0;
}

// Ожидание прерывания
void fd_wait_for_int()
{
   while (__FD_INT_FLAG == 0);// PrintHex(&__FD_INT_FLAG, 1);
   __FD_INT_FLAG = 0;
}


// Иницилизация контроллера и драйвера
void fd_init()
{
#if (CFG_FD_READ_CACHE == 1)
   // Иницилизируем кольцевую кэш-очередь
   uint i;
   for (i = 0; i < CFG_FD_READ_CACHE_SIZE; i++)
      fd_cache[i].LBA = -1;
   fd_cache_pos = 0;
#endif

   fd_clear_int_flag();
   out_DOR(0x1c);    // Включаем двигатель первого привода
   fd_wait_for_RQM();

   // Посылаем команду Иницилизация
   out_FIFO(0x07);
   out_FIFO(0x00);

   //fd_wait_for_RQM();
   fd_wait_for_int();

   out_FIFO(0x08);      // Посылаем комадну Sense Interrupt Status
   uchar ST0;
   ST0 = in_FIFO();           // И считываем ее результат: ST0 и PCN
   in_FIFO();
}




void fd_write_sector_real(uint lba, uchar *buf);


// Записывает один сектор из буфера в указанный сектор на диске
// Сектор указывается с помощью сквозной нумерации (LBA?)
// 
// Кроме записи сектора на диск эта функция обновляет этот сектор
// в кэше, если он там, конечно, есть.
void fd_write_sector(uint lba, uchar *buf)
{
   fd_write_sector_real(lba, buf);

#if (CFG_FD_READ_CACHE == 1)
   uint i;
   for (i = 0; i < CFG_FD_READ_CACHE_SIZE; i++)
      if (fd_cache[i].LBA == lba)
      {
         memcpy(&fd_cache[i].Data, buf, FD_SECTOR_SIZE);
         return;
      }
#endif
}


// А эта функция На Самом Деле записывает сектор на диск
// FIXME: Константы прошиты в код
void fd_write_sector_real(uint lba, uchar *buf)
{
   const int SecPerCyl = 18;

   uchar PCN, ST0, ST1, ST2, C, H, R, N;
   int sector, head, cylinder;

   // Переводим LBA в Cylinder:Head:Sector
   // [80][2][18]
   cylinder = lba / SecPerCyl / 2;
   head = lba % (SecPerCyl * 2) / SecPerCyl;
   sector = lba % SecPerCyl + 1;

   fd_clear_int_flag();
   out_FIFO(0x0f);   // Перемещаем головку на нужный цилиндр
   out_FIFO(0x00);
   out_FIFO((uchar)(cylinder));
   fd_wait_for_int();

   out_FIFO(0x08);      // Посылаем комадну Sense Interrupt Status
   in_FIFO();           // И считываем ее результат: ST0 и PCN
   PCN = in_FIFO();

   // Если передвинуть головку не получилось
   // то вызываем панику. Вообще говоря
   // в таком случае лучше попробовать еще раз.
   if (PCN != cylinder)
      panic("FD: Seek error");



   // Иницилизируем контроллер DMA
   // FIXME!!!!
   // Буфер должен находиться в пределах первых 16 мегабайт!
   // Нужно будет выделить специальный системный буфер и копировать
   // из него в пользовательский буфер.
   outb_p(0x4a, 0x0b);  // 4a - запись
   outb_p(0x4a, 0x0c);  //
   outb_p((uint)(buf) & 0xff, 0x04);         //
   outb_p(((uint)(buf) >> 8 ) & 0xff, 0x04); // 24-битный адрес
   outb_p(((uint)(buf) >> 16) & 0xff, 0x81); //
   outb_p(0xff, 0x05);  //
   outb_p(0x01, 0x05);  // 0x01ff - размер
   outb_p(0x02, 0x0a);


   // Посылаем команду записи
   out_FIFO(0xc5);
   out_FIFO(head << 2);
   out_FIFO(cylinder);
   out_FIFO(head);
   out_FIFO(sector);
   out_FIFO(FD_SECTOR_SIZE_CODE);   // Код размера сектора. 0x02 = 512 байт
   out_FIFO(0x12);   // EOT. Кол-во секторов на дорожке
   out_FIFO(0x1b);   // GPL. Для 1.44mb обычно 0x1b
   out_FIFO(0xff);   // DTL. При записи целыми секторами - 0xff

   ST0 = in_FIFO();
   ST1 = in_FIFO();
   ST2 = in_FIFO();
   C   = in_FIFO();
   H   = in_FIFO();
   R   = in_FIFO();
   N   = in_FIFO();

   if (ST0 & 0xd8) panic("FD: Read error (ST0)");
   if (ST1 & 0x02) panic("FD: Read-only");
   if (ST1)        panic("FD: Read error (ST1)");
   if (ST2 & 0xf7) panic("FD: Read error (ST2)");
}



void fd_read_sector_real(uint lba, uchar *buf);

// Считать сектор с диске
// Сектор указывается с помощью сквозной нумерации (LBA?)
//
// Эта функция на самом деле ищет в кэше уже считанный сектор,
// а если его нет, то вызывает fd_read_sector_real.
void fd_read_sector(uint lba, uchar *buf)
{
#if (CFG_FD_READ_CACHE == 1)

   uint i;

   // Ищем нужный сектор в кеш-очереди
   for (i = 0; i < CFG_FD_READ_CACHE_SIZE; i++)
      if (fd_cache[i].LBA == lba)
      {
         memcpy(buf, &fd_cache[i].Data, FD_SECTOR_SIZE);
         return;
      }

   // Не нашли. Загружаем вытесняя
   fd_read_sector_real(lba, buf);

   fd_cache[fd_cache_pos].LBA = lba;
   memcpy(&fd_cache[fd_cache_pos].Data, buf, FD_SECTOR_SIZE);

   fd_cache_pos = (fd_cache_pos + 1) % CFG_FD_READ_CACHE_SIZE;

#else
   fd_read_sector_real(lba, buf);
#endif
}



// На Самом Деле считывает один сектор в буфер
//
// FIXME: Константы прошиты в код
void fd_read_sector_real(uint lba, uchar *buf)
{
   const int SecPerCyl = 18;

   uchar PCN, ST0, ST1, ST2, C, H, R, N;
   int sector, head, cylinder;

   // Переводим LBA в Cylinder:Head:Sector
   // [80][2][18]
   cylinder = lba / SecPerCyl / 2;
   head = lba % (SecPerCyl * 2) / SecPerCyl;
   sector = lba % SecPerCyl + 1;

   fd_clear_int_flag();
   out_FIFO(0x0f);   // Перемещаем головку на нужный цилиндр
   out_FIFO(0x00);
   out_FIFO((uchar)(cylinder));
   fd_wait_for_int();

   out_FIFO(0x08);      // Посылаем комадну Sense Interrupt Status
   in_FIFO();           // И считываем ее результат: ST0 и PCN
   PCN = in_FIFO();

   // Если передвинуть головку не получилось
   // то вызываем панику. Вообще говоря
   // в таком случае лучше попробовать еще раз.
   if (PCN != cylinder)
      panic("FD: Seek error");



   // Иницилизируем контроллер DMA
   // FIXME!!!!
   // Буфер должен находиться в пределах первых 16 мегабайт!
   // Нужно будет выделить специальный системный буфер и копировать
   // из него в пользовательский буфер.
   outb_p(0x46, 0x0b);  // 4a - запись
   outb_p(0x46, 0x0c);  //
   outb_p((uint)(buf) & 0xff, 0x04);         //
   outb_p(((uint)(buf) >> 8 ) & 0xff, 0x04); // 24-битный адрес
   outb_p(((uint)(buf) >> 16) & 0xff, 0x81); //
   outb_p(0xff, 0x05);  //
   outb_p(0x01, 0x05);  // 0x01ff - размер
   outb_p(0x02, 0x0a);


   // Посылаем команду записи
   out_FIFO(0xc6);
   out_FIFO(head << 2);
   out_FIFO(cylinder);
   out_FIFO(head);
   out_FIFO(sector);
   out_FIFO(FD_SECTOR_SIZE_CODE);   // Код размера сектора. 0x02 = 512 байт
   out_FIFO(0x12);   // EOT. Кол-во секторов на дорожке
   out_FIFO(0x1b);   // GPL. Для 1.44mb обычно 0x1b
   out_FIFO(0xff);   // DTL. При записи целыми секторами - 0xff

   ST0 = in_FIFO();
   ST1 = in_FIFO();
   ST2 = in_FIFO();
   C   = in_FIFO();
   H   = in_FIFO();
   R   = in_FIFO();
   N   = in_FIFO();

   if (ST0 & 0xd8) panic("FD: Read error (ST0)");
   if (ST1 & 0x02) panic("FD: Read-only");
   if (ST1)        panic("FD: Read error (ST1)");
   if (ST2 & 0xf7) panic("FD: Read error (ST2)");
}
