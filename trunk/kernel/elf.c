/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Поддержка бинарных файлов в формате ELF
 *  Пока реализована обработка только статических
 *  файлов, вся линковочная информация игнорируется.
 *
 *  Игнорируются также запросы на read-only-страницы,
 *  все страницы выделяются как read-write
 *
 */



#include <helloos/scrio.h>
#include <helloos/fat.h>
#include <helloos/pager.h>
#include <helloos/scheduler.h>
#include <string.h>

#include <helloos/elf.h>


// Возвращает строку: тип ELF-файла
char *parse_type(ushort type)
{
   switch (type)
   {
      case ET_NONE:  return "ET_NONE (0)";
      case ET_REL:   return "ET_REL (1)";
      case ET_EXEC:  return "ET_EXEC (2)";
      case ET_DYN:   return "ET_DYN (3)";
      case ET_CORE:  return "ET_CORE (4)";
   }
//   if (type >= ET_LOPROC  &&  type <= ET_HIPROC)
   if (type >= ET_LOPROC)
      return "Processor-specific (0xff00-0xffff)";

   return "Unknown";
}

// Возвращает строку: архитектура ELF-файла
char *parse_machine(ushort machine)
{
   switch (machine)
   {
      case EM_NONE:  return "EM_NONE (0)";
      case EM_M32:   return "EM_M32 (1)";
      case EM_SPARC: return "EM_SPARC (2)";
      case EM_386:   return "EM_386 (3)";
      case EM_68K:   return "EM_68K (4)";
      case EM_88K:   return "EM_88K (5)";
      case EM_860:   return "EM_860 (7)";
      case EM_MIPS:  return "EM_MIPS (8)";
   }

   return "Unknown";
}

// Возвращает строку: версия ELF-файла
char *parse_version(ulong version)
{
   switch (version)
   {
      case EV_NONE:  return "EV_NONE (0)";
      case EV_CURRENT: return "EV_CURRENT (1)";
   }

   return "Unknown";
}

// Проверяет сигнатуру ELF
bool check_ident(ulong signature)
{
   return signature == 0x464c457f;
}

// Возвращает строку: класс (разрядность) ELF-файла
char *parse_identclass(uchar class)
{
   switch (class)
   {
      case ELFCLASSNONE:   return "ELFCLASSNONE (0)";
      case ELFCLASS32:     return "ELFCLASS32 (1)";
      case ELFCLASS64:     return "ELFCLASS64 (2)";
   }

   return "Unknown";
}

// Возвращает строку: формат данных ELF-файла
char *parse_identdata(uchar data)
{
   switch (data)
   {
      case ELFDATANONE: return "ELFDATANONE (0)";
      case ELFDATA2LSB: return "ELFDATA2LSB (1)";
      case ELFDATA2MSB: return "ELFDATA2MSB (2)";
   }

   return "Unknown";
}


// Возвращает строку: тип секции
char *parse_sectiontype(ulong type)
{
   switch (type)
   {
      case SHT_NULL:       return "SHT_NULL (0)";
      case SHT_PROGBITS:   return "SHT_PROGBITS (1)";
      case SHT_SYMTAB:     return "SHT_SYMTAB (2)";
      case SHT_STRTAB:     return "SHT_STRTAB (3)";
      case SHT_RELA:       return "SHT_RELA (4)";
      case SHT_HASH:       return "SHT_HASH (5)";
      case SHT_DYNAMIC:    return "SHT_DYNAMIC (6)";
      case SHT_NOTE:       return "SHT_NOTE (7)";
      case SHT_NOBITS:     return "SHT_NOBITS (8)";
      case SHT_REL:        return "SHT_REL (9)";
      case SHT_SHLIB:      return "SHT_SHLIB (10)";
      case SHT_DYNSYM:     return "SHT_DYNSYM (11)";
   }

   if (type >= SHT_LOPROC && type <= SHT_HIPROC)
      return "Processor-specific";
   if (type >= SHT_LOUSER && type <= SHT_HIUSER)
      return "User-specific";

   return "Unknown";
}


