#
# -= HelloOS Educational Project =-
# -===============================-
#
#  $Id$
#
#  �������� makefile �������
#
#  �������� �������� �����:
#
#	image:	������� ����� 3.5"-�������, ���� ����������
#		����������� ������, ����, ��������� ���� � ����
#		���������������� ����������
#
#  install:	���������� �� ������� �� �� �����, �������
#  	���������� � �����, ������ �� ������ �� �����������.
#  	(��. ��������� � ������������ � ����� boot.S)
#
#  emu:		������� ����� � ��������� ��� � ��������� Bochs
#  	(!! ��������� ���������� ���� � bios � emulate.txt !!)
#
#  qemu:		������� ����� � ��������� ��� � ��������� QEmu
#
#  clean:	���������� ��������� ��� ��������� � ���������
#  	�����
#
#  depend:	������������ ����������� �������� ������
#
#

VER_MAJOR = 0
VER_MINOR = 0
VER_ALPHA = f


.EXPORT_ALL_VARIABLES:

CFLAGS = -O2 -ffreestanding -W -Wall -save-temps -I./include/ -I../include/ -I./config/ -I../config/ \
			-DVER_MAJOR=$(VER_MAJOR) -DVER_MINOR=$(VER_MINOR) -DVER_ALPHA=\'$(VER_ALPHA)\'
# -Werror
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
	dd if=boot.bin of=/dev/fd0
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
