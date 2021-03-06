/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ������ 32-������� ����
 *
 *  ���������� ������������ �������������,
 *  ��������� ��������� �������, ��������
 *  ��� ������������ ���������� � �����
 *  ����� ����. ������� ����� ������� ���� ;)
 *
 */


#include <helloos/head.h>

// �������� ������ ����
.text

// ������ ��� ����� ����������
.global HelloWorld


HelloWorld:

// ������������. ��������� ���������� � ���������
// ���������� ��������.
   cli
   movl $KERNEL_DS, %eax
   mov %ax, %ds
   mov %ax, %es
   mov %ax, %fs
   mov %ax, %gs
   mov %ax, %ss
   mov $0x30000, %esp


// ������ �����
   pushl $0
   popfl

// �������������� ���������� ����/�����
   xor %eax, %eax
   movb 0x98001, %al
   push %eax
   movb 0x98000, %al
   push %eax
   call scrio_init
   add $8, %esp

// � ����� �� ������� ���������
   push $woow_msg
   call puts
   add $4, %esp



   call irq_init



// ������������� ���������� ���������
//
// � ���� ������, ������ ������ �������� ���.
// ������� ������� ��������� � 0x0000. ��� ������� ������� (� 0x3000 � 0x4000)
// ��������� ������ ���������� 8Mb �� �������� ��, ������� �� ������� ��. ���
// ������� ���� � ��������� ���� ������� ���. ������ 2 �� ��� ������� ��������
// ������������ �������������. ������ ��� ����� ��������� ������ 8�� ���. ������ - ���
// ����� ��� ��������.
// ��� �������� ���������� ������ � ���� �� ���������� ���� � ������ � DPL=3,
// ����� 0 � ������� 2Gb, �� ���� �� �������� ������ "��" ��������. ���� � ���������
// ������ ���������� ���������� � DPL=0, ����� 2Gb � ������� 2Gb, � ��� �� ����������
// ��������� ��� ������� � ������ ���������.


// ������� ��� ������ � ������ ��� ������� ;)
   push $mapping_msg
   call puts
   add $4, %esp


// �������� ������� �������
   xorl %eax, %eax
   movl $1024, %ecx
   xorl %edi, %edi
   cld
   rep stosl

// ������������� ������ �� ������� �������
   movl $0x1007, 0x0000
   movl $0x2007, 0x0004
   movl $0x3001, 0x0800
   movl $0x4001, 0x0804

// ��������� ��������, ���������� �� �������� ������ 0x00000000 - 0x00800000
   movl $0x7, %eax   // 7 - �����: �������� � cpl=3, ������/������, ������������
   movl $1024*2, %ecx   // ��������� ��� ������� �������
   cld
1: // edi ���������� ������ � 0x1000
   stosl
   addl $0x1000, %eax
   loop 1b

// ��������� ��������, ���������� �� �������� ������ 0x80000000 - 0x80800000

// FIXME: ������ ���� 0x1, �� ����� qemu ��������
// ��������. ��������, ��� ���� � qemu.
   movl $0x7, %eax   // 1 - �����: �������� ������ ����, ������ ������
   movl $1024*2, %ecx   // ��������� ��� ������� �������
   movl $0x3000, %edi
   cld
1:
   stosl
   addl $0x1000, %eax
   loop 1b

   push $mbytes
   call puts
   add $4, %esp


   xorl %eax, %eax   // ��������� �����
   movl %eax, %cr3   //   �������� �������

   movl %cr0, %eax
   orl $0x80000000, %eax   // �������� ���������� ���������
   movl %eax, %cr0

   call pager_init    // �������������� ��� �������� ������



   push $filling_msg
   call puts
   add $4, %esp



// FIXME: � �� ������, ��� ��������� ��������� ������ �� ������
// ���������� ���������� ��� ������� ����. ����� ���� �����
// ��������� ���-�� ����� ����������.

   push $tss_msg
   call puts
   add $4, %esp


