/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Начало 32-битного ядра
 *
 *  Производит всевозможные инициализации,
 *  заполняет системные таблицы, содержит
 *  код обработчиков прерываний и точку
 *  входа ядра. Вообщем самый главный файл ;)
 *
 */


#include <helloos/head.h>

// Начинаем секцию кода
.text

// Делает эту метку глобальной
.global HelloWorld


HelloWorld:

// Иницилизация. Запрещаем прерывания и заполняем
// сегментные регистры.
   cli
   movl $KERNEL_DS, %eax
   mov %ax, %ds
   mov %ax, %es
   mov %ax, %fs
   mov %ax, %gs
   mov %ax, %ss
   mov $0x30000, %esp


// Чистим флаги
   pushl $0
   popfl

// Инициализируем консольный ввод/вывод
   xor %eax, %eax
   movb 0x98001, %al
   push %eax
   movb 0x98000, %al
   push %eax
   call scrio_init
   add $8, %esp

// И сразу же выводим сообщение
   push $woow_msg
   call puts
   add $4, %esp



   call irq_init



// Устанавливаем страничную адресацию
//
// В двух словах, модель памяти выглядит так.
// Каталог страниц находится в 0x0000. Две таблицы страниц (в 0x3000 и 0x4000)
// маппируют первые физические 8Mb на линейное АП, начиная со второго Гб. Это
// область ядра и процессам туда доступа нет. Нижние 2 Гб для каждого процесса
// составляются индивидуально. Сейчас они также маппируют первые 8Мб физ. памяти - это
// нужно для загрузки.
// Все процессы пользуются одними и теми же сегментами кода и данных с DPL=3,
// базой 0 и лимитом 2Gb, то есть им доступны только "их" страницы. Ядро и системные
// вызовы пользуются сегментами с DPL=0, базой 2Gb и лимитом 2Gb, а так же сегментами
// процессов для доступа к памяти процессов.


// Говорим что делаем и делаем что говорим ;)
   push $mapping_msg
   call puts
   add $4, %esp


// Обнуляем каталог страниц
   xorl %eax, %eax
   movl $1024, %ecx
   xorl %edi, %edi
   cld
   rep stosl

// Устанавливаем ссылки на таблицы страниц
   movl $0x1007, 0x0000
   movl $0x2007, 0x0004
   movl $0x3001, 0x0800
   movl $0x4001, 0x0804

// Заполняем страницы, отвечающие за линейные адреса 0x00000000 - 0x00800000
   movl $0x7, %eax   // 7 - флаги: доступны с cpl=3, чтение/запись, присутствуют
   movl $1024*2, %ecx   // Заполняем две таблицы страниц
   cld
1: // edi установлен раннее в 0x1000
   stosl
   addl $0x1000, %eax
   loop 1b

// Заполняем страницы, отвечающие за линейные адреса 0x80000000 - 0x80800000

// FIXME: должно быть 0x1, но тогда qemu престает
// работать. Наверное, это глюк в qemu.
   movl $0x7, %eax   // 1 - флаги: доступны только ядру, только чтение
   movl $1024*2, %ecx   // Заполняем две таблицы страниц
   movl $0x3000, %edi
   cld
1:
   stosl
   addl $0x1000, %eax
   loop 1b

   push $mbytes
   call puts
   add $4, %esp


   xorl %eax, %eax   // Загружаем адрес
   movl %eax, %cr3   //   каталога страниц

   movl %cr0, %eax
   orl $0x80000000, %eax   // Включаем страничную адресацию
   movl %eax, %cr0

   call pager_init    // Инициализируем наш менеджер памяти



   push $filling_msg
   call puts
   add $4, %esp



// FIXME: Я не уверен, что создавать отдельную задачу на каждый
// обработчик прерывания это хорошая идея. Может быть нужно
// придумать что-то более подходящее.

   push $tss_msg
   call puts
   add $4, %esp


// Заполняем TSS задачи-планировщика (IRQ0)
   movl $irq0_tss, %edi
   movl %esp,  ESP0(%edi)
   movl %ss,   SS0(%edi)
   movl %esp,  ESP(%edi)
   movl %ss,   SS(%edi)
   movl %ebp,  EBP(%edi)
   movl %es,   ES(%edi)
   movl %cs,   CS(%edi)
   movl %ds,   DS(%edi)
   movl %ds,   FS(%edi)
   movl $0x0,  EFLAGS(%edi)  // Все флаги сброшены
   movl $irq0_int, EIP(%edi)
   movl $0x0,  CR3(%edi)
   movl $0x0,  IOM(%edi)

