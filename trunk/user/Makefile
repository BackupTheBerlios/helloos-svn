all: test.out gp.out

MYMAIN = ../lib/mymain.aout

test.out: test.aout $(MYMAIN)
	i386-aout-ld -M -o $@ test.aout $(MYMAIN) -e _main >map.test.txt

gp.out: gp.aout $(MYMAIN)
	i386-aout-ld -M -o $@ gp.aout $(MYMAIN) -e _main >map.gp.txt


test.aout: test.c
	i386-aout-gcc $(AOUT_CFLAGS) -c $< -o $@

gp.aout: test.c
	i386-aout-gcc $(AOUT_CFLAGS) -DINVOKE_GP -c $< -o $@


clean:
	rm -f *.{i,s,o} *~ *.aout *.out depend map.*.txt

depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend


include depend
