/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Простой драйвер файловой системы FAT
 *
 *  Поддерживается FAT12, FAT16 и FAT32. Только для чтения.
 *
 *  Описание формата файловой системы см. в [4].
 * 
 *  FIXME:
 *   - Что будет если во время FileIterate произойдет ошибка
 *     кластера? Например, попадется bad cluster или EOC?
 *   - Хотелось бы уменьшить количество обращений к диску.
 *     Например, кэшировать fat-таблицу и проч.
 *
 */



#include <helloos/io.h>
#include <helloos/fat.h>
#include <helloos/fd.h>
#include <helloos/scrio.h>
#include <string.h>
#include <helloos/types.h>
#include <helloos/pager.h>
#include <helloos/syscall.h>
#include <stdio.h>





// Имена полей оставлены такими же, как в
// вышеуказанном документе, но без префиксов.


// Структура для FAT12/FAT16, начинающаяся со
// смещения 36 в нулевом секторе.
struct _FAT1216_BPB_Tail
{
   uchar    DrvNum;        // 1
   uchar    Reserved1;     // 1
   uchar    BootSig;       // 1
   ulong    VolID;         // 4
   uchar    VolLab[11];    // 11
   uchar    FilSysType[8]; // 8
} __attribute__((packed));
typedef struct _FAT1216_BPB_Tail FAT1216_BPB_Tail;


// Структура для FAT32, начинающаяся со смещения 36
// в нулевом секторе.
struct _FAT32_BPB_Tail
{
   ulong    FATSz32;       // 4
   ushort   ExtFlags;      // 2
   ushort   FSVer;         // 2
   ulong    RootClus;      // 4
   ushort   FSInfo;        // 2
   ushort   BkBootSec;     // 2
   uchar    Reserved[12];  // 12
   uchar    DrvNum;        // 1
   uchar    Reserved1;     // 1
   uchar    BootSig;       // 1
   ulong    VolID;         // 4
   uchar    VolLab[11];    // 11
   uchar    FilSysType[8]; // 8
} __attribute__((packed));
typedef struct _FAT32_BPB_Tail FAT32_BPB_Tail;


// Общие для всех FAT'ов поля в нулевом секторе.
// Начинается с начала нулевого сектора.
struct _FAT_BPB
{
   uchar    jmpBoot[3];    // 3
   uchar    OEMName[8];   // 8
   ushort   BytsPerSec;    // 2
   uchar    SecPerClus;    // 1
   ushort   RsvdSecCnt;    // 2
   uchar    NumFATs;       // 1
   ushort   RootEntCnt;    // 2
   ushort   TotSec16;      // 2
   uchar    Media;         // 1
   ushort   FATSz16;       // 2
   ushort   SecPerTrk;     // 2
   ushort   NumHeads;      // 2
   ulong    HiddSec;       // 4
   ulong    TotSec32;      // 4
   union
   {
      // Выбирается в зависимости от формата
      FAT1216_BPB_Tail Tail1216;
      FAT32_BPB_Tail Tail32;
   };
} __attribute__((packed));
typedef struct _FAT_BPB FAT_BPB;



// Структура FSInfo. Пока не используется.
struct _FAT32_FSInfo
{
   ulong    LeadSig;       // 4
   uchar    Reserved1[480];// 480
   ulong    StrucSig;      // 4
   ulong    Free_Count;    // 4
   ulong    Nxt_Free;      // 4
   uchar    Reserved2[12]; // 12
   ulong    TrailSig;      // 4
} __attribute__((packed));
typedef struct _FAT32_FSInfo FAT32_FSInfo;






// Значения переменной Type, в зависимости
// от формата FAT
#define FAT12     0
#define FAT16     1
#define FAT32     2





uchar bpbSector[512];
FAT_BPB *bpb;   // Структура из нулевого сектора
uchar Type;    // Тип файловой системы

ulong FATSz;   // Размер FAT-таблицы. Вычисляется из FATSz16 для FAT1x и FATSz32 для FAT32.
ulong TotSec;  // Количество секторов на разделе. Вычисляется из TotSec16 для FAT1x и TotSec32 для FAT32.
ulong RootDirSectors;   // Количество секторов, занимаемых корневым каталогом. Только для FAT1x. Для FAT32 равно 0.
ulong FirstDataSector;  // Первый сектор, с которого начинается содержимое кластеров.
ulong DataSec;          // Количество секторов с данными на разделе
ulong CountofClusters;  // Количество кластеров на разделе
ulong FirstRootDirSecNum;  // Сектор, с которого начинается корневой каталог. Используется только для FAT1x.

