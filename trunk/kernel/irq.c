/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции для работы с прерываниями IRQ
 *  Пока здесь только одна функция для переназначения
 *  IRQ. Вызывается одной из первых при старте.
 *
 */

#include <helloos/types.h>
#include <helloos/io.h>
#include <helloos/head.h>
#include <helloos/scrio.h>


// Заимствовано из [10]
#define CMD1   0x20
#define DATA1  0x21
#define CMD2   0xa0
#define DATA2  0xa1

#define ICW1_ICW4       0x01     // ICW4 (not) needed
#define ICW1_SINGLE     0x02     // Single (cascade) mode
#define ICW1_INTERVAL4  0x04     // Call address interval 4 (8)
#define ICW1_LEVEL      0x08     // Level triggered (edge) mode
#define ICW1_INIT       0x10     // Initialization - required!

#define ICW4_8086       0x01     // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02     // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08     // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C     // Buffered mode/master
#define ICW4_SFNM       0x10     // Special fully nested (not)



// По умолчанию IRQ0 = int 0x8 и IRQ8 = int 0x70
// Мы переназначаем их на int 0x20 и int 0x28, то есть
// IRQ0..IRQ16 = int 0x20..int 0x30
void remap_irqs(uint pic1, uint pic2)
{
   uchar a1, a2;

   a1 = inb_p(DATA1);
   a2 = inb_p(DATA2);

   outb_p(ICW1_INIT+ICW1_ICW4, CMD1);
   outb_p(ICW1_INIT+ICW1_ICW4, CMD2);

   outb_p(pic1, DATA1);
   outb_p(pic2, DATA2);

   outb_p(4, DATA1);
   outb_p(2, DATA2);

   outb_p(ICW4_8086, DATA1);
   outb_p(ICW4_8086, DATA2);

   outb_p(a1, DATA1);
   outb_p(a2, DATA2);
}


// Инициализация PIC
void irq_init()
{
   puts_color("Remapping IRQs...\t\t", 0x0b);
   remap_irqs(IRQ0_INT, IRQ8_INT);
   printf_color(0x0a, "IRQ0 = int 0x%x, IRQ8 = int 0x%x\n", IRQ0_INT, IRQ8_INT);
}
