main:
	gcc -g callme.c -Wall -o run

run: main
	./run.sh

clean:
	rm -f *.o *.a *.gch run
