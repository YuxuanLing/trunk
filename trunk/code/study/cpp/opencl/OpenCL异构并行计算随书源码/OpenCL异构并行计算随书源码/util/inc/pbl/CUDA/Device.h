/**
 * @file
 * @author yyfn
 * @date 20120724
 *
 * @brief class Device will list all iformation a nvidia gpu has.
 * There are two interface for every function, one static and one nostatic.
 * This class also list all nvidia gpu available
 **/

#ifdef __CUDACC__

#pragma once
#include "CUDAFunction.h"
#include "Env.h"
#include <vector>

/**
 *
 * @class
 *
 * @brief stand for a gpu
 * this class have interface of two types, one is static, other is no-static 
 * static interface has a parameter. no-static interface has no parameter.
 *
 **/
class Device{
private:
	cudaDeviceProp prop;
	int deviceNo;

public:
/**
 * @brief get deivce number
 *
 **/
	static int getDeviceCount(){
		int deviceCount;
		CudaGetDeviceCount(&deviceCount);
	
		return deviceCount;
	}
/**
 *@brief get all device
 *
 */
	static std::vector<Device> getAllDevice(){
		int deviceCount = Device::getDeviceCount();

		Device d;
		std::vector<Device> deviceVector;

		for(int i = 0; i < deviceCount; i++){
			d.deviceNo = i;
			CudaGetDeviceProperties(&(d.prop), d.deviceNo);

			deviceVector.push_back(d);
		}

		return deviceVector;
	}
/**
 * @brief get streaming multiprocessor number
 *
 **/
	static int getSMNumber(int deviceNo){
		int deviceCount = Device::getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);
		return prop.multiProcessorCount;
	}
/**
 *@brief get compute capability of device
 *
 **/
	 static int getComputeCapability(int deviceNo){
		 int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);
		return prop.major*100 + prop.minor*10;
	}

/**
 * @brief get total global memory capacity in Mega
 *
 **/
	static int getTotalGlobalMemory(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.totalGlobalMem/(1024*1024);
	}

/**
 * @brief get total constant memory in kilo of device
 *
 * @param deviceNo device id
 **/
	 static int getTotalConstantMemory(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.totalConstMem/1024;
	}

/**
 * @brief get total shared memory per streaming multiprocessor in kilo bytes of device
 *
 * @param deviceNo device index
 **/
	static int getSharedMemoryPerSM(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.sharedMemPerBlock/1024;
	}

/**
 * @brief get total register per streaming multiprocessor in kilo bytes of device
 *
 * @param deviceNo device index
 **/
	static int getRegistersPerSM(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.regsPerBlock/1024;
	}

/**
 * @brief get warp size of GPU \ device 
 *
 * @param deviceNo deivce index
 **/
	static int getWarpSize(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.warpSize;
	}

/**
 * @brief get max threads per block of GPU \ device 
 *
 * @param deviceNo deivce index
 **/
	static int getMaxThreadsPerBlock(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.maxThreadsPerBlock;
	}

/**
 * @brief get max block dimension of GPU \ device 
 *
 * @param deviceNo deivice index
 **/
	static int3 getMaxBlocksDim(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);
	
		int3 dim;
		dim.x = prop.maxThreadsDim[0];
		dim.y = prop.maxThreadsDim[1];
		dim.z = prop.maxThreadsDim[2];
	
		return dim;
	}
/**
 * @brief get max grid dimension of GPU \ device 
 *
 * @param deviceNo deivice index
 **/
	static int3 getMaxGridsDim(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);
	
		int3 dim;
		dim.x = prop.maxGridSize[0];
		dim.y = prop.maxGridSize[1];
		dim.z = prop.maxGridSize[2];
	
		return dim;
	}

/**
 * @brief get max global memory alignment bytes of GPU \ device 
 *
 * @param deviceNo deivice index
 **/
	static int getMaxGlobalMemoryPitch(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.memPitch/(1024);
	}

/**
 * @brief get texture alignment bytes of GPU
 *
 * @param deviceNo device index
 **/
	static int getTextureAlignInBytes(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.textureAlignment;
	}

	
/**
 * @brief get clock rate in G of GPU device
 *
 * @param deviceNo device index
 **/
	static float getClockRate(int deviceNo){
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.clockRate*1.0e-6;
	}

	/**
 * @brief current GPU is whether can concurrent copy with kernel execution or not
 *
 * @param deviceNo device index
 **/
	static int canConcurrentCopyAndExecute(int deviceNo){
		if(Env::getRuntimeVersion() < 2000){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}
	
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.deviceOverlap;
	}

/**
 * @brief deivce GPU is whether have a time limit on kernel execution or not
 *
 * @param deviceNo device index
 **/
	static int isTimeLimitOnKernel(int deviceNo){
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}
	
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}

		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.kernelExecTimeoutEnabled;
	}

