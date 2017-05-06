#ifndef H_CUDA_FUNCTION
#define H_CUDA_FUNCTION
/**
 * @file CUDAFunction.h
 * @author yyfn
 * @date 20120713
 *
 * @brief This file wrap almost all cuda functions,
 * just handler error, the default operation is exit if error occur.
 * doesn't contain graphics interop
 *
 * The only difference is the first letter is <b>uppercase</b>
 **/

#ifdef __CUDACC__

#include "checkCUDAError.h"

//device management function
#define CudaChooseDevice(device, prop)	checkCUDAError(cudaChooseDevice(device, prop)

#define CudaDeviceGetCacheConfig(pCacheConfig)	checkCUDAError(cudaDeviceGetCacheConfig(pCacheConfig))

#define CudaDeviceGetLimit(pValue, limit) checkCUDAError(cudaDeviceGetLimit(pValue,	limit)) 	

#define CudaDeviceReset()  checkCUDAError(cudaDeviceReset())  	

#define CudaDeviceSetCacheConfig(config) checkCUDAError(cudaDeviceSetCacheConfig(config)) 

#define CudaDeviceSetLimit(limit, value) checkCUDAError(cudaDeviceSetLimit(limit, value)) 	

#define CudaDeviceSynchronize() checkCUDAError(cudaDeviceSynchronize())  	

#define CudaGetDevice(deviceNo) {\
	checkCUDAError(cudaGetDevice(deviceNo));\
}

#define CudaGetDeviceCount(count) {\
	checkCUDAError(cudaGetDeviceCount(count));\
		if(0 == *count){\
		printf("file:%s, line:%d, error:no device", __FILE__, __LINE__);\
	}\
}

#define CudaGetDeviceProperties(prop, device) checkCUDAError(cudaGetDeviceProperties(prop, device)) 	

#define CudaSetDevice(device) checkCUDAError(cudaSetDevice(device))  	

#define CudaSetDeviceFlags(flags) checkCUDAError(cudaSetDeviceFlags(flags))

#define CudaSetValidDevices(arr, len) checkCUDAError(cudaSetValidDevices(arr, len)) 	

//error function
#define CudaGetLastError() checkCUDAError(cudaGetLastError())

#define CudaPeekAtLastError() checkCUDAError(cudaPeekAtLastError())

//stream function
#define CudaStreamCreate(streams) checkCUDAError(cudaStreamCreate(streams))

#define CudaStreamDestroy(stream) checkCUDAError(cudaStreamDestroy(stream))

#define CudaStreamQuery(stream) checkCUDAError(cudaStreamQuery(stream))

#define CudaStreamSynchronize(stream) checkCUDAError(cudaStreamSynchronize(stream))

#define CudaStreamWaitEvent(stream, event, flags) checkCUDAError(cudaStreamWaitEvent(stream, event, flags))

//event function
#define CudaEventCreate(event) checkCUDAError(cudaEventCreate(event))

#define CudaEventCreateWithFlags(event, flags) checkCUDAError(cudaEventCreateWithFlags(event, flags))

#define CudaEventDestroy(event) checkCUDAError(cudaEventDestroy(event))

#define CudaEventElapsedTime(ms, start, end) checkCUDAError(cudaEventElapsedTime(ms, start, end))

#define CudaEventQuery(event) checkCUDAError(cudaEventQuery(event))

#define CudaEventRecord(event, stream) checkCUDAError(cudaEventRecord(event, stream))

#define CudaEventSynchronize(event) checkCUDAError(cudaEventSynchronize(event))

//execution control function
#define CudaConfigureCall(grid, block, shared, stream) checkCUDAError(cudaConfigureCall(grid, block, shared, stream))

#define CudaFuncSetAttributes(attr, func) checkCUDAError(cudaFuncGetAttributes(attr, func))

#define CudaFuncSetCacheConfig(func, config) checkCUDAError(cudaFuncSetCacheConfig(func, config))

#define CudaLaunch(entry) checkCUDAError(cudaLaunch(entry))

#define CudaSetDoubleForDevice(d) checkCUDAError(cudaSetDoubleForDevice(d))

#define CudaSetDoubleForHost(d) checkCUDAError(cudaSetDoubleForHost(d))

#define CudaSetupArgument(arg, size, offset) checkCUDAError(cudaSetupArgument(arg, size, offset))

//memory 
#define CudaFree(ptr) checkCUDAError(cudaFree(ptr))

#define CudaFreeArray(array) checkCUDAError(cudaFreeArray(array))

#define CudaFreeHost(ptr) checkCUDAError(cudaFreeHost(ptr))

