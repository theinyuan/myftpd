<<<<<<< HEAD
myftpd: myftpd.o protocol.o
	gcc protocol.o myftpd.o -o myftpd
=======
myftp: myftp.o protocol.h
	gcc myftp.o protocol.h -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

myftpd: myftpd.o protocol.h
	gcc protocol.h myftpd.o -o myftpd
>>>>>>> ddc2f594824eb23bdc912635dc68a8867208080b

myftpd.o: myftpd.c
	gcc -c myftpd.c

protocol.o: protocol.c protocol.h
	gcc -c protocol.c

clean:
	rm *.o