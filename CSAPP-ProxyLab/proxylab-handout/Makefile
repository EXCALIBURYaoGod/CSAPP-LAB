# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: test proxy hex2dd dd2hex

test.o: test.c csapp.h
	$(CC) $(CFLAGS) -c test.c

hex2dd.o: hex2dd.c csapp.h
	$(CC) $(CFLAGS) -c hex2dd.c

dd2hex.o: dd2hex.c csapp.h 
	$(CC) $(CFLAGS) -c dd2hex.c

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o
	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

hex2dd: hex2dd.o csapp.o
	$(CC) $(CFLAGS) hex2dd.o csapp.o -o hex2dd $(LDFLAGS)

dd2hex:	dd2hex.o csapp.o 
	$(CC) $(CFLAGS) -o dd2hex dd2hex.o csapp.o $(LDFLAGS)

test: test.o csapp.o
	$(CC) $(CFLAGS) -o test test.o csapp.o $(LDFLAGS)

# Creates a tarball in ../proxylab-handin.tar that you can then
# hand in. DO NOT MODIFY THIS!
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o  test proxy core hex2dd dd2hex *.tar *.zip *.gzip *.bzip *.gz

