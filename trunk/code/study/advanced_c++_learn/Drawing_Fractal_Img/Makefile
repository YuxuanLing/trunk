CC = clang 
TARGET = fractal_image

CPP_FLAGS = -lstdc++ -lm -std=c++11
CPP_FILES = $(shell ls *.cpp)
BASE = $(basename $(CPP_FILES))
OBJS = $(addsuffix .o, $(addprefix obj/,$(BASE)))
BINS = $(addprefix bin/,$(BASE))
 
$(TARGET) : $(OBJS)
	-rm -f $@
	$(CC) -o $(TARGET) $(OBJS) $(CPP_FLAGS)
			 
obj/%.o : %.cpp
	@if test ! -d "obj"; then\
		echo "mkdir obj";\
		mkdir -p obj;\
		fi;
	$(CC) -c -o $@ $< -Wno-pragma-pack 

clean:
	-rm -f obj/*
	-rm -f bin/*