ulong CurDir; // Первый кластер текущего каталога



// Загружает Count секторов с диска, начиная с сектора Start.
void LoadSectorsFromDisk(ulong Start, ulong Count, void *Buf)
{
   uchar *cBuf = (uchar*)Buf;
   uint i;

   for (i = 0; i < Count; i++)
   {
      fd_read_sector(Start + i, cBuf);
      cBuf += bpb->BytsPerSec;
   }
}




// Вычисляет номер сектора, с которого начинается кластер номер N.
// Первые два кластера (0 и 1) зарезервированы.
ulong ClusterToSector(ulong N)
{
   return ((ulong)(N - 2) * bpb->SecPerClus) + FirstDataSector;
}


// Возвращает запись в FAT-таблице для сектора N. Эта запись является
// ссылкой на следующий кластер, либо маркером конца цепочки (End of
// Clusterchain, EOC), либо 0, если кластер свободен, либо маркет битого
// кластера (Bad Cluster).
ulong GetNextCluster(ulong N)
{
   ulong SecNum, EntOffset;
   ulong FATOffset;
   ulong Result;

   if (Type == FAT16  ||  Type == FAT32)
   {
      if (Type == FAT16)
         FATOffset = N * 2;
      else
         FATOffset = N * 4;

      SecNum = bpb->RsvdSecCnt + (FATOffset / bpb->BytsPerSec);
      EntOffset = FATOffset % bpb->BytsPerSec;

      uchar FATSectors[bpb->BytsPerSec];// = (uchar*) malloc(bpb->BytsPerSec);
      LoadSectorsFromDisk(SecNum, 1, FATSectors);

      if (Type == FAT16)
         Result = * (ushort *) &FATSectors[EntOffset];
      else
         Result = * (ulong *) &FATSectors[EntOffset] & 0x0FFFFFFF;

      //free(FATSectors);
      return Result;
   }
   else
   {
      FATOffset = N + (N / 2);   // N * 1.5

      SecNum = bpb->RsvdSecCnt + (FATOffset / bpb->BytsPerSec);
      EntOffset = FATOffset % bpb->BytsPerSec;

      uchar FATSectors[bpb->BytsPerSec*2];// = (uchar*) malloc(bpb->BytsPerSec * 2);
      LoadSectorsFromDisk(SecNum, 2, FATSectors);

      Result = * (ushort *) &FATSectors[EntOffset];
      if (N & 1)     // if N is odd
         Result >>= 4;
      else
         Result &= 0x0fff;

      //free(FATSectors);
      return Result;
   }
}



//  Кластеров в цепочке больше нет?
bool IsEOC(ulong Mark)
{
   switch (Type)
   {
      case FAT12:
         return Mark >= 0xff8;
      case FAT16:
         return Mark >= 0xfff8;
      case FAT32:
         return Mark >= 0xffffff8;
      default:
         return 1;
   }
}


// Следующий кластер плохой?
// На самом деле эта функция никогда не должна пригодиться при чтении файлов.
// Она нужна только при записи, но я сомниваюсь, что наша система будет
// отслеживать плохие секторы и кластеры...
bool IsBad(ulong Mark)
{
   switch (Type)
   {
      case FAT12:
         return Mark == 0xff7;
      case FAT16:
         return Mark == 0xfff7;
      case FAT32:
         return Mark == 0xffffff7;
      default:
         return 1;
   }
}


// По записи файла в родительском каталоге определить его первый кластер
ulong GetEntryCluster(DirEntry *Entry)
{
   return (Entry->FstClusHI << 16) | Entry->FstClusLO;
}



void DirIterate(ulong Cluster, DirCallback Callback, void *Data)
{
   if (Cluster == 0)
   {
      RootDirIterate(Callback, Data);
      return;
   }

   //void *Sectors = (void*) malloc(bpb->BytsPerSec * bpb->SecPerClus);
   uchar Sectors[bpb->BytsPerSec * bpb->SecPerClus];
   DirEntry *Entry = (DirEntry*) Sectors;
   LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, Sectors);
   uint cnt = 0;

   while (Entry->Name[0])
   {
      if ((Entry->Name[0] != 0xe5)  &&  ((Entry->Attr & ATTR_LONG_NAME) != ATTR_LONG_NAME))
         if (! Callback(Entry, Data))
            break;

      Entry++;
      cnt++;
      if (cnt >= bpb->BytsPerSec * bpb->SecPerClus / sizeof(DirEntry))
      {
         Cluster = GetNextCluster(Cluster);
         if ((Cluster != 0)  &&  (! IsEOC(Cluster))  && (! IsBad(Cluster)))
         {
            LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, Sectors);
            Entry = (DirEntry*) Sectors;
            cnt = 0;
         }
      }
   }

   //free(Sectors);
}

