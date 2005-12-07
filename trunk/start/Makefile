OBJS = start.o ../lib/string.o ../lib/stdio.o

all: start

start: $(OBJS)
	ld -M -o $@ $(OBJS) -e main >map.txt

.c.o:
	gcc $(CFLAGS) -c $< -o $@


clean:
	rm -f *.{i,s,o} *~ start depend map.txt


depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend

include depend