/**
 * @brief deivce GPU is whether integrated or not
 *
 * @param deviceNo device index
 **/
	static int isIntegratedCard(int deviceNo){
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}
	
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}
	
		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.integrated;
	}

/**
 * @brief whether can map host memory or not for GPU use of device
 *
 * @param deviceNo device index
 **/
	static int canMapHostMemory(int deviceNo){
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}
	
		int deviceCount = getDeviceCount();
		if(deviceNo >= deviceCount){
			printf("the device index you selected is larger than device Number\n");
			exit(1);
		}

		cudaDeviceProp prop;
		CudaGetDeviceProperties(&prop, deviceNo);

		return prop.canMapHostMemory;
	}

/**
 *@brief default constructor
 *
 */
	Device(){
		CudaGetDevice(&deviceNo);
		CudaGetDeviceProperties(&prop, deviceNo);
	}

	Device(int dNo){
		if(dNo >= getDeviceCount()){
			printf("the device index you selected is larger than device Number, program will exit, sorry!\n");
			exit(1);
		}else{
			deviceNo = dNo;
			CudaGetDeviceProperties(&prop, deviceNo);
		}	
	}
/**
 *@brief get device's No.
 *
 **/
	int getIndex() const{
		return deviceNo;
	}
/**
 * @brief get streaming multiprocessor number
 *
 **/
	int getSMNumber() const {
		return prop.multiProcessorCount;
	}

	 int getComputeCapability() const {
		return prop.major*100 + prop.minor*10;
	}

/**
 * @brief get total global memory capacity in Mega
 *
 **/
	int getTotalGlobalMemory() const {
		return prop.totalGlobalMem/(1024*1024);
	}

/**
 * @brief get total constant memory in kilo
 *
 **/
	int getTotalConstantMemory() const {
		return prop.totalConstMem/1024;
	}

/**
 * @brief get total shared memory per streaming multiprocessor in kilo bytes of current GPU if set else default
 *
 **/
	int getSharedMemoryPerSM() const {
		return prop.sharedMemPerBlock/1024;
	}

/**
 * @brief get total register per streaming multiprocessor in kilo bytes of current GPU if set else default
 *
 **/
	int getRegistersPerSM() const {
		return prop.regsPerBlock/1024;
	}

/**
 * @brief get warp size of current GPU
 *
 **/
	int getWarpSize() const {
		return prop.warpSize;
	}

/**
 * @brief get max threads per block of current GPU
 *
 **/
	int getMaxThreadsPerBlock() const {
		return prop.maxThreadsPerBlock;
	}

/**
 * @brief get max block dimension of current GPU \ device 
 *
 **/
	int3 getMaxBlocksDim() const {	
		int3 dim;
		dim.x = prop.maxThreadsDim[0];
		dim.y = prop.maxThreadsDim[1];
		dim.z = prop.maxThreadsDim[2];
	
		return dim;
	}

/**
 * @brief get max grid dimension of GPU \ device 
 *
 **/
	int3 getMaxGridsDim() const {
		int3 dim;
		dim.x = prop.maxGridSize[0];
		dim.y = prop.maxGridSize[1];
		dim.z = prop.maxGridSize[2];
	
		return dim;
	}

/**
 * @brief get max global memory alignment bytes of GPU \ device 
 *
 **/
	int getMaxGlobalMemoryPitch() const {
		return prop.memPitch/(1024);
	}

/**
 * @brief get texture alignment bytes of GPU \ device 
 *
 **/
	int getTextureAlignInBytes() const {
		return prop.textureAlignment;
	}

/**
 * @brief get clock rate in G of current GPU
 *
 **/
 float getClockRate() const {
	return prop.clockRate*1.0e-6;
}

/**
 * @brief current GPU is whether can concurrent copy with kernel execution or not
 *
 **/
	int canConcurrentCopyAndExecute() const {
		if(Env::getRuntimeVersion() < 2000){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}

		return prop.deviceOverlap;
	}

/**
 * @brief current GPU is whether have a time limit on kernel execution or not
 *
 **/
	int isTimeLimitOnKernel() const {
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}

		return prop.kernelExecTimeoutEnabled;
	}

/**
 * @brief current GPU is whether integrated or not
 *
 **/
	int isIntegratedCard() const {
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}

		return prop.integrated;
	}

/**
 * @brief whether can map host memory or not for GPU use of current GPU if set else default
 *
 **/
	int canMapHostMemory() const {
	   if(Env::getRuntimeVersion() < 2020){
			printf("please insall cuda2.0 or later\n");
			exit(1);
		}

		return prop.canMapHostMemory;
	}

};//end of class
#endif
