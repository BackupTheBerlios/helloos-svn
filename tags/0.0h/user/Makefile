all: test.elf gp.elf pf.elf et.elf

%.elf: %.o
	ld -M -e main -o $@ $< >map.$<.txt

gp.o: test.c
	gcc $(CFLAGS) -O1 -DINVOKE_GP -c $< -o $@

pf.o: test.c
	gcc $(CFLAGS) -O1 -DINVOKE_PF -c $< -o $@


et.elf: et.o
	ld -e main et.o -o et.elf

.c.o:
	gcc $(CFLAGS) -O1 -c $< -o $@


clean:
	rm -f *.{i,s,o} *~ *.elf *.o depend map.*.txt

depend:
	for i in *.[cS]; do gcc -M $(CFLAGS) $$i; done >depend


include depend
