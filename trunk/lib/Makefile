OBJS = string.o
AOUT_OBJS = string.aout mymain.aout

all: $(OBJS) $(AOUT_OBJS)

.c.o:
	gcc $(CFLAGS) -c $< -o $@


string.aout: string.c
	i386-aout-gcc $(AOUT_CFLAGS) -c $< -o $@

mymain.aout: mymain.c
	i386-aout-gcc $(AOUT_CFLAGS) -c $< -o $@

depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend

clean:
	rm -f *.{i,s,o} *~ *.aout depend

include depend