// Возвращает строку: флаги секции в красивом формате (а ля objdump)
char str[5];
char *parse_sectionflags(ulong flags)
{
   str[0] = flags & SHF_ALLOC ? 'a' : '-';
   str[1] = flags & SHF_WRITE ? 'w' : '-';
   str[2] = flags & SHF_EXECINSTR ? 'x' : '-';
   str[3] = flags & SHF_MASKPROC ? 'p' : '\0';
   str[4] = '\0';

   return str;
}

// Возвращает строку: тип сегмента
char *parse_pheadertype(ulong type)
{
   switch (type)
   {
      case PT_NULL: return "PT_NULL (0)";
      case PT_LOAD: return "PT_LOAD (1)";
      case PT_DYNAMIC: return "PT_DYNAMIC (2)";
      case PT_INTERP: return "PT_INTERP (3)";
      case PT_NOTE: return "PT_NOTE (4)";
      case PT_SHLIB: return "PT_SHLIB (5)";
      case PT_PHDR: return "PT_PHDR (6)";
   }

   if (type >= PT_LOPROC && type <= PT_HIPROC)
      return "Processor-specific";

   return "Unknown";
}


// Выводит строку: имя секции
void print_sectionname(Elf32_Shdr *StrSec, Elf32_Shdr *Section, DirEntry *file)
{
   char name[100];
   // Смещение в файле null-строки с именем секции
   int pos = StrSec->sh_offset+Section->sh_name;
   // Пользуемся тем, что LoadPart остановится, если увидит конец файла
   LoadPart(file, &name, pos, 100);
   name[100] = 0; // Для уверенности
   if (name[0]) // Имя секции может быть пусто
      printf("%s", name);
   else
      printf("%s", "<null>");
}

// Проверяет, является ли файл ELF-бинарником
bool elf_is(char *Name)
{
   char name83[11];

   Make83Name(Name, name83);
   DirEntry Entry;
   if (FindEntry(0, name83, &Entry) == (uint)-1)
      return 0;

   uint FirstLong;
   LoadPart(&Entry, &FirstLong, 0, sizeof(uint));

   return check_ident(FirstLong);
}


