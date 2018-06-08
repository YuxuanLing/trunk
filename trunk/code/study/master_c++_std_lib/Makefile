CC = clang 

CPP_FLAGS = -lstdc++ -lpthread -std=c++14
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

all: lvalue references meaning stdmove std
	@echo "compile finished !!"

lvalue : lvalue.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)

references: references.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)

meaning: meaning.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)

stdmove: stdmove.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)

std: std.cpp 
	@echo "compiling $@ ......"
	$(CC) -c -o ./obj/$(addsuffix .o, $@) $^ 
	$(CC) -o ./bin/$@ $<  $(CPP_FLAGS)



run: lvalue
	$(shell ./bin/$^)

clean:
	-rm -fr $(MKDIRS)



