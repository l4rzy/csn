target='libcsn.so'
cc=gcc
ld=gcc
cflags=-DENABLE_DEBUG -Wall -c -fPIC -I./inc
ldflags=-shared -lcurl -ltidy

all:
	$(cc) $(cflags) src/csn.c -o csn.o
	$(cc) $(cflags) src/util.c -o util.o
	$(cc) $(cflags) src/error.c -o error.o
	
	$(ld) $(ldflags) csn.o util.o error.o -o $(target)

test:
	$(cc) test/test.c -I./inc libcsn.so -o csn

clean:
	rm -f csn
	rm -f *.o
	rm -f *.so

.PHONY: all test clean
