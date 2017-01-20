

.SUFFIXES:
.SUFFIXES:.cpp .o

CPE=g++
target=run
INCLUDE=
LIBS=


INCFLAG=-I$(INCLUDE)
CFLAGS=-c -g -std=c++11 -Wall

OBJS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))


all:$(OBJS)
.cpp.o:
	$(CPE) $(CFLAGS) $(INCLUDE) $< -o $@


.PHONY:debug
debug:
	@echo $(OBJS)


.PHONY:clean
clean:
	rm -f *.o
