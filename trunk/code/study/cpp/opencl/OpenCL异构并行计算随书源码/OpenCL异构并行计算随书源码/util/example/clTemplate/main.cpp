#include "pbl.h"

int main(int argc, char* argv[]){
    int m = 2048;
    if(argc > 1) m = atoi(argv[1]);
    int n = 2048;
    if(argc > 2) n = atoi(argv[2]);
    int k = 2048;
    if(argc > 3) k = atoi(argv[3]);

	// Get OpenCL platform count
	cl_uint numPlatforms; 
	cl_platform_id* platforms = NULL;
	if(PBL_SUCCESS != pblOCLGetPlatforms(&numPlatforms, &platforms)){
		printMessage("fail to obtain platforms\n");
		return 0;
	}

	for(int i = 0; i < numPlatforms; i++){
		pblOCLListPlatformInfo(platforms[i]);
	}

	cl_platform_id p = platforms[0];

	cl_uint deviceCount;
	cl_device_id *devices = NULL;
	if(PBL_SUCCESS != pblOCLGetDevices(p, CL_DEVICE_TYPE_GPU, &deviceCount, &devices)){
		free(platforms);
		printMessage("error\n");
		return 0;
	}

        printf("Number of devices on Platform0 = %d. \n", deviceCount);

	for(int i = 0; i < deviceCount; i++){
		printf("Device %d\n", i);
		pblOCLListDeviceInfo(devices[i]);
	}

	cl_context cont;
	if(PBL_SUCCESS != pblOCLCreateContextWithPlatform(p, deviceCount, devices, &cont)){
		free(devices);
		free(platforms);
		printMessage("fail to create context\n");
		return 0;
	}

	pblOCLListContextInfo(cont);

	char* source = NULL;
	size_t srcLen;
	const char* filename = "naive.cl";
	if(PBL_SUCCESS != loadFileContent(filename, &source, &srcLen)){
		free(devices);
		free(platforms);
		printMessage("Fail to load Source\n");
		return 0;
	}
	
	const char *options = "-DT=float";
	cl_program program;
	if(PBL_SUCCESS != pblOCLCreateBuildProgramWithSource(cont, source, srcLen, deviceCount, devices, options, &program)){
		free(source);
		free(devices);
		free(platforms);
		printMessage("Fail to create & build program\n");
		return 0;
	}

	const char* kernelName = "matrixMultiplyNaiveKernel";
	cl_kernel kernel;
	if(CL_SUCCESS != pblOCLCreateKernel(program, kernelName, &kernel)) {
		free(source);
		free(devices);
		free(platforms);
		//checkPBLError(err);
		return 0;
	}

	cl_command_queue commandQueue;
    if(PBL_SUCCESS != pblOCLCreateCommandQueue(cont, devices[0], CL_QUEUE_PROFILING_ENABLE, &commandQueue)) {
        free(source);
        free(devices);
        free(platforms);
        //checkPBLError(err);
        return 0;
    }

//	cl_mem buf = clCreateBuffer(cont, CL_MEM_READ_ONLY, size*sizeof(float), NULL, NULL);
//	err = clEnqueueWriteBuffer(commandQueue, buf, CL_TRUE, 0, sizeof(float) * size, host, 0, NULL, NULL);
//	err = clEnqueueReadBuffer(commandQueue, buf, CL_TRUE, 0, sizeof(float) * size, host, 0, NULL, NULL);

//	clReleaseEvent(event);
//	clReleaseMemObject(buf);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(cont);

	free(source);
	free(devices);
	free(platforms);

	return 0;
}
