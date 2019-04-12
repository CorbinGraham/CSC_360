CFLAGS =-Wall -g -std=gnu11
File : apps/test.o
	gcc apps/test.c -o apps/test $(CFLAGS)
test.o : test.c ../apps/File.c
	gcc -c test.c $(CFLAGS)
clean :
	-rm -f test
	-rm -f test.o