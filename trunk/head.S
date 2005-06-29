/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  head.S
 *
 *  Начало 32-битного ядра
 *
 *  Проверяет A20 и иницилизирует регистры для
 *  перехода на часть, написанную на C.
 *
 */

// Начинаем секцию кода
.text

// Делает эту метку глобальной
.global startup_32


// Сегмент данных ядра. Используем второй
// сегмент в GDT (см. boot.S)
#define __KERNEL_DS  0x10


startup_32:

// Иницилизация. Запрещаем прерывания и заполняем
// сегментные регистры.
cld
cli
movl $(__KERNEL_DS),%eax
mov %ax, %ds
mov %ax, %es
mov %ax, %fs
mov %ax, %gs
mov %ax, %ss
mov $0x10000, %esp

// Убеждаемся, что A20 включилась
xorl %eax, %eax
1: incl %eax
   movl %eax, 0x000000
   cmpl %eax, 0x100000
   je 1b    /* Если нет - ждем пока не включится сама ;) */

// Чистим флаги
mov $0x10000, %esp
pushl $0
popfl



// Заполняем TSS главной задачи. См. комментарии в scheduler.c
   mov $TaskPool, %edx

   mov %esp, 4(%edx)
   mov %ss, 8(%edx)
   mov %esp, 56(%edx)
   mov %ss, 80(%edx)
   mov %ebp, 60(%edx)
   mov %es, 72(%edx)
   mov %cs, 76(%edx)
   mov %ds, 84(%edx)
   movw $0x10, 88(%edx)
   movl $0x200, 36(%edx)
   movl $start_my_kernel, 32(%edx)


// FIXME: Я не уверен, что создавать отдельную задачу на каждый
// обработчик прерывания это хорошая идея. Может быть нужно
// придумать что-то более подходящее.

// Заполняем TSS задачи-планировщика (IRQ0)
   movl %esp,  IRQ0_ESP0
   movl %ss,   IRQ0_SS0
   movl %esp,  IRQ0_ESP
   movl %ss,   IRQ0_SS
   movl %ebp,  IRQ0_EBP
   movl %es,   IRQ0_ES
   movl %cs,   IRQ0_CS
   movl %ds,   IRQ0_DS
   movl $0x10, IRQ0_FS
   movl $0x00000000, IRQ0_EFLAGS  // Все флаги сброшены
   movl $irq0_int, IRQ0_EIP

// Заполняем TSS функции игнорирования прерывания
   movl %esp,  II_ESP0
   movl %ss,   II_SS0
   movl %esp,  II_ESP
   movl %ss,   II_SS
   movl %ebp,  II_EBP
   movl %es,   II_ES
   movl %cs,   II_CS
   movl %ds,   II_DS
   movl $0x10, II_FS
   movl $0x00000000, II_EFLAGS  // Все флаги сброшены
   movl $irq0_int, II_EIP

// Заполняем TSS обработчика IRQ1
   movl %esp,  IRQ1_ESP0
   movl %ss,   IRQ1_SS0
   movl %esp,  IRQ1_ESP
   movl %ss,   IRQ1_SS
   movl %ebp,  IRQ1_EBP
   movl %es,   IRQ1_ES
   movl %cs,   IRQ1_CS
   movl %ds,   IRQ1_DS
   movl $0x10, IRQ1_FS
   movl $0x00000000, IRQ1_EFLAGS  // Все флаги сброшены
   movl $irq1_int, IRQ1_EIP



// Дописываем TSS'ы к GDT
   sgdt GDT_Size     // Размер и адрес GDT помещаются в GDT_Size и GDT_Addr

   xor %eax, %eax
   mov GDT_Size, %ax

   mov GDT_Addr, %ebx

   add %eax, %ebx    // ebx += eax + 1
   inc %ebx

   mov $TaskPool, %edx
   movw $0x0067,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x89,    (%ebx) ; inc %ebx
   movb $0x40,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx

   movw $24, 104(%edx)

   mov $ii_tss, %edx
   movw $0x0067,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x89,    (%ebx) ; inc %ebx
   movb $0x40,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx

   mov $irq0_tss, %edx
   movw $0x0067,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x89,    (%ebx) ; inc %ebx
   movb $0x40,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx

   mov $irq1_tss, %edx
   movw $0x0067,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x89,    (%ebx) ; inc %ebx
   movb $0x40,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx


   addw $32, %ax  // 32 - суммарный размер созданных TSS
   movw %ax, GDT_Size

   lgdt GDT_Size

   sub $31, %ax // ax = GDT_Size - 31