// ��������� TSS ������-������������ (IRQ0)
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
   movl $0x0,  EFLAGS(%edi)  // ��� ����� ��������
   movl $irq0_int, EIP(%edi)
   movl $0x0,  CR3(%edi)
   movl $0x0,  IOM(%edi)

// ���������� TSS'� � GDT


   push $gdt_msg
   call puts
   add $4, %esp


   xor %eax, %eax
   mov GDT_Size, %ax // eax = ������ GDT - 1

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

   addw $8, GDT_Size  // 32 - ��������� ������ ��������� TSS

   lgdt gdt_descr


// ������������� ����������� ���� �������� ����������.
// ��� ���� �������������� (����) ���������� ������
// TSS ������������� �����������.
// ��� IRQ0 ������ TSS ������������

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

// �.�. idt ��������� � ������ bss, �� � �� ���� ������
// �������� � idt_descr ".long 0x80000000+$idt". ������� �����
// ��������� idt_descr ����� �������� �������� �����
// idt � ����������� ���� .long 0x80000000. ����������,
// ��� ������ ������������ ����� ������� ������.
   movl $idt_descr, %eax
   addl $idt, 2(%eax)
   lidt idt_descr


// ���������� ���������� IRQ6 - �� IRQ6
// ���������� interrupt-gate
   mov $idt+(IRQ0_INT+6)*8, %edi
   mov $irq6_int, %edx
   movw %dx,      0(%edi)
   movw $KERNEL_CS, 2(%edi)
   movw $0x8e00,  4(%edi)
   shrl $16, %edx
   movw %dx,      6(%edi)



// ���������� ��������� ����� ��� ���������� int 0x80
// ���������� trap-gate
   mov $idt+0x80*8, %edi
   mov $SYS_CALL, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)


// ���������� ���������� #GP
// trap-gate
   mov $idt+0x0d*8, %edi
   mov $gp_fault, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)


// FIXME!!! � �� ���� ���������� ���������� #PF ���� ��
// ����������� ��������� IRQ6 �� ������ ����������. �.�.
// ���� ��� ��� ������ ����, �� ���� � ��� #PF ����� ��������
// ���������� ��� ������� ;)
   mov $idt+0x0e*8, %edi
   mov $pf_fault, %edx
   movw %dx,      0(%edi)
   movw $0x0008,  2(%edi)
   movw $0xef00,  4(%edi)
   shr $16, %edx
   movw %dx,      6(%edi)



// �������������� �����������
   call init_scheduler


   sti   // �� ����� ����!

   call fd_init
   call fat_init
   call ipc_init

   push $loading_start_msg
   call puts
   add $4, %esp

   push $startarg
   push $startfilename
   call bin_load_bin
   add $8, %esp


// ���������� ����������� ��� ���������� IRQ0
// ������ ����� ������ ���� �������, �.�. �� �����
// ��������� �� ������������ �� ������������� ������
cli
   mov $idt+IRQ0_INT*8, %edi
   movw $0x0000,  0(%edi)
   movw $IRQ0_TSS,2(%edi)
   movw $0x8500,  4(%edi)
   movw $0x0000,  6(%edi)

sti

   // ����... ���!..
   // ������������� �� ������� ������ ������� �����������
   int $IRQ0_INT


startfilename:
   .ascii "start\0"

startarg:
   .ascii "arg\0"


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
   .ascii "\nOk, \001\021kernel\003 activated. Now loading the \001\015start...\n\n\0"



// "����������" �������������� ����������
// ��� ��� �� ������ - ������� PIC'�, ��� ����������
// ����������
.align 2
.global ignore_int
ignore_int:
   push %eax

   mov $0x20, %al
   out %al, $0x20

   pop %eax

   iret


// �����������
.align 2
.global irq0_int
irq0_int:
   call scheduler // �������� C-�������

   incb 0xb8f00         // ��� ����������� ����������
   movb $0x07, 0xb8f01  //   �� ���, ��� ��� ��������

   mov $0x20, %al
   out %al, $0x20

   iret
   jmp irq0_int



// ���������� IRQ6. ������ �� ������ ������������� ���� __FD_INT_FLAG
// � 1 ��� �������� �����.
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



