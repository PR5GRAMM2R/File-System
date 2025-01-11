CC = gcc

all : filesys

filesys : testcase.o fs.o fs_internal.o buf.o disk.o
	$(CC) -o a testcase.o fs.o fs_internal.o buf.o disk.o

testcase.o : 
	$(CC) -c testcase.c

fs.o : 
	$(CC) -c fs.c

fs_internal.o : 
	$(CC) -c fs_internal.c

buf.o :
	$(CC) -c buf.c

disk.o :
	$(CC) -c disk.c
