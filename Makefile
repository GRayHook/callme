-include .env

TG_TOKEN := $(strip $(TG_TOKEN))
TG_PROXY := $(strip $(TG_PROXY))

TG_TOKEN_DEF := -DTG_TOKEN='"$(TG_TOKEN)"'
TG_PROXY_DEF := -DTG_PROXY='"$(TG_PROXY)"'

main: libtgcommon.a
ifeq ($(strip $(TG_TOKEN)),)
	$(error TG_TOKEN is empty. Create .env (see .env.example) or run: make TG_TOKEN=...)
endif
	gcc -g callme.c -Wall $(TG_TOKEN_DEF) $(TG_PROXY_DEF) -L. -ltgcommon -lcurl -ljson-c -lpthread -o callme

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
