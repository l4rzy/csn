cc=gcc -g
cxx=g++
ld=gcc
csn=build/libcsn.so

test:
	$(cc) -DENABLE_DEBUG test/demo.c -I./inc $(csn) -o demo

clean:
	rm -f demo
	rm -f *.o

.PHONY: all test clean