// Дописываем TSS'ы к GDT


   push $gdt_msg
   call puts
   add $4, %esp


   xor %eax, %eax
   mov GDT_Size, %ax // eax = размер GDT - 1

   mov GDT_Addr, %ebx

   add %eax, %ebx    // ebx += eax + 1
   inc %ebx

   mov $irq0_tss+0x80000000, %edx
   movw $0x0067,  0(%ebx)
   movw %dx,      2(%ebx)
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      4(%ebx)
   movb $0x89,    5(%ebx)
   movb $0x40,    6(%ebx)
   movb %ch,      7(%ebx)

   addw $8, GDT_Size  // 32 - суммарный размер созданных TSS

   lgdt gdt_descr


// Устанавливаем обработчики всех мыслимых прерываний.
// Для всех неиспользуемых (пока) прерываний ставим
// TSS игнорирующего обработчика.
// Для IRQ0 ставим TSS планировщика

   push $idt_msg
   call puts
   add $4, %esp

setup_idt:
   mov $ignore_int, %edx
   mov $KERNEL_CS*0x10000, %eax
   mov %dx, %ax
   mov $0x8e00, %dx

   mov $idt, %edi
   mov $256, %ecx
rp_sidt:
   movl %eax, (%edi)
   movl %edx, 4(%edi)
   addl $8, %edi
   dec %ecx
   jne rp_sidt

// Т.к. idt находится в секции bss, то я не могу просто
// записать в idt_descr ".long 0x80000000+$idt". Поэтому перед
// загрузкой idt_descr нужно добавить реальный адрес
// idt в оставленное поле .long 0x80000000. Разумеется,
// все должно адресоваться через верхнюю память.
   movl $idt_descr, %eax
   addl $idt, 2(%eax)
   lidt idt_descr


// Записываем обработчик IRQ6 - на IRQ6
// Используем interrupt-gate
   mov $idt+(IRQ0_INT+6)*8, %edi
   mov $irq6_int, %edx
   movw %dx,      0(%edi)
   movw $KERNEL_CS, 2(%edi)
   movw $0x8e00,  4(%edi)
   shrl $16, %edx
   movw %dx,      6(%edi)



// Записываем системный вызов как обработчик int 0x80
// Используем trap-gate
   mov $idt+0x80*8, %edi
   mov $SYS_CALL, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)


// Записываем обработчик #GP
// trap-gate
   mov $idt+0x0d*8, %edi
   mov $gp_fault, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)


// FIXME!!! Я не могу установить обработчик #PF пока не
// переназначу дискетное IRQ6 на другое прерывание. Т.к.
// пока мне это делать лень, то пока у нас #PF будет вызывать
// прерывание как дискета ;)
   mov $idt+0x0e*8, %edi
   mov $pf_fault, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)



// Инициализируем планировщик
   call init_scheduler


   sti   // Да будет свет!

   call fd_init
   call fat_init

   push $loading_start_msg
   call puts
   add $4, %esp

   push $startfilename
   call aout_load
   add $4, %esp


// Записываем планировщик как обработчик IRQ0
// Раньше этого нельзя было сделать, т.к. он сразу
// попытался бы переключится на отсутствующую задачу
   mov $idt+IRQ0_INT*8, %edi
   movw $0x0000,  0(%edi)
   movw $IRQ0_TSS,2(%edi)
   movw $0x8500,  4(%edi)
   movw $0x0000,  6(%edi)

   // Уффф... Все!..
   // Переключаемся на главную задачу вызывая планировщик
   int $IRQ0_INT


startfilename:
   .ascii "start.out\0"


woow_msg:
   .ascii "\001\024woow!\n\0"

mapping_msg:
   .ascii "\001\023Mapping pages...\0"

filling_msg:
   .ascii "\001\023Filling system tables...\t\0"

tss_msg:
   .ascii "\001\022TSS... \0"

gdt_msg:
   .ascii "\001\022GDT... \0"

idt_msg:
   .ascii "\001\022IDT...\n\0"

mbytes:
   .ascii "\001\022\t\t8Mb mapped on low and high memory\n\0"

loading_start_msg:
   .ascii "\nOk, \001\021kernel\003 activated. Now loading the \001\015start.out...\n\n\0"



// "Обработчик" неиспользуемых прерываний
// Все что он делает - говорит PIC'у, что прерывание
// обработано
.align 2
.global ignore_int
ignore_int:
   push %eax

   mov $0x20, %al
   out %al, $0x20

   pop %eax

   iret


// Планировщик
.align 2
.global irq0_int
irq0_int:
   call scheduler // Вызываем C-функцию

   incb 0xb8f00         // Для визуального наблюдения
   movb $0x07, 0xb8f01  //   за тем, как все зависает

   mov $0x20, %al
   out %al, $0x20

   iret
   jmp irq0_int



