
.SUFFIXES:
.SUFFIXES: .cpp .o

CPE=g++
target=run
INCLUDE=../include
LIBDIR=/usr/local/lib
INCOBJ=$(INCLUDE)/zSocket.o $(INCLUDE)/zLogger.o $(INCLUDE)/zEpoll.o $(INCLUDE)/zTCPServer.o


INCFLAG=-I$(INCLUDE) 
CFLAGS=-c -g -std=c++11 -Wall
LIBS=-L$(LIBDIR) -llog4cplus


OBJ=test.o
all:$(target)

$(target):$(OBJ)
	$(CPE) $^ $(INCOBJ)  $(LIBS)  -o $@
.cpp.o:
	$(CPE) $(CFLAGS) $(INCFLAG) $< -o $@

.PHONY:clean
clean:
	rm -f *.o
	rm -f $(target)


