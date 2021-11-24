all: myftpd myftp

myftpd: myftpd.o protocol.o
	gcc protocol.o myftpd.o -o myftpd

myftp: myftp.o protocol.o
	gcc myftp.o protocol.o -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

myftpd.o: myftpd.c
	gcc -c myftpd.c

protocol.o: protocol.c protocol.h
	gcc -c protocol.c

clean:
	rm *.o