CC = clang 
TARGET = cpp_study 

CPP_FLAGS = -lstdc++ -std=c++11
CPP_FILES = $(shell ls *.cpp)
BASE = $(basename $(CPP_FILES))
OBJS = $(addsuffix .o, $(addprefix obj/,$(BASE)))
BINS = $(addprefix bin/,$(BASE))
BINS_ALL = $(BASE)
BINS_SINGLE_FILE = $(filter-out A_Complex_Num_Class,$(BINS_ALL)))
 
$(TARGET) : $(BINS)
#	-rm -f $@
#	$(CC) -o $(TARGET) $(OBJS) $(CPP_FLAGS)
			 
obj/%.o : %.cpp
	@if test ! -d "obj"; then\
		echo "mkdir obj";\
		mkdir -p obj;\
		fi;
	$(CC) -c -o $@ $<  $(CPP_FLAGS)

#A_Complex_Num_Class : A_Complex_Num_Class.o Complex.o
#	@if test ! -d "bin"; then\
#		echo "mkdir bin";\
#		mkdir -p bin;\
#		fi;
#	$(CC) -o ./bin/$@ ./obj/$^  $(CPP_FLAGS)
#
#
#$(BINS_SINGLE_FILE) : $(addsuffix .cpp, $(BINS_SINGLE_FILE))
#	@if test ! -d "bin"; then\
#		echo "mkdir bin";\
#		mkdir -p bin;\
#		fi;
#	$(CC) -o ./bin/$@ ./obj/$^  $(CPP_FLAGS)
#


bin/% : %.cpp
	@if test ! -d "bin"; then\
		echo "mkdir bin";\
		mkdir -p bin;\
		fi;
	$(CC) -o $@ $<  $(CPP_FLAGS)


clean:
	-rm -f obj/*
	-rm -f bin/*

