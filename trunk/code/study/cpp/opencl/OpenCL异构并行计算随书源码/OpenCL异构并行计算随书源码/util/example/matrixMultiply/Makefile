CC:=g++
DIR:=../../inc/ 
CFLAGS := -O3 -DPBL_USE_OPENCL -I/usr/local/cuda/include -lOpenCL -I$(DIR)  -L/usr/local/cuda-7.0/lib64

all : naive shared sharedBlocking sharedFloat4 sharedFloat4Blocking sharedBlockingOpt

sharedBlockingOpt : main_sharedFloat4BlockingXY.cpp
	$(CC) $< -o $@ $(CFLAGS)

sharedFloat4Blocking : main_sharedFloat4Blocking.cpp
	$(CC) $< -o $@ $(CFLAGS)

sharedFloat4 : main_sharedFloat4.cpp
	$(CC) $< -o $@ $(CFLAGS)

sharedBlocking : main_sharedBlocking.cpp
	$(CC) $< -o $@ $(CFLAGS)

shared : main_shared.cpp
	$(CC) $< -o $@ $(CFLAGS)

naive : main_naive.cpp
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -fr core.* .o naive shared sharedBlocking