uchar Sectors[224*32];

void RootDirIterate(DirCallback Callback, void *Data)
{
   if (Type == FAT12  ||  Type == FAT16)
   {
//      uchar *Sectors = (uchar*) malloc(bpb->BytsPerSec * RootDirSectors);
//      uchar Sectors[bpb->BytsPerSec*RootDirSectors];
      DirEntry *Entry = (DirEntry*) Sectors;
      LoadSectorsFromDisk(bpb->RsvdSecCnt + FATSz * bpb->NumFATs, RootDirSectors, Sectors);

      while (Entry->Name[0])
      {
         if ((Entry->Name[0] != 0xe5)  &&  ((Entry->Attr & ATTR_LONG_NAME) != ATTR_LONG_NAME))
            if (! Callback(Entry, Data))
               break;
         Entry++;
      }

      //free(Sectors);
   }
   else
      DirIterate(bpb->Tail32.RootClus, Callback, Data);
}



bool ListDirCallback(DirEntry *Entry, void *Data)
{
//   printf("\"%.11s\"\t", Entry->Name);
   nputs(Entry->Name, 11);
   puts(" ");
   if ((Entry->Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME) puts("LFN ");
   else
   {
      if (Entry->Attr & ATTR_READ_ONLY) puts("RO ");
      if (Entry->Attr & ATTR_HIDDEN) puts("HID ");
      if (Entry->Attr & ATTR_SYSTEM) puts("SYS ");
      if (Entry->Attr & ATTR_DIRECTORY) puts("DIR ");
      if (Entry->Attr & ATTR_ARCHIVE) puts("ARC ");
      if (Entry->Attr & ATTR_VOLUME_ID) puts("VID ");
   }
//   printf("Cluster: %u ", GetEntryCluster(Entry));
//   printf("Size: %u ", Entry->FileSize);
   puts("\n");

   Data=Data; // Make the compiler happy ;)

   return 1;
}


void ListDir(ulong Cluster)
{
   DirIterate(Cluster, ListDirCallback, 0);
}




void FileIterate(DirEntry *Entry, FileCallback Callback, void *Data)
{
   ulong Size = Entry->FileSize;
   ulong Cluster = GetEntryCluster(Entry);

   byte *TmpPage = (byte*)alloc_first_page();

   LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, TmpPage);
   while (Size > (unsigned int )bpb->BytsPerSec * bpb->SecPerClus)
   {
      if (! Callback(TmpPage, bpb->BytsPerSec * bpb->SecPerClus, Data))
      {
         free_page((addr_t)TmpPage);
         return;
      }

      Size -= bpb->BytsPerSec * bpb->SecPerClus;

      Cluster = GetNextCluster(Cluster);
      if ((Cluster == 0) || IsEOC(Cluster) || IsBad(Cluster))
      {
         puts("Unexpected clusterchain termination!\n");
         free_page((addr_t)TmpPage);
         return;
      }
      LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, TmpPage);
   }

   Callback(TmpPage, Size, Data);

   free_page((addr_t)TmpPage);
   //free(Sectors);
}



bool PrintFileCallback(uchar *Block, ulong len, void *Data)
{
   ulong i;
   uchar s[] = {0, 0};
   for (i = 0; i < len; i++)
   {
      s[0] = Block[i];
      puts(s);
   }
   Data=Data; // Make the compiler happy ;)
   return 1;
}

// Печатает файл на экран

void PrintFile(DirEntry *Entry)
{
   FileIterate(Entry, PrintFileCallback, 0);
}


typedef struct
{
   uint start, len;
   uchar *Buf;

   uint filepos;
   uint bufpos;
} LoadPartData;

bool LoadPartCallback(uchar *Block, ulong len, LoadPartData *Data)
{
   if (Data->filepos >= Data->start + Data->len)
      return 0;

   if (Data->filepos + len >= Data->start)
   {
      uint s = (Data->start > Data->filepos) ? (Data->start - Data->filepos) : 0;
      uint n = len - s;
      if (n > Data->len-Data->bufpos) n = Data->len - Data->bufpos;
      memcpy(&Data->Buf[Data->bufpos], &Block[s], n);
      Data->bufpos += n;
   }
   

   Data->filepos += len;
   return 1;
}

