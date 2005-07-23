OBJS = start.aout ../lib/string.aout ../lib/mymain.aout

all: start.out

start.out: $(OBJS)
	i386-aout-ld -M -o $@ $(OBJS) -e _main >map.txt

start.aout: start.c
	i386-aout-gcc $(AOUT_CFLAGS) -c $< -o $@

clean:
	rm -f *.{i,s,o} *~ *.aout *.out depend map.txt


depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend

include depend
