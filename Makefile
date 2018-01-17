CFLAGS  = -ggdb3 -Wall

all: esvm-core.so

clean:
	rm *.so *.o *.a

esvm-core.so: esvm-core.o libsvm.a
	gcc -shared -o $@ $< libsvm.a

esvm-core.o: esvm-core.c
	gcc $(CFLAGS) -fPIC -c $< -o $@

libsvm.a: libsvm.o
	ar -cvq $@ $<

libsvm.so: libsvm.o
	gcc -shared -o $@ $<

libsvm.o: libsvm/svm.cpp
	gcc $(CFLAGS) -fPIC -c $< -o $@
