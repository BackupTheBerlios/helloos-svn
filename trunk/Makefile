#
# -= HelloOS Educational Project =-
# -===============================-
#
#  $Id$
#
#  Основной makefile проекта
#
#  Описание основных целей:
#
#  all: скомпилировать загрузчик и ядро
#    Загрузчик компилируется в файл boot.bin, а
#    ядро в файл hello.
#
#  depend: генерирует зависимости и записывает
#    их в файл depend, который включается в этот
#    makefile. Эта цель вызывается из make all.
#    При компиляции "с нуля" может возникнуть
#    ошибка типа "depend: No such file", но тем
#    не менее, компиляция должна продолжиться и
#    завершиться успешно.
# 
#  clean: удалить все временные и бинарные файлы
#
#  install: записывает на дискету загрузчик и ядро,
#    причем НЕ стирая ее содержимого. (см. замечание
#    в комментариях к файлу boot.S)
#
#  image: создает минимальный образ дискеты с системой.
#    Создается файл image с образом чистой дискеты,
#    с записанным загрузчиком. На этот образ копируется
#    ядро.
#
#  emu:	 создает образ дискеты и запускает эмулятор
#    bochs с конфигурационным файлом emulate.txt.
#


CFLAGS = -O2 -ffreestanding -W -Wall -save-temps
# -Werror

KERNEL_OBJS = head.o scheduler.o fd.o panic.o hello_stdio.o hello_string.o fat.o pager.o syscall.o aout.o


all: boot.bin hello test.out gp.out start.out

boot.bin: boot.o
	ld86 -d -s -o $@ $<

boot.o: boot.S
	as86 -0 -a -o $@ $<


hello: $(KERNEL_OBJS)
	ld -M -Ttext 0x5000 -e startup_32 --oformat=binary $(KERNEL_OBJS) -o $@ >map.txt


.c.o:
	gcc $(CFLAGS) -c $<

.S.o:
	gcc $(CFLAGS) -c $<


AOUT_CFLAGS = -nostdlib -nostartfiles $(CFLAGS)

###################################

START_OBJS = hello_string.aout.o

start.out: start.aout.o mymain.o $(START_OBJS)
	i386-aout-ld -M -o $@ start.aout.o mymain.o $(START_OBJS) -e _start_my_kernel >map.start.txt

start.aout.o: start.c
	i386-aout-gcc -o $@ $(AOUT_CFLAGS) -c $<


hello_string.aout.o: hello_string.c
	i386-aout-gcc -o $@ $(AOUT_CFLAGS) -c $<


###################################

test.out: mymain.o test.o
	i386-aout-ld -M -o $@ test.o mymain.o -e _main >map.aout.txt

test.o: test.c
	i386-aout-gcc $(AOUT_CFLAGS) -c $< -save-temps

gp.out: mymain.o gp.o
	i386-aout-ld -M -o $@ gp.o mymain.o -e _main >map.aout.txt

gp.o: test.c
	i386-aout-gcc -DINVOKE_GP -o $@ $(AOUT_CFLAGS) -c $< -save-temps

mymain.o: mymain.c
	i386-aout-gcc -c $<



depend:
	for i in *.[cS]; do gcc -M $$i; done >depend


clean:
	rm -rf *.{i,s,o} *.bin hello image map.*txt *~ depend *.out


new: clean all


install: all
	dd if=boot.bin of=/dev/fd0
	mount /mnt/floppy
	cp -f hello /mnt/floppy
	cp -f start.out /mnt/floppy
	cp -f test.out /mnt/floppy
	cp -f gp.out /mnt/floppy
	umount /mnt/floppy


image: all
	dd if=/dev/zero of=image bs=512 count=2880
	chmod +x image
	dd if=boot.bin of=image bs=512 count=1 conv=notrunc
	mcopy -i image hello start.out test.out gp.out ::


emu: image
	bochs -qf emulate.txt

qemu: image
	qemu -fda image

include depend
