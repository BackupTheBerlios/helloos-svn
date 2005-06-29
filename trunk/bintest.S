/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 * Тестовый бинарник для загрузки. Мигает
 * символом, адрес которого записан в
 * 0x70000. При старте увеличивает адрес до
 * следующего символа.
 *
 * FIXME: Убрать это безобразие.
 *
 */

.text

.global start


mov $0x10, %ax
mov %ax, %fs

mov %fs:0x70000, %eax
mov %eax, position
add $2, %eax
mov %eax, %fs:0x70000

start:
   mov position, %ebx
   incb %fs:(%ebx)

   jmp start

position:
   .long 0x00000000
