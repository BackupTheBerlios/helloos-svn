OBJS = head.o irq.o scheduler.o scrio.o fd.o panic.o fat.o pager.o syscall.o elf.o binfmt.o ipc.o
LIBOBJS = ../lib/string.o

all: hello

hello: $(OBJS) $(LIBOBJS)
	ld -M -Ttext 0x5000 -e HelloWorld --oformat=binary $(OBJS) $(LIBOBJS) -o $@ >map.txt

.c.o:
	gcc $(CFLAGS) -c $< -o $@

.S.o:
	gcc $(CFLAGS) -c $< -o $@

.S.s:
	gcc $(CFLAGS) -E $< -o $@


depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend

clean:
	rm -rf *.{i,s,o} hello map.txt *~ depend

include depend
