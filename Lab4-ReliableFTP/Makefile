#make file for the project

CC = g++
CFLAGS = -c
WFLAG = -Wall
LFLAG = -pthread
SFLAG = -I/home/scf-22/csci551b/openssl/include -L/home/scf-22/csci551b/openssl/lib
DFLAG = -g
OBJS1 = server.o
OBJS2 = client.o
OBJSCOMMON = commonFunctions.o 

all: server.exe client.exe

server.exe: $(OBJSCOMMON) $(OBJS1)
		$(CC) $(OBJSCOMMON) $(OBJS1) -g -lcrypto -o receiver $(LFLAG)

client.exe: $(OBJSCOMMON) $(OBJS2)
		$(CC) $(OBJSCOMMON) $(OBJS2) -g -lcrypto -o sender $(LFLAG)
		
commonFunctions.o: common.h
		$(CC) $(CFLAGS) -lcrypto -g -lcrypto commonFunctions.cpp

server.o: server.h
		$(CC) $(CFLAGS) -g -lcrypto server.cpp
		
client.o: client.h
		$(CC) $(CFLAGS) -g -lcrypto client.cpp

clean:
		rm -f receiver sender *.o *~ *.log