// Трюк, делающий переход ljmp, расположенный по метке jjj
// на сегмент %ax вместо $0xAA. Я не нашел способа сделать
// это культурно. Наверное, плохо искал.
   mov $jjj, %ebx
   mov %ax, 5(%ebx)


// Устанавливаем обработчики всех мыслимых прерываний.
// Для всех неиспользуемых (пока) прерываний ставим
// TSS игнорирующего обработчика.
// Для IRQ0 ставим TSS планировщика
setup_idt:
   addw $8, %ax
   mov %ax, %bx

   shll $16, %eax
   movw $0x00008500, %dx

   mov $_idt, %edi
   mov $256, %ecx
rp_sidt:
   movl %eax, (%edi)
   movl %edx, 4(%edi)
   addl $8, %edi
   dec %ecx
   jne rp_sidt


   lidt idt_descr


// Записываем TSS планировщика как обработчик irq0, а
// обработчик IRQ1 - на IRQ1
   mov %bx, %ax
   addw $8, %ax

   mov $_idt, %edi
   addl $64, %edi
   movw $0x0000, (%edi) ;  add $2, %edi
   movw %ax, (%edi)     ;  add $2, %edi
   movw $0x8500, (%edi) ;  add $2, %edi
   movw $0x0000, (%edi)


   addw $8, %ax

   mov $_idt, %edi
   addl $112, %edi
   movw $0x0000, (%edi) ;  add $2, %edi
   movw %ax, (%edi)     ;  add $2, %edi
   movw $0x8500, (%edi) ;  add $2, %edi
   movw $0x0000, (%edi)


// Инициализируем планировщик
call init_scheduler


// Уффф... Переходим на C-ядро!

sti
//call start_my_kernel

jjj:
   ljmp $0xAA, $0x00




// Мы никогда не должны сюда вернуться
inf:  jmp inf


.align 2
.global ignore_int
ignore_int:
   mov $0x20, %al
   out %al, $0x20

   iret
   jmp ignore_int


// Планировщик
// Сейчас он снимает текущую задачу, выбирает очередную из
// очереди TSS_QUEUE и запускает.
.align 2
.global irq0_int
irq0_int:
   call scheduler

   incb 0xb8f00
   movb $0x07, 0xb8f01

   mov $0x20, %al
   out %al, $0x20

   iret
   jmp irq0_int



// Обработчик IRQ1. Сейчас он просто устанавливает флаг __FD_INT_FLAG
// в 1 для драйвера диска.
.align 2
.global irq1_int
irq1_int:

   incb 0xb8f02
   movb $0x09, 0xb8f03

   movb $1, __FD_INT_FLAG

   mov $0x20, %al
   out %al, $0x20

   iret
   jmp irq1_int




.align 2
.word 0
idt_descr:
   .word 256 * 8 - 1
   .long _idt

.equiv _idt, 0x00040000



// Сюда загрузим адрес и размер GDT.
// FIXME: Мы дописываем сегменты к старой GDT, хранящейся
// в бутсекторе.
GDT_Size: .word 0x0000
GDT_Addr: .long 0x00000000


// Область для TSS'ов различных задач. Конечно, потом ее не
// будет в коде, когда я придумаю культурное размещение в
// памяти всех системных таблиц.


.global ii_tss
ii_tss:
II_TL  :   .long 0x00000000      /* Task link */
II_ESP0:   .long 0x00000000      /* ESP0 */
II_SS0 :   .long 0x00000000      /* SS0 */
II_ESP1:   .long 0x00000000      /* ESP1 */
II_SS1 :   .long 0x00000000      /* SS1 */
II_ESP2:   .long 0x00000000      /* ESP2 */
II_SS2 :   .long 0x00000000      /* SS2 */
II_CR3 :   .long 0x00000000      /* CR3 */
II_EIP :   .long 0x00000000      /* EIP */
II_EFLAGS: .long 0x00000000      /* EFLAGS */
II_EAX :   .long 0x00000000      /* EAX */
II_ECX :   .long 0x00000000      /* ECX */
II_EDX :   .long 0x00000000      /* EDX */
II_EBX :   .long 0x00000000      /* EBX */
II_ESP :   .long 0x00000000      /* ESP */
II_EBP :   .long 0x00000000      /* EBP */
II_ESI :   .long 0x00000000      /* ESI */
II_EDI :   .long 0x00000000      /* EDI */
II_ES  :   .long 0x00000000      /* ES */
II_CS  :   .long 0x00000000      /* CS */
II_SS  :   .long 0x00000000      /* SS */
II_DS  :   .long 0x00000000      /* DS */
II_FS  :   .long 0x00000000      /* FS */
II_GS  :   .long 0x00000000      /* GS */
II_LDT :   .long 0x00000000      /* LDT */
II_IOM :   .long 0x00000000      /* T and I/O Map */


