myftpd: myftpd.o protocol.h
	gcc protocol.h myftpd.o -o myftpd

myftpd.o: myftpd.c
	gcc -c myftpd.c

clean:
	rm *.o