void LoadPart(DirEntry *Entry, void *Buf, uint start, uint len)
{
   LoadPartData Data = {
      .start = start,
      .len = len,
      .Buf = Buf,
      .filepos = 0,
      .bufpos = 0
   };
   FileIterate(Entry, (FileCallback)LoadPartCallback, (void*)&Data);
}




uchar CharTo83(uchar c)
{
   // Заменяем запрещенные символы на подчеркивание
   // Запрещенными являются:
   //  - меньшие 0x20
   //  - " * + , / : ; < = > ? [ \ ] |
   //  Заодно я запрещаю использование второй половины таблицы ascii (пока ;)
   if (c<0x20 || c==0x22 || c==0x2a || c==0x2b || c==0x2c || c==0x2e || c==0x2f || (c>=0x3a && c<=0x3f) || c==0x5b || c==0x5c || c==0x5d || c >= 0x80)
      c = '_';

   // Приводим к верхнему регистру
   if (c >= 0x61 && c <= 0x7a) c -= 0x20;

   return c;
}

void Make83Name(char *fullname, char *name83)
{
   int len = 0, i, j;

   // Предварительно заполняем пробелами
   for (i = 0; i < 11; i++) name83[i] = 0x20;

   // Отдельно обрабатываем . и ..
   if (strcmp(fullname, ".") == 0)
   {
      name83[0] = '.';
      return;
   }
   if (strcmp(fullname, "..") == 0)
   {
      name83[0] = '.'; name83[1] = '.';
      return;
   }

   // Выбираем 8 символов для имени
   i = 0;
   while ((len <= 8)  &&  (i < strlen(fullname))  &&  (fullname[i] != '.'))
      name83[len++] = CharTo83(fullname[i++]);

   // Ищем точку
   for (i = 0; i < strlen(fullname); i++) if (fullname[i] == '.') break;

   // Если нашли - набираем расширение
   if (i < strlen(fullname))
   {
      j = 8;
      i++;
      while ((j < 11)  &&  (i < strlen(fullname)))
         name83[j++] = CharTo83(fullname[i++]);
   }

   // Первый символ не может быть пробелом (например, для ".foo")
   if (name83[0] == 0x20)
      name83[0] = '_';
}



struct _FindData
{
   uchar *Name;
   ulong Result;
   DirEntry *EntryBuf;
};
typedef struct _FindData FindData;

bool FindEntryCallback(DirEntry *Entry, FindData *Data)
{
   if (strncmp(Entry->Name, Data->Name, 11) == 0)
   {
      Data->Result = GetEntryCluster(Entry);
      if (Data->EntryBuf)
         memcpy(Data->EntryBuf, Entry, sizeof(DirEntry));
      return 0;
   }
   return 1;
}

// Ищет файл в данном каталоге
// Возвращает первый кластер найденного файла/каталога
// При неудаче возвращает -1
// Если параметр EntryBuf != 0, то в него записывается DirEntry
// найденного файла/каталого

ulong FindEntry(ulong DirCluster, char *Name83, DirEntry *EntryBuf)
{
   FindData Data;
   Data.Name = Name83;
   Data.Result = -1;
   Data.EntryBuf = EntryBuf;
   DirIterate(DirCluster, (DirCallback)FindEntryCallback, &Data);
   return Data.Result;
}




// Иницилизация драйвера
void fat_init()
{
   puts_color("Starting FAT driver...", 0x0b);

   LoadSectorsFromDisk(0, 1, &bpbSector);
   bpb = (FAT_BPB*)bpbSector;


   RootDirSectors = ((bpb->RootEntCnt * 32) + (bpb->BytsPerSec - 1)) / bpb->BytsPerSec;

   if (bpb->FATSz16 != 0)
      FATSz = bpb->FATSz16;
   else
      FATSz = bpb->Tail32.FATSz32;

   FirstDataSector = bpb->RsvdSecCnt + (bpb->NumFATs * FATSz) + RootDirSectors;

   if (bpb->TotSec16 != 0)
      TotSec = bpb->TotSec16;
   else
      TotSec = bpb->TotSec32;

   DataSec = TotSec - FirstDataSector;

   CountofClusters = DataSec / bpb->SecPerClus;

   if (CountofClusters < 4085)
      Type = FAT12;
   else
   {
      if (CountofClusters < 65525)
         Type = FAT16;
      else
         Type = FAT32;
   }

   if (Type == FAT12  ||  Type == FAT16)
      FirstRootDirSecNum = bpb->RsvdSecCnt + (bpb->NumFATs * bpb->FATSz16);
   else
      FirstRootDirSecNum = bpb->Tail32.RootClus;

   puts_color("\t\tFAT", 0x0a);
   if (Type == FAT12) puts_color("12", 0x0a);
   if (Type == FAT16) puts_color("16", 0x0a);
   if (Type == FAT32) puts_color("32", 0x0a);

   if (*(ushort*)(&bpbSector[510]) != 0xAA55)
      puts_color(" (bad signature!)", 0x0c);

   printf_color(0x0a, ", %d clusters", CountofClusters);

   puts("\n");
}



