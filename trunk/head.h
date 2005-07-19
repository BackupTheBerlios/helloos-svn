/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  ��������� ���������
 *
 *  ����� ���������� �������� ����������,
 *  �������� ��������� ������ � ����.
 *  �������������� ���������
 *
 */


#ifndef __HEAD_H
#define __HEAD_H


// �������� � TSS
#define TL     0x00
#define ESP0   0x04
#define SS0    0x08
#define ESP1   0x0c
#define SS1    0x10
#define ESP2   0x14
#define SS2    0x18
#define CR3    0x1c
#define EIP    0x20
#define EFLAGS 0x24
#define EAX    0x28
#define ECX    0x2c
#define EDX    0x30
#define EBX    0x34
#define ESP    0x38
#define EBP    0x3c
#define ESI    0x40
#define EDI    0x44
#define ES     0x48
#define CS     0x4c
#define SS     0x50
#define DS     0x54
#define FS     0x58
#define GS     0x5c
#define LDT    0x60
#define IOM    0x64


// �������� TSS'� ������ ������
#define MAIN_TSS 0x28

// �������� ����� syscall_stack � TaskStruct
#define END_OF_SYSCALL_STACK 0x100

// �������� TSS'� ������������� ����������
#define II_TSS 0x30
// �������� TSS'� ������������
#define IRQ0_TSS 0x38
// �������� TSS'� ����������� irq6
#define IRQ6_TSS 0x40

// �������� ���� ����
#define KERNEL_CS 0x08
// �������� ������ ����
#define KERNEL_DS 0x10

// �������� ���� ������������
#define USER_CS 0x1b
// �������� ������ ������������
#define USER_DS 0x23



#endif // __HEAD_H
