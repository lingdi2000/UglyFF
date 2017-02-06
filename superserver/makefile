
.SUFFIXES:
.SUFFIXES: .cpp .o

CPE=g++
target=superserver
INCLUDE=../include
LIBDIR=/usr/local/lib
INCOBJ=$(INCLUDE)/zSocket.o $(INCLUDE)/zLogger.o $(INCLUDE)/zEpoll.o $(INCLUDE)/zTCPServer.o $(INCLUDE)/zNetService.o $(INCLUDE)/zService.o $(INCLUDE)/zThread.o $(INCLUDE)/zTCPTaskPool.o $(INCLUDE)/zTCPTask.o


INCFLAG=-I$(INCLUDE) 
CFLAGS=-c -g -std=c++11 -Wall
LIBS=-L$(LIBDIR) -llog4cplus -lpthread


OBJ= superserver.o ServerTask.o ServerManager.o 
all:$(target)

$(target):$(OBJ)
	$(CPE) $^ $(INCOBJ)  $(LIBS)  -o $@
.cpp.o:
	$(CPE) $(CFLAGS) $(INCFLAG) $< -o $@

.PHONY:clean
clean:
	rm -f *.o
	rm -f $(target)