void fat_main()
{
   fat_init();

   puts("\n>>> Browser:\n");

   CurDir = 0;

   while (1)
   {
      char Name[100], N83[11];
      DirEntry Entry;

      ListDir(CurDir);

      //scanf("%s", &Name);
      readline(Name, 100);
      Make83Name(Name, N83);

      ulong New = FindEntry(CurDir, N83, &Entry);
      if (New == (uint)-1)
      {
         puts("No such file or directory: \"");
         nputs(N83, 11);
         puts("\"\n");
      }
      else
      {
         nputs(Entry.Name, 11);
         puts("\n");
         if (Entry.Attr & ATTR_DIRECTORY)
            CurDir = New;
         else
            PrintFile(&Entry);
      }
   }



   return;
}









// Системный вызов find_file
// Ищет файл или каталог с данным 8.3-именем и заполняет структуру DirEntry
// Возвращает первый кластер файла и (uint)-1 в случае неудачи
uint syscall_find_file(uint dir, char *name83, DirEntry *fileentry)
{
   char locname[11];
   memcpy_from_user(locname, name83, 11);
   DirEntry locentry;
   uint res = FindEntry(dir, locname, &locentry);
   memcpy_to_user(fileentry, &locentry, sizeof(DirEntry));
   return res;
}



bool user_load_file(uchar *Block, ulong len, LoadPartData *Data)
{
   if (Data->filepos >= Data->start + Data->len)
      return 0;

   if (Data->filepos + len >= Data->start)
   {
      uint s = (Data->start > Data->filepos) ? (Data->start - Data->filepos) : 0;
      uint n = len - s;
      if (n > Data->len-Data->bufpos) n = Data->len - Data->bufpos;
      memcpy_to_user(&Data->Buf[Data->bufpos], &Block[s], n);
      Data->bufpos += n;
   }
   

   Data->filepos += len;
   return 1;
}

// Системный вызов file_load
// Загружает кусок файла, заданный структурой chunk в буфер Buf
// Если chunk == 0, то загружается весь файл
// Возвращает размер загруженного куска, который может быть меньше
// чем chunk.length в случае, если файл закончится раньше чем
// заполнится буфер
uint syscall_file_load(DirEntry *Entry, byte *Buf, FileChunk *chunk)
{
   FileChunk locchunk;
   if (chunk)
      memcpy_from_user(&locchunk, chunk, sizeof(FileChunk));
   LoadPartData Data = {
      .start = chunk == 0 ? 0 : locchunk.start,
      .len = chunk == 0 ? (uint)-1 : locchunk.length,
      .Buf = Buf,
      .bufpos = 0,
      .filepos = 0
   };
   DirEntry locentry;
   memcpy_from_user(&locentry, Entry, sizeof(DirEntry));
   FileIterate(&locentry, (FileCallback)user_load_file, &Data);
   return Data.bufpos;
}





typedef struct
{
   DirEntry *buf;
   uint size;
   uint bufpos;
   uint count;
} DirLoadData;

bool DirLoadCallback(DirEntry *Entry, DirLoadData *Data)
{
   if (Data->count < Data->size)
      memcpy_to_user(&Data->buf[Data->bufpos], Entry, sizeof(DirEntry));
   Data->bufpos++;
   Data->count++;
   return 1;
}

// Системный вызов dir_load
// Заполняет массив Buf структурами для каждого дочернего файла или каталога
// каталога, заданного параметром dir
// size определяет количество элементов в структуре Buf
// Если size меньше количество элементов каталога, то загружается только
// size первых элементов
// Возвращает полное количество элементов в каталоге
uint syscall_dir_load(uint dir, DirEntry *Buf, uint size)
{
   DirLoadData Data =
   {
      .buf = Buf,
      .size = size,
      .bufpos = 0,
      .count = 0
   };
   DirIterate(dir, (DirCallback)DirLoadCallback, &Data);
   return Data.count;
}
