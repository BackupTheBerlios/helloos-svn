/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ������� ������� �������� ������� FAT
 *
 *  �������������� FAT12, FAT16 � FAT32. ������ ��� ������.
 *
 *  �������� ������� �������� ������� ��. � [4].
 * 
 *  FIXME:
 *   - ��� ����� ���� �� ����� FileIterate ���������� ������
 *     ��������? ��������, ��������� bad cluster ��� EOC?
 *   - �������� �� ��������� ���������� ��������� � �����.
 *     ��������, ���������� fat-������� � ����.
 *
 */



#include "io.h"
#include "fat.h"
#include "fd.h"
#include "hello_stdio.h"
#include "hello_string.h"
#include "types.h"





// ����� ����� ��������� ������ ��, ��� �
// ������������� ���������, �� ��� ���������.


// ��������� ��� FAT12/FAT16, ������������ ��
// �������� 36 � ������� �������.
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


// ��������� ��� FAT32, ������������ �� �������� 36
// � ������� �������.
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


// ����� ��� ���� FAT'�� ���� � ������� �������.
// ���������� � ������ �������� �������.
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
      // ���������� � ����������� �� �������
      FAT1216_BPB_Tail Tail1216;
      FAT32_BPB_Tail Tail32;
   };
} __attribute__((packed));
typedef struct _FAT_BPB FAT_BPB;



// ��������� FSInfo. ���� �� ������������.
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






// �������� ���������� Type, � �����������
// �� ������� FAT
#define FAT12     0
#define FAT16     1
#define FAT32     2





uchar bpbSector[512];
FAT_BPB *bpb;   // ��������� �� �������� �������
uchar Type;    // ��� �������� �������

ulong FATSz;   // ������ FAT-�������. ����������� �� FATSz16 ��� FAT1x � FATSz32 ��� FAT32.
ulong TotSec;  // ���������� �������� �� �������. ����������� �� TotSec16 ��� FAT1x � TotSec32 ��� FAT32.
ulong RootDirSectors;   // ���������� ��������, ���������� �������� ���������. ������ ��� FAT1x. ��� FAT32 ����� 0.
ulong FirstDataSector;  // ������ ������, � �������� ���������� ���������� ���������.
ulong DataSec;          // ���������� �������� � ������� �� �������
ulong CountofClusters;  // ���������� ��������� �� �������
ulong FirstRootDirSecNum;  // ������, � �������� ���������� �������� �������. ������������ ������ ��� FAT1x.

ulong CurDir; // ������ ������� �������� ��������



// ��������� Count �������� � �����, ������� � ������� Start.
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




// ��������� ����� �������, � �������� ���������� ������� ����� N.
// ������ ��� �������� (0 � 1) ���������������.
ulong ClusterToSector(ulong N)
{
   return ((ulong)(N - 2) * bpb->SecPerClus) + FirstDataSector;
}


// ���������� ������ � FAT-������� ��� ������� N. ��� ������ ��������
// ������� �� ��������� �������, ���� �������� ����� ������� (End of
// Clusterchain, EOC), ���� 0, ���� ������� ��������, ���� ������ ������
// �������� (Bad Cluster).
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



//  ��������� � ������� ������ ���?
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


// ��������� ������� ������?
// �� ����� ���� ��� ������� ������� �� ������ ����������� ��� ������ ������.
// ��� ����� ������ ��� ������, �� � ����������, ��� ���� ������� �����
// ����������� ������ ������� � ��������...
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


// �� ������ ����� � ������������ �������� ���������� ��� ������ �������
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


