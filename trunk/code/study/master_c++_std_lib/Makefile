CC = clang 

CPP_FLAGS = -lstdc++ -std=c++11
CPP_FILES = $(shell ls *.cpp)
BASE = $(basename $(CPP_FILES))
OBJS = $(addsuffix .o, $(addprefix obj/,$(BASE)))
BINS = $(addprefix bin/,$(BASE))
BINS_ALL = $(BASE)
BINS_SINGLE_FILE = $(filter-out A_Complex_Num_Class,$(BINS_ALL)))

MKDIRS := bin
MKDIRS += obj

default: directories all


$(sort $(MKDIRS)):
	-mkdir -p $@

directories: $(sort $(MKDIRS))

all: lvalue
	@echo "compile finished !!"

lvalue : lvalue.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)

run: lvalue
	$(shell ./bin/$^)

clean:
	-rm -fr $(MKDIRS)



