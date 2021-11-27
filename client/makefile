myftp: myftp.o ../protocol.o
	gcc protocol.o myftp.o -o myftp

myftp.o: myftp.c
	gcc -c myftp.c

../protocol.o: ../protocol.c ../protocol.h
	gcc -c ../protocol.c

clean:
	rm *.o