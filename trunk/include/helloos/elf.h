/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Константы и структуры для работы с ELF
 *
 *  См. [11] - отличная дока!
 *
 *  Значком [*] помечены поля и константы, которые
 *  сейчас игнорируются HelloOS при загрузке файлов
 *
 */




#ifndef __ELF_H
#define __ELF_H

#include <helloos/types.h>


// Определить, является ли файл ELF
bool elf_is(char *Name);
// Напечатать заголовки ELF
void elf_info(char *Name);
// Запустить файл
void elf_load(char *Name);
// Обработчик #PF для ELF-бинарников
addr_t elf_pf(addr_t address);


// Размер поля e_ident
#define EI_NIDENT 16

// Заголовок ELF
typedef struct {
   uchar    e_ident[EI_NIDENT];  // Идентификация файла
   ushort   e_type;              // Тип объектного файла
   ushort   e_machine;           // Архитектура
   ulong    e_version;           // Версия [*]
   addr_t   e_entry;             // Точка входа
   offs_t   e_phoff;             // Смещение заголовков программы
   offs_t   e_shoff;             // Смещение таблицы секций
   ulong    e_flags;             // Флаги (для 386 нет флагов) [*]
   ushort   e_ehsize;            // Размер заголовка [*]
   ushort   e_phentsize;         // Размер одного заголовка программы
   ushort   e_phnum;             // Количество заголовков программы
   ushort   e_shentsize;         // Размер заголовка секции
   ushort   e_shnum;             // Количество секций
   ushort   e_shstrndx;          // Индекс секции, содержащей названия секций
}  Elf32_Ehdr;


// Типы объектного файла
// HelloOS принимает только ET_EXEC

// Для порядка
#define ET_NONE   0
// Перемещаемый
#define ET_REL    1
// Исполняемый
#define ET_EXEC   2
// Динамическая библиотека
#define ET_DYN    3
// Core-файл
#define ET_CORE   4
// Диапазон специфичных для процессора констант
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff


// Архитектуры
// HelloOS принимает только EM_386

// Для порядка
#define EM_NONE   0
// AT&T WE 32100
#define EM_M32    1
// SPARC
#define EM_SPARC  2
// Intel 80386
#define EM_386    3
// Motorola 68000
#define EM_68K    4
// Motorola 88000
#define EM_88K    5
// Intel 80860
#define EM_860    7
// MIPS RS3000
#define EM_MIPS   8


// Версии формата. В доке, по которой я изучал ELF
// описывается формат 1, и, насколько я знаю, других
// пока не существует [*]
#define EV_NONE      0
#define EV_CURRENT   1


// Смещения в e_ident

// Смещение класса файла (разрядности)
#define EI_CLASS  4
// Смещение формата данных (low/high-endian)
#define EI_DATA   5
// Смещение версии файла [*]
#define EI_VERSION   6
// Начало выравнивающих байтов [*]
#define EI_PAD    7


// Классы (разрядности) файлов
// HelloOS принимает только ELFCLASS32

#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2


// Форматы данных
// HelloOS принимает только ELFDATA2LSB

#define ELFDATANONE  0
// low-endian
#define ELFDATA2LSB  1
// high-endian
#define ELFDATA2MSB  2


//=================================//


// Предопределенные номера секций [*]

// Неопределенная секция. Эта константа используется для
// отметки в ссылках, для которых не нужно указывать номер
// секции.
#define SHN_UNDEF       0
// Задает диапазон зарезервированных номеров. Секций с
// такими номерами в файле не будет (еще бы! 65 тыс. секций!)
#define SHN_LORESERVE   0xff00
// Диапазон архитектурно-специфичных секций
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
// Если в ссылке стоит SHN_ABS, то адрес абсолютных
#define SHN_ABS         0xfff1
// Если в ссылке стоит SHN_COMMON, то это extern-переменная в С
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff


