/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Тупой загрузчик тупых бинарных файлов.
 *  Создает для задачи сегмент кода, сегмент
 *  данных, TSS, сегмент для TSS, заполняет
 *  все поля и включает TSS в очередь планировщика.
 *
 *  FIXME: Убрать все это безобразие с глаз долой.
 *
 */

.text

.global dbg
.global dbg_

dbg:
   push %ebp
   mov %esp, %ebp

   sub $12, %esp

pusha

   mov 8(%ebp), %edx

   sgdt -6(%ebp)

   xor %eax, %eax
   mov -6(%ebp), %ax

   mov -4(%ebp), %ebx

   add %eax, %ebx
   inc %ebx

// Создаем еще один сегмент кода
   movw $0xffff,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x9a,    (%ebx) ; inc %ebx
   movb $0xcf,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx

// и данных
   addw $8, %ax

   movw $0xffff,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x92,    (%ebx) ; inc %ebx
   movb $0xcf,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx


   addw $8, %ax


   mov 12(%ebp), %edx

// Заполняем TSS
   movl %esp,   4(%edx)
   movl %ss,    8(%edx)
   movl %esp,  56(%edx)
   movl %ss,   80(%edx)
   movl %ebp,  60(%edx)
   movl %es,   72(%edx)
   xor %ecx, %ecx
   mov %ax, %cx
   sub $15, %cx
   movl %ecx,  76(%edx)
   xor %ecx, %ecx
   mov %ax, %cx
   sub $7, %cx
   movl %ecx,   84(%edx)
   movl $0x200,36(%edx)

// И создаем для него сегмент
   movw $0x0067,  (%ebx); add $2, %ebx
   movw %dx,      (%ebx); add $2, %ebx
   mov %edx,   %ecx
   shr $16,    %ecx
   movb %cl,      (%ebx) ; inc %ebx
   movb $0x8b,    (%ebx) ; inc %ebx
   movb $0x40,    (%ebx) ; inc %ebx
   movb %ch,      (%ebx) ; inc %ebx



   addw $8, %ax
   movw %ax, -6(%ebp)

dbg_:

   lgdt -6(%ebp)

   sub $7, %ax

// Добавляем в цепочку планировщика
   cli
   movl TSS_LENGTH, %edx
   movl $TSS_QUEUE, %ebx
   shl $1, %edx
   addl %edx, %ebx

   movw %ax, (%ebx)
   shr $1, %edx
   inc %edx
   movl %edx, TSS_LENGTH
   sti

//   mov $jjj, %ebx
//   mov %ax, 5(%ebx)
//
//jjj:
//   ljmp $0xAA, $0x00

   popa

   leave
   ret
