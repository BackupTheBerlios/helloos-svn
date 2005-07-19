/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Системные константы
 *
 *  Здесь определены значения селекторов,
 *  смещений системных таблиц и проч.
 *  низкоуровневые константы
 *
 */


#ifndef __HEAD_H
#define __HEAD_H


// Смещения в TSS
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


// Селектор TSS'а первой задачи
#define MAIN_TSS 0x28

// Смещение конца syscall_stack в TaskStruct
#define END_OF_SYSCALL_STACK 0x100

// Селектор TSS'а игнорирования прерывания
#define II_TSS 0x30
// Селектор TSS'а планировщика
#define IRQ0_TSS 0x38
// Селектор TSS'а обработчика irq6
#define IRQ6_TSS 0x40

// Селектор кода ядра
#define KERNEL_CS 0x08
// Селектор данных ядра
#define KERNEL_DS 0x10

// Селектор кода пользователя
#define USER_CS 0x1b
// Селектор данных пользователя
#define USER_DS 0x23



#endif // __HEAD_H
