cc=gcc
cxx=g++
ld=gcc
csn=build/libcsn.so

test:
	$(cc) -DENABLE_DEBUG test/demo.c -I./inc $(csn) -o demo
	$(cc) -DENABLE_DEBUG test/queue.c -I./inc $(csn) -o queue
	$(cc) -DENABLE_DEBUG test/buf.c -I./inc $(csn) -o buf
	$(cc) -DENABLE_DEBUG test/xpath.c -I./inc $(csn) -o xpath

clean:
	rm -f demo democpp queue buf
	rm -f *.o

.PHONY: all test clean
