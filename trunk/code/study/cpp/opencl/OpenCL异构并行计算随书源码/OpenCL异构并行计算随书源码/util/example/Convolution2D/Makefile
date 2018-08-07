CC:=g++
DIR:=../../inc/ 
CFLAGS := -O3 -DPBL_USE_OPENCL -I/usr/local/cuda/include -lOpenCL -I$(DIR)  -L/usr/local/cuda-7.0/lib64

all : naive constant shared sharedBlocking

sharedBlocking : main_shared_blocking.cpp
	$(CC) $< -o $@ $(CFLAGS)

shared : main_shared.cpp
	$(CC) $< -o $@ $(CFLAGS)

constant : main_constant.cpp
	$(CC) $< -o $@ $(CFLAGS)

naive : main_naive.cpp
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -fr core.* .o naive constant shared sharedBlocking
