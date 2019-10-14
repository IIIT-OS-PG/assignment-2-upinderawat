CC=g++
FLAG_T= -pthread -Wall
FLAG_P= -Wall
FLAG_GDB = -g

peer: readfiles.o user_handler.o group_handler.o
	$(CC) $(FLAG_P) -o peer.o peer.cpp readfiles.o user_handler.o group_handler.o
peer_gdb: readfiles.o user_handler.o group_handler.o
	$(CC) $(FLAG_P) $(FLAG_GDB) -o peer.o peer.cpp readfiles.o user_handler.o group_handler.o

tracker: readfiles.o
	$(CC) $(FLAG_T) -o tracker.o tracker.cpp readfiles.o

tracker_gdb: readfiles.o
	$(CC) $(FLAG_T) $(FLAG_GDB) -o tracker.o tracker.cpp readfiles.o


group_handler.o: group_handler.h
user_handler.o: user_handler.h

readfiles.o: readfiles.h

clean:
	rm *.o
	rm Metadata/Users/u_ids/*
	rm Metadata/Users/user_data/* -rf
	rm Metadata/Groups/g_ids/*
	rm Metadata/Groups/group_data/* -rf
	rm Metadata/Groups/g_ids_requests/*
