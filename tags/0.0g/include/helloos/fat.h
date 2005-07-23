/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Заголовок драйвера FAT
 *
 */


#ifndef __FAT_H
#define __FAT_H


#include <helloos/types.h>


// Содержимое каталога составлено их таких
// структур.
struct _DirEntry
{
   uchar    Name[11];      // 11
   uchar    Attr;          // 1
   uchar    NTRes;         // 1
   uchar    CrtTimeTenth;  // 1
   ushort   CrtTime;       // 2
   ushort   CrtDate;       // 2
   ushort   LstAccDate;    // 2
   ushort   FstClusHI;     // 2
   ushort   WrtTime;       // 2
   ushort   WrtDate;       // 2
   ushort   FstClusLO;     // 2
   ulong    FileSize;      // 4
} __attribute__((packed));
typedef struct _DirEntry DirEntry;


// Атрибуты файлов и каталогов
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  0x0f


// Тип callback-функции для пролистывания каталога
// Ей передается DirEntry и пользовательские данные
// Если callback-функция возвращает 0, пролистывание
// прекращается.
typedef bool (*DirCallback)(DirEntry* Entry, void* Data);

// Тип callback-функции для пролистывания файла
// Передается блок данных, его длина и пользовательские данные
// Если callback-функция возвращает 0, пролистывание
// прекращается.
typedef bool (*FileCallback)(uchar *Block, ulong len, void *Data);



// Отладочная функция с простым файловым браузером
void fat_main();


// Иницилизация драйвера
void fat_init();


// Пролистывание корневого каталога.
// То же самое, что DirIterate с первым параметром 0.
void RootDirIterate(DirCallback, void *Data);


// Пролистывание каталога
// Каталог задается своим первым кластером ;)
// На каждую запись в каталоге вызывается функция Callback,
// которой передается указатель на запись и параметр Data.
// Самой функцией параметр Data не используется.
// Не пытайтесь изменять значение записи из callback-функции,
// изменения не сохранятся.
void DirIterate(ulong Cluster, DirCallback Callback, void *Data);


// Вывод на экран содержимого каталога. Использует DirIterate.
void ListDir(ulong Cluster);


// Пролистывание файла. Файл задается своей записью в каталоге.
// При каждом вызове Callback ей передается указатель на очередной
// кусок данных (сейчас - очередной кластер) и его размер.
// Не пытайтесь изменять данные из callback-функции.
void FileIterate(DirEntry *Entry, FileCallback Callback, void *Data);


// Печатает файл на экран
void PrintFile(DirEntry *Entry);


// Перевод длинного имени в имя типа 8.3 с дополнением пробелами,
// как это описано в [4] для формата имени в записях каталогов.
// Вряд ли эта функция кому-то полезна, кроме самого драйвера.
// Буфер name83 должен быть не меньше 11 байт.
void Make83Name(char *fullname, char *name83);


// Ищет файл в данном каталоге
// Возвращает первый кластер найденного файла/каталога
// При неудаче возвращает -1
// Если параметр EntryBuf != 0, то в него записывается DirEntry
// найденного файла/каталого
ulong FindEntry(ulong DirCluster, char *Name83, DirEntry *EntryBuf);

// Читает в буфер часть файла, len байт, начиная с байта номер start
void LoadPart(DirEntry *Entry, void *Buf, uint start, uint len);

#endif // __FAT_H
