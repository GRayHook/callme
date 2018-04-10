main: libtgcommon.a
	gcc -g callme.c -Wall -L. -ltgcommon -lcurl -ljson-c -lpthread -o callme

libtgcommon.a: tgcommon.o
	ar cr libtgcommon.a tgcommon.o

tgcommon.o: libtg/tgcommon.c
	gcc -g -Wall -lcurl -ljson-c -c libtg/tgcommon.c -lpthread -o tgcommon.o

run: main
	./run.sh

install: main
	sudo cp callme /usr/local/bin/callme

clean:
	rm -f *.o *.a *.gch callme