void RootDirIterate(DirCallback Callback, void *Data)
{
   if (Type == FAT12  ||  Type == FAT16)
   {
//      uchar *Sectors = (uchar*) malloc(bpb->BytsPerSec * RootDirSectors);
      static uchar Sectors[224*32];
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
   //uchar *Sectors = (uchar*) malloc(bpb->BytsPerSec * bpb->SecPerClus);
   static uchar Sectors[512];//bpb->BytsPerSec * bpb->SecPerClus];
   //uchar Sectors[bpb->BytsPerSec * bpb->SecPerClus];

   LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, Sectors);
   while (Size > (unsigned int )bpb->BytsPerSec * bpb->SecPerClus)
   {
      if (! Callback(Sectors, bpb->BytsPerSec * bpb->SecPerClus, Data))
      {
         //free(Sectors);
         return;
      }

      Size -= bpb->BytsPerSec * bpb->SecPerClus;

      Cluster = GetNextCluster(Cluster);
      if ((Cluster == 0) || IsEOC(Cluster) || IsBad(Cluster))
      {
         puts("Unexpected clusterchain termination!\n");
         //free(Sectors);
         return;
      }
      LoadSectorsFromDisk(ClusterToSector(Cluster), bpb->SecPerClus, Sectors);
   }

   Callback(Sectors, Size, Data);

   //free(Sectors);
}



bool PrintFileCallback(uchar *Block, ulong len, void *Data)
{
   ulong i;
   uchar s[] = {0, 0};
//   PrintHex(&Block, 4); puts("\n");
//   PrintHex(&len, 4);
//   while(1);
   for (i = 0; i < len; i++)
   {
      s[0] = Block[i];
      puts(s);
   }
   Data=Data; // Make the compiler happy ;)
   return 1;
}

// �������� ���� �� �����

void PrintFile(DirEntry *Entry)
{
   FileIterate(Entry, PrintFileCallback, 0);
}




uchar CharTo83(uchar c)
{
   // �������� ����������� ������� �� �������������
   // ������������ ��������:
   //  - ������� 0x20
   //  - " * + , / : ; < = > ? [ \ ] |
   //  ������ � �������� ������������� ������ �������� ������� ascii (���� ;)
   if (c<0x20 || c==0x22 || c==0x2a || c==0x2b || c==0x2c || c==0x2e || c==0x2f || (c>=0x3a && c<=0x3f) || c==0x5b || c==0x5c || c==0x5d || c >= 0x80)
      c = '_';

   // �������� � �������� ��������
   if (c >= 0x61 && c <= 0x7a) c -= 0x20;

   return c;
}

void Make83Name(char *fullname, char *name83)
{
   int len = 0, i, j;

   // �������������� ��������� ���������
   for (i = 0; i < 11; i++) name83[i] = 0x20;

   // �������� ������������ . � ..
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

   // �������� 8 �������� ��� �����
   i = 0;
   while ((len <= 8)  &&  (i < strlen(fullname))  &&  (fullname[i] != '.'))
      name83[len++] = CharTo83(fullname[i++]);

   // ���� �����
   for (i = 0; i < strlen(fullname); i++) if (fullname[i] == '.') break;

   // ���� ����� - �������� ����������
   if (i < strlen(fullname))
   {
      j = 8;
      i++;
      while ((j < 11)  &&  (i < strlen(fullname)))
         name83[j++] = CharTo83(fullname[i++]);
   }

   // ������ ������ �� ����� ���� �������� (��������, ��� ".foo")
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

// ���� ���� � ������ ��������
// ���������� ������ ������� ���������� �����/��������
// ��� ������� ���������� -1
// ���� �������� EntryBuf != 0, �� � ���� ������������ DirEntry
// ���������� �����/��������

ulong FindEntry(ulong DirCluster, char *Name83, DirEntry *EntryBuf)
{
   FindData Data;
   Data.Name = Name83;
   Data.Result = -1;
   Data.EntryBuf = EntryBuf;
   DirIterate(DirCluster, (DirCallback)FindEntryCallback, &Data);
   return Data.Result;
}




// ������������ ��������
void fat_init()
{
   LoadSectorsFromDisk(0, 1, &bpbSector);
   bpb = (FAT_BPB*)bpbSector;

   {
      ushort Sig;
      Sig = bpbSector[511];
      Sig *= 256;
      Sig += bpbSector[510];
      if (Sig != 0xAA55)
      {
         puts("0xAA55 signature is NOT OK\n");
         return;
      }
   }


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

   if (Type == FAT12) puts("Hmm... this is FAT12... ");
   if (Type == FAT16) puts("Hmm... this is FAT16... ");
   if (Type == FAT32) puts("Hmm... this is FAT32... ");
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
