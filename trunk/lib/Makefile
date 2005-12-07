OBJS = string.o stdio.o

all: $(OBJS)

.c.o:
	gcc $(CFLAGS) -c $< -o $@

depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend

clean:
	rm -f *.{i,s,o} *~ *.o *.elf depend

include depend
