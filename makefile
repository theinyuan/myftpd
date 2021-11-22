myftp: myftp.o protocol.h
	gcc myftp.o protocol.h -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

myftpd: myftpd.o protocol.h
	gcc protocol.h myftpd.o -o myftpd

myftpd.o: myftpd.c
	gcc -c myftpd.c

clean:
	rm *.o