#include <float.h>
#include <math.h>
#include <time.h>

#include <assert.h>

#include "pbl.h"

double getMillSecond() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

    return ts.tv_sec*1000.0 + ts.tv_nsec/1000000.0;
}

void initFloat(size_t len, float* d) {
    for(size_t i = 0; i < len; i++) {
        d[i] = 1.0f;//*rand()/RAND_MAX;
    }
}

void matrixMul(int m, int n, int K, float* a, float* b, float* c) {
    memset(c, 0, m*n*sizeof(float));
    for(int i = 0; i < m; i++) {
        for(int k = 0; k < K; k++){
            for(int j = 0; j < n; j++) {
                c[i*n+j] += a[i*K+k]*b[k*n+j];
            }
        }
    }
}

void checkResult(size_t len, float* a, float *b) {
    float l1 = 0.0f, l2 = 0.0f, max = -FLT_MAX, min = FLT_MAX;

    for(size_t i = 0; i < len; i++) {
        float diff = fabsf(a[i] - b[i]);
        l1 += diff;
        l2 += diff*diff;
        max = fmaxf(diff, max);
        min = fminf(diff, min);
        if(diff > 1e-3) printf("%d %d %.3f %.3f %.3f\n", i/2048, i%2048, a[i], b[i], diff);
    }

    printf("%.3f %.3f %.3f %.3f\n", l1/len, l2/len, max, min);
}

int main(int argc, char* argv[]){
    int m = 2048;
    if(argc > 1) m = atoi(argv[1]);
    int n = 2048;
    if(argc > 2) n = atoi(argv[2]);
    int k = 2048;
    if(argc > 3) k = atoi(argv[3]);

    int iter = 20;
    if(argc > 4) iter = atoi(argv[4]);

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
    const char* filename = "sharedBlocking.cl";
    if(PBL_SUCCESS != loadFileContent(filename, &source, &srcLen)){
        free(devices);
        free(platforms);
        printMessage("Fail to load Source\n");
        return 0;
    }

    const char *options = "-DBS=16 -Dunroll_m=4 -Dunroll_n=4";
    cl_program program;
    if(PBL_SUCCESS != pblOCLCreateBuildProgramWithSource(cont, source, srcLen, deviceCount, devices, options, &program)){
        free(source);
        free(devices);
        free(platforms);
        printMessage("Fail to create & build program\n");
        return 0;
    }

    const char* kernelName = "matrixMultiplyKernel";
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

    float* h_a = (float*) malloc(m*k*sizeof(float));
    assert(NULL != h_a);
    initFloat(m*k, h_a);

    float* h_b = (float*) malloc(n*k*sizeof(float));
    assert(NULL != h_b);
    initFloat(m*k, h_b);

    float* h_c = (float*) malloc(m*n*sizeof(float));
    assert(NULL != h_c);

    cl_mem buf_a = clCreateBuffer(cont, CL_MEM_READ_ONLY, m*k*sizeof(float), NULL, NULL);
    assert(CL_SUCCESS == clEnqueueWriteBuffer(commandQueue, buf_a, CL_TRUE, 0, sizeof(float)*m*k, h_a, 0, NULL, NULL));
    cl_mem buf_b = clCreateBuffer(cont, CL_MEM_READ_ONLY, n*k*sizeof(float), NULL, NULL);
    assert(CL_SUCCESS == clEnqueueWriteBuffer(commandQueue, buf_b, CL_TRUE, 0, sizeof(float)*n*k, h_b, 0, NULL, NULL));
    cl_mem buf_c = clCreateBuffer(cont, CL_MEM_WRITE_ONLY, n*m*sizeof(float), NULL, NULL);


    assert(CL_SUCCESS == clSetKernelArg(kernel, 0, sizeof(int), &m));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 1, sizeof(int), &n));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 2, sizeof(int), &k));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 3, sizeof(cl_mem), &buf_a));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 4, sizeof(cl_mem), &buf_b));
    assert(CL_SUCCESS == clSetKernelArg(kernel, 5, sizeof(cl_mem), &buf_c));

    size_t globalSize[] = {n/4, m/4};
    size_t localSize[] = {16, 16};

    double st = getMillSecond();
    for(int it = 0; it < iter; it++)
    assert(CL_SUCCESS == clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, globalSize, localSize, 0, NULL, NULL));
    assert(CL_SUCCESS == clFinish(commandQueue));
    double et= getMillSecond();
    double time = (et-st)/iter;
    float flops = 2.0*m*n*k/time/1024/1024;
    printf("use time = %.3f, flops = %.3f\n", (float)time, flops);

    assert(CL_SUCCESS == clEnqueueReadBuffer(commandQueue, buf_c, CL_TRUE, 0, sizeof(float)*m*n, h_c, 0, NULL, NULL));

    float* h_c2 = (float*) malloc(m*n*sizeof(float));
    assert(NULL != h_c2);

    matrixMul(m, n, k, h_a, h_b, h_c2);

    checkResult(m*n, h_c, h_c2);

    //	clReleaseEvent(event);
    clReleaseMemObject(buf_a);
    clReleaseMemObject(buf_b);
    clReleaseMemObject(buf_c);

    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(cont);

    free(source);
    free(devices);
    free(platforms);

    free(h_a);
    free(h_b);
    free(h_c);
    free(h_c2);

    return 0;
}