#define CudaGetSymbolAddress(ptr, symbol) checkCUDAError(cudaGetSymbolAddress(ptr, symbol))

#define CudaGetSymbolSize(size, symbol) checkCUDAError(cudaGetSymbolSize(size, symbol))

#define CudaHostAlloc(host, size, flags) checkCUDAError(cudaHostAlloc(host, size, flags))

#define CudaHostGetDevicePointer(dptr, hptr, flags) checkCUDAError(cudaHostGetDevicePointer(dptr, hptr, flags))

#define CudaHostGetFlags(flags, host) checkCUDAError(cudaHostGetFlags(flags,host))

#define CudaHostRegister(ptr, size, flags) checkCUDAError(cudaHostRegister(ptr, size, flags))

#define CudaHostUnregister(ptr) checkCUDAError(cudaHostUnregister(ptr))

#define CudaMalloc(ptr, size) checkCUDAError(cudaMalloc(ptr, size))

#define CudaMalloc3D(pitchedDevPtr, extent) checkCUDAError(cudaMalloc3D(pitchedDevPtr, extent))

#define CudaMalloc3DArray(array, desc, extent, flags) checkCUDAError(cudaMalloc3DArray(array, desc, extent, flags))

#define CudaMallocArray(array, desc, width, height, flags) checkCUDAError(cudaMallocArray(array, desc, width, height, flags))

#define CudaMallocHost(ptr, size) checkCUDAError(cudaMallocHost(ptr, size))

#define CudaMallocPitch(ptr, pitch, width, height) checkCUDAError(cudaMallocPitch(ptr, pitch, width, height))

#define CudaMemcpy(dst, src, count, kind) checkCUDAError(cudaMemcpy(dst, src, count, kind))

#define CudaMemcpy2D(dst, dpitch, src, spitch, width, height, kind) checkCUDAError(cudaMemcpy2D(dst, dpitch, src,spitch, width, height, kind))

#define CudaMemcpy2DArrayToArray(dst, wOffsetDst, hOffsetDst, src, wOffsetSrc, hOffsetSrc, width, height, kind) checkCUDAError(cudaMemcpy2DArrayToArray(dst, wOffsetDst, hOffsetDst, src, wOffsetSrc, hOffsetSrc, width, height, kind))

#define CudaMemcpy2DFromArray(dst, dpitch, src, wOffset, hOffset, width, height, kind) checkCUDAError(cudaMemcpy2DFromArray(dst, dpitch, src, wOffset, hOffset, width, height, kind))

#define CudaMemcpy2DToArray(dst, wOffset, hOffset, src, spitch, width, height, kind) checkCUDAError(cudaMemcpy2DToArray(dst, wOffset, hOffset, src, spitch, width, height, kind))

#define CudaMemcpy3D(p) checkCUDAError(cudaMemcpy3D(p))

#define CudaMemcpy3DPeer(p) checkCUDAError(cudaMemcpy3DPeer(p))

#define CudaMemcpyArrayToArray(dst, wOffsetDst, hOffsetDst, src, wOffsetSrc, hOffsetSrc, count, kind) checkCUDAError(cudaMemcpyArrayToArray(dst, wOffsetDst, hOffsetDst, src, wOffsetSrc, hOffsetSrc, count, kind))

#define CudaMemcpyFromArray(dst, src, wOffset, hOffset, count, kind) checkCUDAError(cudaMemcpyFromArray(dst, src, wOffset, hOffset, count, kind))

#define CudaMemcpyFromSymbol(dst, symbol, count, offset, kind) checkCUDAError(cudaMemcpyFromSymbol(dst, symbol, count, offset, kind))

#define CudaMemcpyPeer(dst, dstDevice, src, srcDevice, count) checkCUDAError(cudaMemcpyPeer(dst, dstDevice, src, srcDevice, count))

#define CudaMemcpyToArray(dst, wOffset, hOffset, src, count, kind) checkCUDAError(cudaMemcpyToArray(dst, wOffset, hOffset, src, count, kind))

#define CudaMemcpyToSymbol(symbol, src, count, offset, kind) checkCUDAError(cudaMemcpyToSymbol(symbol, src, count, offset, kind))

#define CudaMemGetInfo(freeMem, totol) checkCUDAError(cudaMemGetInfo(free, total))

#define CudaMemset(ptr, value, count)	{\
	checkCUDAError(cudaMemset(ptr, value, count));\
}

#define CudaMemset2D(ptr, pitch, value, width, height) {\
	checkCUDAError(cudaMemset2D(ptr, pitch, value, width, height));\
}