// Печатает заголовки ELF
void elf_info(char *Name)
{
   char name83[11];

   Make83Name(Name, name83);
   DirEntry Entry;
   if (FindEntry(0, name83, &Entry) == (uint)-1)
   {
      printf("Cannot open file '%s'!\n", Name);
      return;
   }

   Elf32_Ehdr Header;   
   LoadPart(&Entry, &Header, 0, sizeof(Header));

   printf("HEADER:\n");

   if (! check_ident(*(ulong*)(&Header.e_ident)))
   { printf("\tBad elf ident\n"); return; }

   printf("\tHeader.e_ident[EI_CLASS] = %s\n", parse_identclass(Header.e_ident[EI_CLASS]));
   if (Header.e_ident[EI_CLASS] != ELFCLASS32)
   { printf("\tI dont want to support other than 32-bin ELFs"); return; }

   printf("\tHeader.e_ident[EI_DATA] = %s\n", parse_identdata(Header.e_ident[EI_DATA]));
   if (Header.e_ident[EI_DATA] != ELFDATA2LSB)
   { printf("\tI dont want to support other than low-endian ELFs"); return; }

   printf("\tHeader.e_ident[EI_VERSION] = %s\n", parse_version(Header.e_ident[EI_VERSION]));

   printf("\tHeader.e_type = %s\n", parse_type(Header.e_type));
   printf("\tHeader.e_machine = %s\n", parse_machine(Header.e_machine));
   if (Header.e_machine != EM_386) { printf("\tI support only i386 ELFs\n"); return; }
   printf("\tHeader.e_version = %s\n", parse_version(Header.e_version));
   printf("\tHeader.e_entry = 0x%x\n", Header.e_entry);
   printf("\tHeader.e_phoff = 0x%x\n", Header.e_phoff);
   printf("\tHeader.e_shoff = 0x%x\n", Header.e_shoff);
   printf("\tHeader.e_flags = 0x%x\n", Header.e_flags);
   if (Header.e_flags) printf("\tHmm... What flags? There are no defined flags for 386!\n");
   printf("\tHeader.e_ehsize = %d (%d)\n",  Header.e_ehsize, sizeof(Elf32_Ehdr));
   printf("\tHeader.e_phentsize = %d\n", Header.e_phentsize);
   printf("\tHeader.e_phnum = %d\n", Header.e_phnum);
   printf("\tHeader.e_shentsize = %d\n", Header.e_shentsize);
   printf("\tHeader.e_shnum = %d\n", Header.e_shnum);
   printf("\tHeader.e_shstrndx = %d\n", Header.e_shstrndx);


   printf("SECTIONS:\n");

   Elf32_Shdr Section[Header.e_shnum];
   LoadPart(&Entry, &Section, Header.e_shoff, sizeof(Elf32_Shdr)*Header.e_shnum);

   uint i;
   for (i = 0; i < Header.e_shnum; i++)
   {
      printf("\tSection %d:", i);
      printf(" Name = ");//, Section[i].sh_name);
      print_sectionname(&Section[Header.e_shstrndx], &Section[i], &Entry);
      printf(" Type = %s", parse_sectiontype(Section[i].sh_type));
      printf(" Addr = 0x%x", Section[i].sh_addr);
      printf(" Offs = 0x%x", Section[i].sh_offset);
      printf(" Size = 0x%x", Section[i].sh_size);
      char *flags = parse_sectionflags(Section[i].sh_flags);
      printf(" Flags = %s", flags);
      printf("\n");
   }

   printf("PROGRAM HEADERS:\n");

   Elf32_Phdr PHeader[Header.e_phnum];
   LoadPart(&Entry, &PHeader, Header.e_phoff, sizeof(Elf32_Phdr)*Header.e_phnum);

   for (i = 0; i < Header.e_phnum; i++)
   {
      printf("\tPHeader %d:", i);
      printf(" Type = %s", parse_pheadertype(PHeader[i].p_type));
      printf(" Offs = 0x%x", PHeader[i].p_offset);
      printf(" VAddr = 0x%x", PHeader[i].p_vaddr);
      printf(" PAddr = 0x%x", PHeader[i].p_paddr);
      printf(" FileSz = 0x%x", PHeader[i].p_filesz);
      printf(" MemSz = 0x%x", PHeader[i].p_memsz);
      printf(" PAlign = 0x%x", PHeader[i].p_align);

      printf("\n");
   }
}


// Определена в head.S
extern void user_exit_code();


// FIXME: это количество страниц, выделяемых на стек процесса.
// Нужно как-то лучше придумать.
#define USER_STACK_PAGES   2

