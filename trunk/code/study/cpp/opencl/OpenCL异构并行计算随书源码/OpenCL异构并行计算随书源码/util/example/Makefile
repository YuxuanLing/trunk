CC:=g++
NVCC:=nvcc
CFLAGS:= -O3 -Wall
NVCFLAGS:= -O3 -arch=sm_20 -lcublas -lcurand -lcufft -Xcompiler -Wall

all: testBMP testCutil testIo testPPMReadWrite testRandom testSplit testYMath testBMP maxGPU testAtomicFloat testCublasGemm testCublasGemv testCUDARandomize testCufft testTriangleSolver

testBMP:testBMP.cpp
	$(CC) $(CFLAGS) $< -o $@

testCutil:testCutil.cpp	
	$(CC) $(CFLAGS) $< -o $@

testIo:testIo.cpp
	$(CC) $(CFLAGS) $< -o $@

testPPMReadWrite:testPPMReadWrite.cpp
	$(CC) $(CFLAGS) $< -o $@

testRandom:testRandom.cpp
	$(CC) $(CFLAGS) $< -o $@

testSplit:testSplit.cpp 
	$(CC) $(CFLAGS) $< -o $@
	
testYMath:testYMath.cpp 
	$(CC) $(CFLAGS) $< -o $@

maxGPU:maxGPU.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

P2Pcopy:P2PCopy.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testAtomicFloat:testAtomicfloat.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testCublasGemm:testCublasGemm.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testCublasGemv:testCublasGemv.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testCUDARandomize:testCUDARandom.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testCufft:testCufft.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

testTriangleSolver:testTriangleSolver.cu
	$(NVCC) $(NVCFLAGS) $< -o $@

clean:
	rm ./testYMath;
	rm ./testSplit;
	rm ./testRandom;
	rm ./testPPMReadWrite;
	rm ./testIo;
	rm ./testCutil;
	rm ./testBMP;