#define CudaMemset3D(pitchedDevPtr, value, extent) checkCUDAError(cudaMemset3D(pitchedDevPtr, value, extent))

//uva
#define CudaPointerGetAttributes(atrr, ptr) checkCUDAError(cudaPointerGetAttributes(attr, ptr))

#define CudaDeviceCanAccessPeer(can, device, peerDevice) checkCUDAError(cudaDeviceCanAccessPeer(can, device, peerDevice))

#define CudaDeviceDisablePeerAccess(peerDevice) checkCUDAError(cudaDeviceDisablePeerAccess(peerDevice))

#define CudaDeviceEnablePeerAccess(peerDevice, flags) checkCUDAError(cudaDeviceEnablePeerAccess(peerDevice, flags))

//texture
#define CudaBindTexture(offset, texRef, ptr, desc) checkCUDAError(cudaBindTexture(offset, texRef, ptr, desc))

#define CudaBindTexture2D(offset, texRef, ptr, desc, width, height, pitch) \
	checkCUDAError(cudaBindTexture2D(offset, texRef, ptr, desc, width, height, pitch))

#define CudaBindTextureToArray(texRef, array, desc) checkCUDAError(cudaBindTextureToArray(texRef, array, desc))

/*
 struct cudaChannelFormatDesc 	cudaCreateChannelDesc (int x, int y, int z, int w, enum cudaChannelFormatKind f)
 Returns a channel descriptor using the specified format.
 */

#define CudaGetChannelDesc(desc, array) checkCUDAError(cudaGetChannelDesc(desc, array))

#define CudaGetTextureAlignmentOffset(offset, texRef) checkCUDAError(cudaGetTextureAlignmentOffset(offset, texRef))

#define CudaGetTextureReference(texRef, symbol) checkCUDAError(cudaGetTextureReference(texRef, symbol))

#define CudaUnbindTexture(texRef) checkCUDAError(cudaUnbindTexture(texRef))

//surface
#define CudaBindSurfaceToArray(sr, array, desc) checkCUDAError(cudaBindSurfaceToArray(sr, array, desc))

#define CudaGetSurfaceReference(sr, symbol) checkCUDAError(cudaGetSurfaceReference(sr, symbol))

//version
#define CudaDriverGetVersion(version) checkCUDAError(cudaDriverGetVersion(version))

#define CudaRuntimeGetVersion(version) checkCUDAError(cudaRuntimeGetVersion(version))

/*
 //c++
 template<class T , int dim>
 cudaError_t 	cudaBindSurfaceToArray (const struct surface< T, dim > &surf, const struct cudaArray *array)
 [C++ API] Binds an array to a surface
 template<class T , int dim>
 cudaError_t 	cudaBindSurfaceToArray (const struct surface< T, dim > &surf, const struct cudaArray *array, const struct cudaChannelFormatDesc &desc)
 [C++ API] Binds an array to a surface
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTexture (size_t *offset, const struct texture< T, dim, readMode > &tex, const void *devPtr, size_t size=UINT_MAX)
 [C++ API] Binds a memory area to a texture
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTexture (size_t *offset, const struct texture< T, dim, readMode > &tex, const void *devPtr, const struct cudaChannelFormatDesc &desc, size_t size=UINT_MAX)
 [C++ API] Binds a memory area to a texture
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTexture2D (size_t *offset, const struct texture< T, dim, readMode > &tex, const void *devPtr, size_t width, size_t height, size_t pitch)
 [C++ API] Binds a 2D memory area to a texture
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTexture2D (size_t *offset, const struct texture< T, dim, readMode > &tex, const void *devPtr, const struct cudaChannelFormatDesc &desc, size_t width, size_t height, size_t pitch)
 [C++ API] Binds a 2D memory area to a texture
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTextureToArray (const struct texture< T, dim, readMode > &tex, const struct cudaArray *array)
 [C++ API] Binds an array to a texture
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaBindTextureToArray (const struct texture< T, dim, readMode > &tex, const struct cudaArray *array, const struct cudaChannelFormatDesc &desc)
 [C++ API] Binds an array to a texture
 template<class T >
 cudaChannelFormatDesc 	cudaCreateChannelDesc (void)
 [C++ API] Returns a channel descriptor using the specified format
 cudaError_t 	cudaEventCreate (cudaEvent_t *event, unsigned int flags)
 [C++ API] Creates an event object with the specified flags
 template<class T >
 cudaError_t 	cudaFuncGetAttributes (struct cudaFuncAttributes *attr, T *entry)
 [C++ API] Find out attributes for a given function
 template<class T >
 cudaError_t 	cudaFuncSetCacheConfig (T *func, enum cudaFuncCache cacheConfig)
 Sets the preferred cache configuration for a device function.
 template<class T >
 cudaError_t 	cudaGetSymbolAddress (void **devPtr, const T &symbol)
 [C++ API] Finds the address associated with a CUDA symbol
 template<class T >
 cudaError_t 	cudaGetSymbolSize (size_t *size, const T &symbol)
 [C++ API] Finds the size of the object associated with a CUDA symbol
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaGetTextureAlignmentOffset (size_t *offset, const struct texture< T, dim, readMode > &tex)
 [C++ API] Get the alignment offset of a texture
 template<class T >
 cudaError_t 	cudaLaunch (T *entry)
 [C++ API] Launches a device function
 cudaError_t 	cudaMallocHost (void **ptr, size_t size, unsigned int flags)
 [C++ API] Allocates page-locked memory on the host
 template<class T >
 cudaError_t 	cudaSetupArgument (T arg, size_t offset)
 [C++ API] Configure a device launch
 template<class T , int dim, enum cudaTextureReadMode readMode>
 cudaError_t 	cudaUnbindTexture (const struct texture< T, dim, readMode > &tex)
 */
