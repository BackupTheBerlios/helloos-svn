all: boot.bin

boot.bin: boot.o
	ld86 -d -s -o $@ $<

boot.o: boot.S
	as86 -0 -a -o $@ $<

clean:
	rm -f *.o *~ *.bin
