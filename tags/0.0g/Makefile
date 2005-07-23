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
#	image:	создает образ 3.5"-дискеты, куда включается
#		загрузочный сектор, ядро, стартовый файл и пара
#		демонстрационных бинарников
#
#  install:	записывает на дискету те же файлы, которые
#  	включаются в образ, причем НЕ стирая ее содержимого.
#  	(см. замечание в комментариях к файлу boot.S)
#
#  emu:		создает образ и запускает его в эмуляторе Bochs
#  	(!! поставьте правильные пути к bios в emulate.txt !!)
#
#  qemu:		создает образ и запускает его в эмуляторе QEmu
#
#  clean:	рекурсивно подчищает все бинарники и временные
#  	файлы
#
#  depend:	просчитывает зависимости исходных файлов
#
#

VER_MAJOR = 0
VER_MINOR = 0
VER_ALPHA = g


.EXPORT_ALL_VARIABLES:

CFLAGS = -O2 -ffreestanding -W -Wall -save-temps -I./include/ -I../include/ -I./config/ -I../config/ \
			-DVER_MAJOR=$(VER_MAJOR) -DVER_MINOR=$(VER_MINOR) -DVER_ALPHA=\'$(VER_ALPHA)\' -Werror
AOUT_CFLAGS = -nostdlib -nostartfiles $(CFLAGS)


SUBDIRS = boot lib kernel start user


all: image

new: clean all


subdirs:
	set -e ; for i in $(SUBDIRS); do make -C $$i; done


FILES = kernel/hello start/start.out user/test.out user/gp.out

image: subdirs
	dd if=/dev/zero of=image bs=512 count=2880
	chmod +x image
	dd if=boot/boot.bin of=image bs=512 count=1 conv=notrunc
	mcopy -i image $(FILES) ::


install: subdirs
	dd if=boot/boot.bin of=/dev/fd0
	mount /mnt/floppy
	cp -f $(FILES) /mnt/floppy
	umount /mnt/floppy


emu: image
	bochs -qf emulate.txt

qemu: image
	qemu -fda image


clean:
	for i in $(SUBDIRS); do make -C $$i clean; done
	rm -f image *~

depend:
	for i in $(SUBDIRS); do make -C $$i depend; done