int getCudaCorePerSM(int device) {
	cudaDeviceProp dp;
	CudaGetDeviceProperties(&dp, device);
	int ret;
	if (dp.major >= 3) {
		ret = 192;
	} else if (dp.major > 2 && dp.minor >= 1) {
		ret = 48;
	} else if (dp.major >= 2) {
		ret = 32;
	} else {
		ret = 8;
	}

	return ret;
}

int getMaxFlopsGPU() {
	int deviceCount;
	CudaGetDeviceCount(&deviceCount);
	if (deviceCount <= 0) {
		printf("no device\n");
		exit(0);
	}
	float flops = 0.0f;
	int ret;
	for (int i = 0; i < deviceCount; i++) {
		cudaDeviceProp dp;
		CudaGetDeviceProperties(&dp, i);
		float tempf = dp.clockRate * dp.multiProcessorCount
				* getCudaCorePerSM(i);
		if (tempf > flops) {
			flops = tempf;
			ret = i;
		}
	}

	return ret;
}

int getMaxFlopsGPUWithCC(int major, int minor) {
	int deviceCount;
	CudaGetDeviceCount(&deviceCount);
	if (deviceCount <= 0) {
		printf("no device\n");
		exit(0);
	}
	float flops = 0.0f;
	int ret = -1;
	for (int i = 0; i < deviceCount; i++) {
		cudaDeviceProp dp;
		CudaGetDeviceProperties(&dp, i);
		if (dp.major > major || (dp.major == major && dp.minor >= minor)) {
			float tempf = dp.clockRate * dp.multiProcessorCount
					* getCudaCorePerSM(i);
			if (tempf > flops) {
				flops = tempf;
				ret = i;
			}
		}
	}

	if (-1 == ret) {
		printf("no device's CC >= %d.%d\n", major, minor);
		exit(0);
	}

	return ret;
}