// Создает процесс для ELF-файла
// (см. комментарии в binfmt.c)
// Возвращает PID или (uint)-1
uint elf_load(char *Name, char *arg)
{
   if (! elf_is(Name))
      return -1;

   char name83[11];
   DirEntry Entry;

   Make83Name(Name, name83);

   if (FindEntry(0, name83, &Entry) == (uint)-1)
   {
      printf("Cannot open file '%s'!\n", Name);
      return -1;
   }

   // Грузим ELF-заголовок
   Elf32_Ehdr Header;
   LoadPart(&Entry, &Header, 0, sizeof(Header));

   if (Header.e_ident[EI_CLASS] != ELFCLASS32 ||
         Header.e_ident[EI_DATA] != ELFDATA2LSB ||
         Header.e_machine != EM_386)
      return -1;


   // Грузим заголовки сегментов
   Elf32_Phdr PHeader[Header.e_phnum];
   LoadPart(&Entry, &PHeader, Header.e_phoff, sizeof(PHeader));


   // Создаем для процесса каталог страниц
   ulong *pg_dir;
   pg_dir = (ulong*)alloc_first_page();
   memset(pg_dir, 0, PAGE_SIZE);

   // Прописываем системные таблицы страниц, находящиеся по адресам 0x3000 и 0x4000
   int i;
   for (i = 0; i < sys_pagerefs_n; i++)
      pg_dir[sys_pagerefs[i].index_in_catalog] = sys_pagerefs[i].table_address + SYS_PAGE_ATTR;

   // Выделяем память для TaskStruct задачи
   TaskStruct *task = (TaskStruct*)alloc_first_page();

   // Сохраняем таблицу дескрипторов
   GDTDescriptor GDT;
   __asm__("sgdt %0":: "m" (GDT));
   ushort desc_count = (GDT.Size + 1) >> 3;
   ushort tssn = desc_count;


   // Прописываем атрибуты задачи
   task->pid = CurPID++;
   task->state = PS_RUNNING;
   task->tsss = tssn << 3; // Селектор TSS
   task->BinFormat = BIN_ELF; // Формат бинарника
   memcpy(&task->file, &Entry, sizeof(Entry));           // Сохраням DirEntry файла
   memcpy(&task->elf_header, &Header, sizeof(Header));   // а также заголовок ELF
   memset(&task->pheaders, 0, sizeof(task->pheaders));   // и
   memcpy(&task->pheaders, &PHeader, sizeof(PHeader));   // заголовки сегментов


   // Заполняем TSS
   task->tss.tl = 0;
   task->tss.esp0 = (ulong)&task->syscall_stack + sizeof(task->syscall_stack); // Стек для системных вызовов
   task->tss.ss0 = KERNEL_DS;

   task->tss.cr3 = (ulong)pg_dir;
   task->tss.eip = Header.e_entry;
   task->tss.eflags = 0x200; // Только IF
   task->tss.eax = task->tss.ebx =
      task->tss.ecx = task->tss.edx =
      task->tss.esi = task->tss.edi = 0;
   task->tss.cs = USER_CS;
   task->tss.es = task->tss.ss =
      task->tss.ds = task->tss.fs =
      task->tss.gs = USER_DS;
   task->tss.ldt = 0;
   task->tss.iomap_trace = 0;
   // esp и ebp устанавливаются ниже

   // Чтобы процесс имел возможность нормально завершится, мы должны предоставить ему адрес
   // возврата в стеке. Процесс передаст управление по этому адресу при выходе из main().
   // Для этого мы маппируем страницу с функцией user_exit_code (head.S) в АП процесса.
   // Адрес, по которому мы ее будет маппировать, расположим сразу после стека (FIXME: создаем
   // сами себе грабли для динамической линковки...)

   addr_t exit_page = USER_STACK_PAGES * PAGE_SIZE;

   // Выделим одну стековую страницу для того, чтобы положить в стек адрес возврата
   addr_t stack_page = alloc_first_page();
   map_page(stack_page, task, PAGE_ADDR(USER_STACK_PAGES * PAGE_SIZE - 1), PAGE_ATTR);

   uint arglen = strlen(arg)+1;
   arglen = (arglen + 3) / 4 * 4; // округляем вверх до четырех
   memcpy((char*)(stack_page+0x1000-arglen), arg, strlen(arg)+1);
   *(ulong*)(stack_page+0x1000-arglen-4) = USER_STACK_PAGES*PAGE_SIZE-arglen;
   *(ulong*)(stack_page+0x1000-arglen-8) = USER_STACK_PAGES*PAGE_SIZE-arglen-4;
   *(ulong*)(stack_page+0x1000-arglen-12)= 1;
   *(ulong*)(stack_page+0x1000-arglen-16)=exit_page;
   task->tss.esp = task->tss.ebp = USER_STACK_PAGES * PAGE_SIZE - arglen-16; // 4 байта для адреса возврата
   map_page((addr_t)&user_exit_code, task, exit_page, PA_USER | PA_P | PA_NONFREE);

   // Создаем в GDT дескриптор для TSS
   // FIXME: Их нужно удалять при завершении процесса!

   // TSS адресуется через верхнюю память
   ulong tss_addr = (ulong)&task->tss + 0x80000000;
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr - 0x80000000);
   GDT.Addr[tssn].a = (tss_addr<<16)|0x0067;
   GDT.Addr[tssn].b = (tss_addr&0xff000000)|0x00408b00|((tss_addr>>16)&0xff);
   GDT.Addr = (Descriptor*)((ulong)GDT.Addr + 0x80000000);

   // Загружаем gdtr
   GDT.Size += 8; // Один дескриптор добавили
   __asm__("lgdt %0"::"m"(GDT));

   // Живи!
   Task[NTasks] = task;
   NTasks++;

   return task->pid;
}


