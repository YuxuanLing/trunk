#include <assert.h>
#include <time.h>
#include "pbl.h"

double getMillSecond() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ts.tv_sec*1000.0 + ts.tv_nsec/1000000.0;
}

int main(int argc, char* argv[]){
    PBLStatus_t err;

    // Get OpenCL platform count
    cl_uint numPlatforms; 
    cl_platform_id* platforms = NULL;
    if(-1 == pblOCLGetPlatforms(&numPlatforms, &platforms)){
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
    const char* filename = "naiveKernel.cl";
    if(PBL_SUCCESS != loadFileContent(filename, &source, &srcLen)){
        free(devices);
        free(platforms);
        printMessage("Fail to load Source\n");
        return 0;
    }

    const char *options = "-DT=float -DfilterSize=5";
    cl_program program;
    if(PBL_SUCCESS != pblOCLCreateBuildProgramWithSource(cont, source, srcLen, deviceCount, devices, options, &program)){
        free(source);
        free(devices);
        free(platforms);
        printMessage("Fail to create & build program\n");
        return 0;
    }

    const char* kernelName = "convolutionNaive";
    cl_kernel kernel;
    err = pblOCLCreateKernel(program, kernelName, &kernel);
    if(PBL_SUCCESS != err){
        free(source);
        free(devices);
        free(platforms);
        //checkPBLError(err);
        printMessage("");
        return 0;
    }

    cl_command_queue commandQueue;
    assert(PBL_SUCCESS == pblOCLCreateCommandQueue(cont, devices[0], CL_QUEUE_PROFILING_ENABLE, &commandQueue));

    int numRows = 1024;
    if(argc > 1) numRows = atoi(argv[1]);
    int numCols = 1024;
    if(argc > 2) numCols = atoi(argv[2]);

    int size = numRows*numCols;

    float* h_in = (float*) malloc(sizeof(float)*size);
    for(int i = 0; i < size; i++) {
        h_in[i ] = 1;//i %2;
    }

    //bug for pass ptr when create buffer
    cl_mem in; 
    assert(PBL_SUCCESS == pblOCLCreateBuffer(cont, CL_MEM_READ_ONLY, size*sizeof(float), NULL, &in));
    assert(CL_SUCCESS == clEnqueueWriteBuffer(commandQueue, in, CL_TRUE, 0, sizeof(float)*size, h_in, 0, NULL, NULL));

    size = 5*5;
    float *h_filter = (float*) malloc(size*sizeof(float));
    for(int i = 0; i < size; i++) {
        h_filter[i ] = 1;//i %2;
    }

    int iter = 100;

    cl_mem filter; 
    assert(PBL_SUCCESS == pblOCLCreateBuffer(cont, CL_MEM_READ_ONLY, size*sizeof(float), NULL, &filter));
    assert(CL_SUCCESS == clEnqueueWriteBuffer(commandQueue, filter, CL_TRUE, 0, sizeof(float)*size, h_filter, 0, NULL, NULL));

    size = (numRows-4)*(numCols-4);
    cl_mem out;
    assert(PBL_SUCCESS == pblOCLCreateBuffer(cont, CL_MEM_WRITE_ONLY, size*sizeof(float), NULL, &out));
    //	err = clEnqueueWriteBuffer(commandQueue, buf, CL_TRUE, 0, sizeof(float) * size, host, 0, NULL, NULL);

    size_t local_work_size[] = {32, 16};
    size_t gx = (numCols+local_work_size[0]-1)/local_work_size[0]*local_work_size[0];
    size_t gy = (numRows+local_work_size[1]-1)/local_work_size[1]*local_work_size[1];
    //size_t global_work_size[] = {gx, gy};
    size_t global_work_size[] = {1024, 1024};

    int temp = numCols-4;
    assert(CL_SUCCESS == clSetKernelArg(kernel, 0, sizeof(int), (void*)&temp));
    temp = numRows-4;
    assert(CL_SUCCESS == clSetKernelArg(kernel, 1, sizeof(int), (void*)&temp));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&in));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&filter));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&out));
    cl_event event;

    double st = getMillSecond();
    for(int i = 0; i < iter; i++) {
    assert(CL_SUCCESS == clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL));
    assert(CL_SUCCESS == clFinish(commandQueue));
    }
    double et = getMillSecond();
    double time = (et-st)/iter;
printf("%.3f ms\n", time);

    size = (numRows-4)*(numCols-4);
    float *h_out = (float*) malloc(size*sizeof(float));
    assert(CL_SUCCESS == clEnqueueReadBuffer(commandQueue, out, CL_TRUE, 0, sizeof(float) * size, h_out, 0, NULL, NULL));
    for(int i = 0; i < size; i++) printf("%.3f\n", h_out[i]);

//    clReleaseEvent(event);
    clReleaseMemObject(in);
    clReleaseMemObject(filter);
    clReleaseMemObject(out);

    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(cont);

    free(h_out);
    free(h_filter);
    free(h_in);

    free(source);
    free(devices);
    free(platforms);

    return 0;
}