void listDeviceInfo(int device) {
	cudaSetDevice(device);
	cudaDeviceProp deviceProp;
	CudaGetDeviceProperties(&deviceProp, device);

	printf("\nDevice %d: \"%s\"\n", device, deviceProp.name);

#if CUDART_VERSION >= 2020
	int driverVersion, runtimeVersion;
	CudaDriverGetVersion(&driverVersion);
	CudaRuntimeGetVersion(&runtimeVersion);
	printf("  CUDA Driver Version / Runtime Version          %d.%d / %d.%d\n", driverVersion/1000, (driverVersion%100)/10, runtimeVersion/1000, (runtimeVersion%100)/10);
#endif
	printf("  CUDA Capability Major/Minor version number:    %d.%d\n",
			deviceProp.major, deviceProp.minor);

	char msg[256];
	sprintf(msg,"  Total amount of global memory: %.0f MBytes (%llu bytes)\n",(float) deviceProp.totalGlobalMem / 1048576.0f,
			(unsigned long long) deviceProp.totalGlobalMem);
	printf(msg);
#if CUDART_VERSION >= 2000
	printf("  (%2d) Multiprocessors x (%3d) CUDA Cores/MP:    %d CUDA Cores\n",
			deviceProp.multiProcessorCount,
			getCudaCorePerSM(device),
			getCudaCorePerSM(device) * deviceProp.multiProcessorCount);
#endif
	printf(
			"  GPU Clock rate:                                %.0f MHz (%0.2f GHz)\n",
			deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
#if CUDART_VERSION >= 4000
	printf("  Memory Clock rate:                             %.0f Mhz\n", deviceProp.memoryClockRate * 1e-3f);
	printf("  Memory Bus Width:                              %d-bit\n", deviceProp.memoryBusWidth);
	printf("  L2 Cache Size:                                 %d bytes\n", deviceProp.l2CacheSize);

	printf("  Max Texture Dimension Size (x,y,z)             1D=(%d), 2D=(%d,%d), 3D=(%d,%d,%d)\n",
			deviceProp.maxTexture1D, deviceProp.maxTexture2D[0], deviceProp.maxTexture2D[1],
			deviceProp.maxTexture3D[0], deviceProp.maxTexture3D[1], deviceProp.maxTexture3D[2]);
	printf("  Max Layered Texture Size (dim) x layers        1D=(%d) x %d, 2D=(%d,%d) x %d\n",
			deviceProp.maxTexture1DLayered[0], deviceProp.maxTexture1DLayered[1],
			deviceProp.maxTexture2DLayered[0], deviceProp.maxTexture2DLayered[1], deviceProp.maxTexture2DLayered[2]);
#endif
	printf("  Total amount of constant memory:               %ld bytes\n", deviceProp.totalConstMem);
	printf("  Total amount of shared memory per block:       %ld bytes\n", deviceProp.sharedMemPerBlock);
	printf("  Total number of registers available per block: %d\n",
			deviceProp.regsPerBlock);
	printf("  Warp size:                                     %d\n",
			deviceProp.warpSize);
	printf("  Maximum number of threads per multiprocessor:  %d\n",
			deviceProp.maxThreadsPerMultiProcessor);
	printf("  Maximum number of threads per block:           %d\n",
			deviceProp.maxThreadsPerBlock);
	printf("  Maximum sizes of each dimension of a block:    %d x %d x %d\n",
			deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1],
			deviceProp.maxThreadsDim[2]);
	printf("  Maximum sizes of each dimension of a grid:     %d x %d x %d\n",
			deviceProp.maxGridSize[0], deviceProp.maxGridSize[1],
			deviceProp.maxGridSize[2]);
	printf("  Maximum memory pitch:                          %ld bytes\n", deviceProp.memPitch);
	printf("  Texture alignment:                             %ld bytes\n", deviceProp.textureAlignment);

#if CUDART_VERSION >= 4000
	printf("  Concurrent copy and execution:                 %s with %d copy engine(s)\n", (deviceProp.deviceOverlap ? "Yes" : "No"), deviceProp.asyncEngineCount);
#else
	printf("  Concurrent copy and execution:                 %s\n",
			deviceProp.deviceOverlap ? "Yes" : "No");
#endif

#if CUDART_VERSION >= 2020
	printf("  Run time limit on kernels:                     %s\n", deviceProp.kernelExecTimeoutEnabled ? "Yes" : "No");
	printf("  Integrated GPU sharing Host Memory:            %s\n", deviceProp.integrated ? "Yes" : "No");
	printf("  Support host page-locked memory mapping:       %s\n", deviceProp.canMapHostMemory ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3000
	printf("  Concurrent kernel execution:                   %s\n", deviceProp.concurrentKernels ? "Yes" : "No");
	printf("  Alignment requirement for Surfaces:            %s\n", deviceProp.surfaceAlignment ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3010
	printf("  Device has ECC support enabled:                %s\n", deviceProp.ECCEnabled ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 3020
	printf("  Device is using TCC driver mode:               %s\n", deviceProp.tccDriver ? "Yes" : "No");
#endif
#if CUDART_VERSION >= 4000
	printf("  Device supports Unified Addressing (UVA):      %s\n", deviceProp.unifiedAddressing ? "Yes" : "No");
	printf("  Device PCI Bus ID / PCI location ID:           %d / %d\n", deviceProp.pciBusID, deviceProp.pciDeviceID);
#endif

#if CUDART_VERSION >= 2020
	const char *sComputeMode[] =
	{
		"Default (multiple host threads can use ::cudaSetDevice() with device simultaneously)",
		"Exclusive (only one host thread in one process is able to use ::cudaSetDevice() with this device)",
		"Prohibited (no host thread can use ::cudaSetDevice() with this device)",
		"Exclusive Process (many threads in one process is able to use ::cudaSetDevice() with this device)",
		"Unknown",
		NULL
	};
	printf("  Compute Mode:\n");
	printf("     < %s >\n", sComputeMode[deviceProp.computeMode]);
#endif
}

#endif

#endif
