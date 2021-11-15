myftpd: myftpd.o protocol.o
	gcc protocol.o myftpd.o -o myftpd

myftpd.o: myftpd.c protocol.h
	gcc -c myftpd.c

protocol.o: protocol.c protocol.h
	gcc -c protocol.c

clean:
	rm *.o