// Структура линковочной записи [*]
typedef struct
{
   ulong    sh_name;
   ulong    sh_type;
   ulong    sh_flags;
   addr_t   sh_addr;
   offs_t   sh_offset;
   ulong    sh_size;
   ulong    sh_link;
   ulong    sh_info;
   ulong    sh_addralign;
   ulong    sh_entsize;
} Elf32_Shdr;

// Типы секций [*]
// На данный момент я пропускаю всю информацию о секциях
// и для загрузки использую только заголовки программы
// (program headers). Соответственно, вся динамическая и
// линковочная информация игнорируется.

// Отсутсвие секции
#define SHT_NULL     0
// Содержимое зависит от программы. Обычно это загружаемые
// секции с кодом или данными.
#define SHT_PROGBITS 1
// Таблица символов
#define SHT_SYMTAB   2
// Таблица строк
#define SHT_STRTAB   3
// Relocation entries with explicit addends (?)
#define SHT_RELA     4
// Хеш-таблица символов (?)
#define SHT_HASH     5
// Таблица для динамической линковки
#define SHT_DYNAMIC  6
// Дополнительная информация
#define SHT_NOTE     7
// Такая же, как PROGBITS, но не содержит данных. Обычно это
// секция .bss.
#define SHT_NOBITS   8
// Relocation entries without explicit addends (?)
#define SHT_REL      9
// Этот тип зарезервирован, но не определен
#define SHT_SHLIB    10
// Минимальная таблица символов, необходимых для линковки
#define SHT_DYNSYM   11
// Дополнительные диапазоны
#define SHT_LOPROC   0x70000000
#define SHT_HIPROC   0x7fffffff
#define SHT_LOUSER   0x80000000
#define SHT_HIUSER   0xffffffff

// Флаги секций [*]

// Должна быть доступна для записи во время работы программы
#define SHF_WRITE       0x1
// Должна быть загружена в память программы
#define SHF_ALLOC       0x2
// Должна быть доступна для выполнения содержащихся инструкций
#define SHF_EXECINSTR   0x4
// Маска для архитектурно-специфичных флагов
#define SHF_MASKPROC    0xf0000000


//=================================//

// Структура заголовка программы. Сейчас для загрузки бинарников
// используются только эти заголовки, определяющие сегменты кода
// и данных. Как выяснилось, GCC кроме этих двух сегментов определяет
// еще и сегмент стека со странными атрибутами: размер=0 и смещение=0.
// К тому же этот сегмент имеет не определенный в документации тип.
// Поэтому этот сегмент игнорируется, а стек распологается в начале АП
// процесса и для него выделяется USER_STACK_PAGES страниц.
typedef struct
{
   ulong    p_type;     // Тип сегмента [*]
   offs_t   p_offset;   // Смещение в файле
   addr_t   p_vaddr;    // Виртуальный адрес в АП процесса
   addr_t   p_paddr;    // Физический адрес в АП процесса [*]
   ulong    p_filesz;   // Размер в файле
   ulong    p_memsz;    // Размер в памяти (может быть больше чем в файле,
                        // в таком случае дополняется нулями)
   ulong    p_flags;    // Флаги
   ulong    p_align;    // Выравнивание по этой величине
} Elf32_Phdr;

// Типы сегментов
// HelloOS загружает все сегменты, к которым программа будет обращаться.
// Это определяется по p_vaddr и p_memsz. При этом тип игнорируется.

// Неопределенный
#define PT_NULL      0
// Загружаемый
#define PT_LOAD      1
// Информация для динамической линковки
#define PT_DYNAMIC   2
// Указатель на имя интерпретатора
#define PT_INTERP    3
// Дополнительная информация
#define PT_NOTE      4
// Зарезервировано, но не определено
#define PT_SHLIB     5
// Определяет саму таблицу сегментов
#define PT_PHDR      6
// Диапазон архитектурно-специфичных типов
#define PT_LOPROC    0x70000000
#define PT_HIPROC    0x7fffffff

#endif // __ELF_H
