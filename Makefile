ROOT    = /home/lepisma/Desktop/emacs-25.3
CC      = gcc
LD      = gcc
CFLAGS  = -ggdb3 -Wall
LDFLAGS =

all: libsvm.so mat.so

clean:
	rm *.so *.o

mat.so: mat.o
	$(LD) -shared $(LDFLAGS) -o mat.so libsvm.so mat.o

mat.o: mat.c
	$(CC) $(CFLAGS) -I$(ROOT)/src -fPIC -c mat.c

libsvm.so: libsvm.o
	$(LD) -shared $(LDFLAGS) -o libsvm.so libsvm.o

libsvm.o: svm.cpp
	$(CC) $(CFLAGS) -I$(ROOT)/src -fPIC -c svm.cpp -o libsvm.o