.global irq0_tss
irq0_tss:
IRQ0_TL  :   .long 0x00000000      /* Task link */
IRQ0_ESP0:   .long 0x00000000      /* ESP0 */
IRQ0_SS0 :   .long 0x00000000      /* SS0 */
IRQ0_ESP1:   .long 0x00000000      /* ESP1 */
IRQ0_SS1 :   .long 0x00000000      /* SS1 */
IRQ0_ESP2:   .long 0x00000000      /* ESP2 */
IRQ0_SS2 :   .long 0x00000000      /* SS2 */
IRQ0_CR3 :   .long 0x00000000      /* CR3 */
IRQ0_EIP :   .long 0x00000000      /* EIP */
IRQ0_EFLAGS: .long 0x00000000      /* EFLAGS */
IRQ0_EAX :   .long 0x00000000      /* EAX */
IRQ0_ECX :   .long 0x00000000      /* ECX */
IRQ0_EDX :   .long 0x00000000      /* EDX */
IRQ0_EBX :   .long 0x00000000      /* EBX */
IRQ0_ESP :   .long 0x00000000      /* ESP */
IRQ0_EBP :   .long 0x00000000      /* EBP */
IRQ0_ESI :   .long 0x00000000      /* ESI */
IRQ0_EDI :   .long 0x00000000      /* EDI */
IRQ0_ES  :   .long 0x00000000      /* ES */
IRQ0_CS  :   .long 0x00000000      /* CS */
IRQ0_SS  :   .long 0x00000000      /* SS */
IRQ0_DS  :   .long 0x00000000      /* DS */
IRQ0_FS  :   .long 0x00000000      /* FS */
IRQ0_GS  :   .long 0x00000000      /* GS */
IRQ0_LDT :   .long 0x00000000      /* LDT */
IRQ0_IOM :   .long 0x00000000      /* T and I/O Map */


.global irq1_tss
irq1_tss:
IRQ1_TL  :   .long 0x00000000      /* Task link */
IRQ1_ESP0:   .long 0x00000000      /* ESP0 */
IRQ1_SS0 :   .long 0x00000000      /* SS0 */
IRQ1_ESP1:   .long 0x00000000      /* ESP1 */
IRQ1_SS1 :   .long 0x00000000      /* SS1 */
IRQ1_ESP2:   .long 0x00000000      /* ESP2 */
IRQ1_SS2 :   .long 0x00000000      /* SS2 */
IRQ1_CR3 :   .long 0x00000000      /* CR3 */
IRQ1_EIP :   .long 0x00000000      /* EIP */
IRQ1_EFLAGS: .long 0x00000000      /* EFLAGS */
IRQ1_EAX :   .long 0x00000000      /* EAX */
IRQ1_ECX :   .long 0x00000000      /* ECX */
IRQ1_EDX :   .long 0x00000000      /* EDX */
IRQ1_EBX :   .long 0x00000000      /* EBX */
IRQ1_ESP :   .long 0x00000000      /* ESP */
IRQ1_EBP :   .long 0x00000000      /* EBP */
IRQ1_ESI :   .long 0x00000000      /* ESI */
IRQ1_EDI :   .long 0x00000000      /* EDI */
IRQ1_ES  :   .long 0x00000000      /* ES */
IRQ1_CS  :   .long 0x00000000      /* CS */
IRQ1_SS  :   .long 0x00000000      /* SS */
IRQ1_DS  :   .long 0x00000000      /* DS */
IRQ1_FS  :   .long 0x00000000      /* FS */
IRQ1_GS  :   .long 0x00000000      /* GS */
IRQ1_LDT :   .long 0x00000000      /* LDT */
IRQ1_IOM :   .long 0x00000000      /* T and I/O Map */