// Обработчик IRQ6. Сейчас он просто устанавливает флаг __FD_INT_FLAG
// в 1 для драйвера диска.
.align 2
.global irq6_int
irq6_int:
   push %eax
   push %ds
   mov $KERNEL_DS, %eax
   mov %eax, %ds

   incb 0xb8f02
   movb $0x09, 0xb8f03

   movb $1, __FD_INT_FLAG

   mov $0x20, %al
   out %al, $0x20

   pop %ds
   pop %eax

   iret



// Код Системного Вызова
//
// При системном вызове должно быть:
//   * eax = номер системного вызова
//   * ebx,ecx,edx = параметры системного вызова
//
// Сегменты переключаются на сегменты ядра. Обращение
// к пользовательской памяти производится через GS.
.global SYS_CALL
.align 2
SYS_CALL:
   push %ds
   push %es
   push %fs
   push %gs
   push %edx
   push %ecx
   push %ebx

   movl $KERNEL_DS, %ebx
   movl %ebx, %ds
   movl %ebx, %es
   movl %ebx, %fs
   movl $USER_DS, %ebx
   movl %ebx, %gs

   // Существует ли такой syscall?
   cmpl syscall_nr, %eax
   jge bad_syscall

   call *syscall_table(,%eax,4)

syscall_ret:

   pop %ebx
   pop %ecx
   pop %edx
   pop %gs
   pop %fs
   pop %es
   pop %ds

   iret

bad_syscall:
   mov $-1, %eax
   jmp syscall_ret
   


// Обработчик General Protection Fault
// Просто убивает текущий процесс
.global gp_fault
.align 2
gp_fault:
   mov $KERNEL_DS, %ebx
   mov %ebx, %ds
   mov %ebx, %es
   mov %ebx, %fs
   mov $USER_DS, %ebx
   mov %ebx, %gs


   push $gp_str
   call puts// Скажем в чем дело
   add $4, %esp


   call scheduler_kill_current
// Мы сюда не вернемся

gp_str:
   .ascii "\001\014Ouch! General Protection Fault! Killing current task...\n\0"




// Обработчик Page Fault
// В будущем он будет делать умные вещи, например Demand-Loading,
// Copy-On-Write, Swapping и проч. В сейчас просто грохнем процесс.
// FIXME: Сейчас это не работает! см. комментарии для кода установки
// прерываний.
.global pf_fault
.align 2
pf_fault:
   push %ds    // 7
   push %es    // 6
   push %fs    // 5
   push %gs    // 4
   push %edx   // 3
   push %ecx   // 2
   push %ebx   // 1
   push %eax   // 0

   mov $KERNEL_DS, %ebx
   mov %ebx, %ds
   mov %ebx, %es
   mov %ebx, %fs
   mov $USER_DS, %ebx
   mov %ebx, %gs

   mov 8*4(%esp), %eax

   push %eax
   mov %cr2, %eax
   push %eax

   call pf_handler
   add $8, %esp

   
   pop %eax
   pop %ebx
   pop %ecx
   pop %edx
   pop %gs
   pop %fs
   pop %es
   pop %ds
   add $4, %esp

   iret


pf_str:
   .ascii "\001\014Ouch! Page Fault! Killing current task...\n\0"



.data

// Дескриптор IDT
.align 2
idt_descr:
   .word 256 * 8 - 1
   .long 0x80000000 // + $idt see above

// Таблица дескрипторов
.global gdt
.align 16
gdt:
// Это заменит GDT, установленную загрузчиком
   .long 0x00000000  // Нулевой дескриптор всегда
   .long 0x00000000  //  должен быть нулевым

   .long 0x0000ffff  // Сегмент кода
   .long 0x80c79a00  //   для ядра

   .long 0x0000ffff  // Сегмент данных
   .long 0x80c79200  //   для ядра

   .long 0x0000ffff  // Сегмент кода
   .long 0x00c7fa00  //   для процессов

   .long 0x0000ffff  // Сегмент данных
   .long 0x00c7f200  //   для процессов


   .fill 256-5, 8, 0 // 5 уже установлено

// Дескриптор GDT
.align 2
.global gdt_descr
gdt_descr:
GDT_Size: .word 0x0027     // Эти значения изменятся
GDT_Addr: .long 0x80000000+gdt //   во время выполнения

.bss


// Область для TSS'ов обработчиков прерываний

ii_tss:     .fill 0x68,1,0
.global irq0_tss  // Мы его трогаем в scheduler()
irq0_tss:   .fill 0x68,1,0
irq6_tss:   .fill 0x68,1,0


// Таблица прерываний
.global idt
.align 16
idt:
   .fill 256, 8, 0