// ��� ���������� ������
//
// ��� ��������� ������ ������ ����:
//   * eax = ����� ���������� ������
//   * ebx,ecx,edx = ��������� ���������� ������
//
// �������� ������������� �� �������� ����. ���������
// � ���������������� ������ ������������ ����� GS.
.global SYS_CALL
.align 2
SYS_CALL:
   push %ds
   push %es
   push %fs
   push %gs
   push %edi
   push %esi
   push %edx
   push %ecx
   push %ebx

   movl $KERNEL_DS, %ebx
   movl %ebx, %ds
   movl %ebx, %es
   movl %ebx, %fs
   movl $USER_DS, %ebx
   movl %ebx, %gs

   // ���������� �� ����� syscall?
   cmpl syscall_nr, %eax
   jge bad_syscall

   call *syscall_table(,%eax,4)

syscall_ret:

   pop %ebx
   pop %ecx
   pop %edx
   pop %esi
   pop %edi
   pop %gs
   pop %fs
   pop %es
   pop %ds

   iret

bad_syscall:
   mov $-1, %eax
   jmp syscall_ret
   


// ���������� General Protection Fault
// ������ ������� ������� �������
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
   call puts// ������ � ��� ����
   add $4, %esp


   call scheduler_kill_current
// �� ���� �� ��������

gp_str:
   .ascii "\001\014Ouch! General Protection Fault! Killing current task...\n\0"




// ���������� Page Fault
// � ������� �� ����� ������ ����� ����, �������� Demand-Loading,
// Copy-On-Write, Swapping � ����. � ������ ������ ������� �������.
// FIXME: ������ ��� �� ��������! ��. ����������� ��� ���� ���������
// ����������.
.global pf_fault
.align 2
pf_fault:
   push %ds    // 9
   push %es    // 8
   push %fs    // 7
   push %gs    // 6
   push %edi   // 5
   push %esi   // 4
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

   mov 10*4(%esp), %eax

   push %eax
   mov %cr2, %eax
   push %eax

   call pf_handler
   add $8, %esp

   
   pop %eax
   pop %ebx
   pop %ecx
   pop %edx
   pop %esi
   pop %edi
   pop %gs
   pop %fs
   pop %es
   pop %ds
   add $4, %esp

   iret


pf_str:
   .ascii "\001\014Ouch! Page Fault! Killing current task...\n\0"


.align 0x1000

// ���� ���������������� �������� �������� ���������� ��� ������
.global user_exit_code
user_exit_code:
   // � eax ���������� ��� ����������
   mov %eax, %ebx
   mov $0, %eax   // 0 - ��� ������ sys_exit
   int $0x80


.data

// ���������� IDT
.align 2
idt_descr:
   .word 256 * 8 - 1
   .long 0x80000000 // + $idt see above

// ������� ������������
.global gdt
.align 16
gdt:
// ��� ������� GDT, ������������� �����������
   .long 0x00000000  // ������� ���������� ������
   .long 0x00000000  //  ������ ���� �������

   .long 0x0000ffff  // ������� ����
   .long 0x80c79a00  //   ��� ����

   .long 0x0000ffff  // ������� ������
   .long 0x80c79200  //   ��� ����

   .long 0x0000ffff  // ������� ����
   .long 0x00c7fa00  //   ��� ���������

   .long 0x0000ffff  // ������� ������
   .long 0x00c7f200  //   ��� ���������


   .fill 256-5, 8, 0 // 5 ��� �����������

// ���������� GDT
.align 2
.global gdt_descr
gdt_descr:
GDT_Size: .word 0x0027     // ��� �������� ���������
GDT_Addr: .long 0x80000000+gdt //   �� ����� ����������

.bss


// ������� ��� TSS'�� ������������ ����������

ii_tss:     .fill 0x68,1,0
.global irq0_tss  // �� ��� ������� � scheduler()
irq0_tss:   .fill 0x68,1,0
irq6_tss:   .fill 0x68,1,0


// ������� ����������
.global idt
.align 16
idt:
   .fill 256, 8, 0

