BINS = test cat ls xo waitvar setvar

all: $(BINS)

%: %.o
	ld -M -e main -o $@ $< ../lib/stdio.o ../lib/string.o >map.$<.txt

.c.o:
	gcc $(CFLAGS) -O1 -c $< -o $@


clean:
	rm -f *.{i,s,o} *~ $(BINS) *.o depend map.*.txt

depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend


include depend
