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
#  all:   скомпилировать загрузчик и ядро
#    Загрузчик компилируется в файл boot.bin, а
#    ядро в файл hello.
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


all: boot.bin hello bintest.bin

boot.bin: boot.o
	ld86 -d -s -o boot.bin boot.o

boot.o: boot.S
	as86 -0 -a -o boot.o boot.S


hello: head.o start.o scheduler.o fd.o panic.o hello_stdio.o hello_string.o fat.o
	ld -M -Ttext 0x1000 -e startup_32 --oformat=binary  head.o start.o scheduler.o fd.o panic.o hello_stdio.o hello_string.o fat.o -o hello >map.txt

head.o: head.S config.h
	gcc -c head.S

start.o: start.c io.h types.h config.h fat.h hello_stdio.h
	gcc $(CFLAGS) -c start.c

fd.o: fd.c fd.h io.h types.h config.h
	gcc $(CFLAGS) -c fd.c

panic.o: panic.c panic.h types.h config.h
	gcc $(CFLAGS) -c panic.c

hello_stdio.o: hello_stdio.c types.h config.h
	gcc $(CFLAGS) -c hello_stdio.c

hello_string.o: hello_string.c types.h config.h
	gcc $(CFLAGS) -c hello_string.c

fat.o: fat.c fat.h types.h fd.h config.h
	gcc $(CFLAGS) -c fat.c

scheduler.o: scheduler.c scheduler.h types.h panic.h config.h
	gcc $(CFLAGS) -c scheduler.c



#binload.o: binload.S
#	gcc -c binload.S

bintest.bin: bintest.S
	gcc -c bintest.S
	ld -Ttext 0x0000 -e start --oformat=binary bintest.o -o bintest.bin



clean:
	rm -rf *.{i,s,o} *.bin hello image map.txt *~


new: clean all


install: all
	dd if=boot.bin of=/dev/fd0
	mount /mnt/floppy
	cp -f hello /mnt/floppy
	cp -f bintest.bin /mnt/floppy
	umount /mnt/floppy


image: all
	dd if=/dev/zero of=image bs=512 count=2880
	chmod +x image
	dd if=boot.bin of=image bs=512 count=1 conv=notrunc
	mcopy -i image hello bintest.bin ::


emu: image
	bochs -qf emulate.txt

qemu: image
	qemu -fda image

