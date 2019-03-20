target='libcsn.so'
cc=gcc
cxx=g++
ld=gcc
cflags=-DENABLE_DEBUG -Wall -c -fPIC -I./inc
ldflags=-shared -lcurl -ltidy

all:
	$(cc) $(cflags) src/csn.c -o csn.o
	$(cc) $(cflags) src/parser.c -o parser.o
	$(cc) $(cflags) src/util.c -o util.o

	$(ld) $(ldflags) csn.o util.o parser.o -o $(target)

test:all
	$(cc) test/test.c -I./inc libcsn.so -o csn
	$(cxx) test/test.cpp -I./inc libcsn.so -o csnpp

clean:
	rm -f csn
	rm -f *.o
	rm -f *.so

.PHONY: all test clean