// Обработчик #PF для ELF-процессов
// Аргумент - адрес, по которому программа попыталась обратиться
// Результат - адрес загруженной страницы + флаги, или 0, если процесс "ошибся адресом"
addr_t elf_pf(addr_t address)
{
   // Может быть, стоит это передавать параметром
   TaskStruct *task = Task[Current];

   // Преобразуем в адрес страницы
   addr_t pageaddr = PAGE_ADDR(address);

   bool ok = 0;
   uchar *tmppage = 0;

   int i;
   for (i = 0; i < task->elf_header.e_phnum; i++)
   {
      Elf32_Phdr *ph = &task->pheaders[i];
      // Ищем сегмент, который содержит этот адрес
      if (address >= ph->p_vaddr && (address < ph->p_vaddr + ph->p_memsz))
      {
         // Адреса сегмента в памяти и в файле, согласно доке, должны быть равны по модулю
         // размера страницы, и это упрощает нам жизнь.
         // fileoffs - начало в файле первой страницы, содержащей сегмент (вернее - его часть)
         offs_t fileoffs = PAGE_ADDR(ph->p_offset);
         // segstart - начало в памяти первой страницы, содержащей сегмент (вернее - его часть)
         addr_t segstart = PAGE_ADDR(ph->p_vaddr);
         // filesize - размер блока, который мы будем грузить из файла. Может быть либо целой
         // страницой, либо частью страницы. Если обратились к неинициализированной части
         // сегмента (memsz>address>filesz), то filesize будет меньше 0. Это исправляется
         // следующей строкой.
         int filesize = MIN(PAGE_SIZE, (int)ph->p_filesz - ((int)pageaddr - (int)ph->p_vaddr));
         filesize = MAX(filesize, 0);
         // Выделяем новую страницу...
         tmppage = (uchar*)alloc_first_page();
         // ... и грузим ее из файла. Адрес в файле: fileoffs+pageaddr-segstart. Размер указан
         // в заголовке сегмента (p_filesz).
         if (filesize > 0)
            LoadPart(&task->file, tmppage, fileoffs+pageaddr-segstart, filesize);
         // diff - размер неинициализированной части, которую заполняем нулями
         int diff = MIN(PAGE_SIZE, ph->p_memsz - (pageaddr - ph->p_vaddr)) - filesize;
         if (diff > 0)
            memset(&tmppage[filesize], 0, diff);
         // Сделано, все свободны
         ok = 1;
         break;
      }
   }

   // Если не нашлось сегмента, который содержал бы нужный нам адрес, то считаем, что это стек,
   // и выделяем для него страницу, заполненную нулями.
   // FIXME: GCC создает специальный сегмент для стека, но в доке он не описывается. Надо
   // разобраться с ним.
   if (!ok)
   {
      // Если адрес внутри нашего стека
      if (address < USER_STACK_PAGES * PAGE_SIZE)
      {
         tmppage = (uchar*)alloc_first_page();
//         printf_color(0x04, "New stack page for ELF: 0x%x\n", address);
         memset(tmppage, 0, PAGE_SIZE);
         ok = 1;
      }
   }

   if (ok)
      return (ulong)tmppage + PAGE_ATTR;
   else
      return 0;
}
