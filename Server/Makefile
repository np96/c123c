CPP = $(wildcard *.cpp)

INCL=-I ../lib

LIBS = -levent -lCommon -lhiredis

LIB_DIR = -L ../libs_out

all: make_lib make_project

make_lib:
	make -C ../lib

make_project:
	g++ -std=c++11 $(CPP) -Wall -o hello $(INCL) $(LIBS) $(LIB_DIR)
