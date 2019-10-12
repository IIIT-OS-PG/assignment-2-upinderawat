CC=g++
FLAG_T= -pthread -Wall
FLAG_P= -Wall

peer: readfiles.o user_handler.o
	$(CC) $(FLAG_P) -o peer.o peer.cpp readfiles.o user_handler.o

tracker: readfiles.o
	$(CC) $(FLAG_T) -o tracker.o tracker.cpp readfiles.o

user_handler.o: user_handler.h

readfiles.o: readfiles.h

clean:
	rm *.o
