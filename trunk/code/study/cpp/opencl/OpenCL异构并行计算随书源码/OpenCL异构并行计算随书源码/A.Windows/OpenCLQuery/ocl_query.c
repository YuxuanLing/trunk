#include "ocl_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* 默认最大平台个数 */
#define MAX_PLATFORM_NUMBERS    16

/* 每个平台默认最大设备个数 */
#define MAX_DEVICE_NUMBERS      16

static cl_platform_id sPlatforms[MAX_PLATFORM_NUMBERS];
static cl_device_id sDevices[MAX_DEVICE_NUMBERS];

static int sCurrentPlatformCount;
static int sCurrentDeviceCount;

static char sPlatformNames[MAX_PLATFORM_NUMBERS][256];
static char sDeviceNames[MAX_DEVICE_NUMBERS][256];


int CL_Query_InitPlatforms(void)
{
    cl_uint nPlatforms = 0;

    // 先获得平台个数
    clGetPlatformIDs(0, NULL, &nPlatforms);

    if (nPlatforms == 0)
        return 0;

    if (nPlatforms > MAX_PLATFORM_NUMBERS)
        nPlatforms = MAX_PLATFORM_NUMBERS;

    // 获取当前所有OpenCL平台对象
    clGetPlatformIDs(nPlatforms, sPlatforms, NULL);

    // 获得每个OpenCL平台对象对应的平台名称
    for (cl_uint i = 0; i < nPlatforms; i++)
        clGetPlatformInfo(sPlatforms[i], CL_PLATFORM_NAME, 256, sPlatformNames[i], NULL);

    sCurrentPlatformCount = nPlatforms;
    
    return nPlatforms;
}

const char* CL_Query_GetPlatformName(int index)
{
    if (index < 0 || index >= sCurrentPlatformCount)
        return NULL;

    return sPlatformNames[index];
}

int CL_Query_InitDevices(int platformIndex)
{
    if (platformIndex < 0 || platformIndex >= sCurrentPlatformCount)
        return 0;

    cl_platform_id platform = sPlatforms[platformIndex];

    cl_uint nDevices = 0;

    // 先获得当前平台能支持的设备数
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &nDevices);
    if (nDevices == 0)
        return 0;

    if (nDevices > MAX_DEVICE_NUMBERS)
        nDevices = MAX_DEVICE_NUMBERS;

    // 再获得每个支持设备的对象
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, nDevices, sDevices, NULL);

    for (cl_uint i = 0; i < nDevices; i++)
    {
        // 获得相应的设备名
        clGetDeviceInfo(sDevices[i], CL_DEVICE_NAME, 256, sDeviceNames[i], NULL);
    }

    sCurrentDeviceCount = nDevices;

    return nDevices;
}

const char* CL_Query_GetDeviceName(int index)
{
    if (index < 0 || index >= sCurrentDeviceCount)
        return NULL;

    return sDeviceNames[index];
}

/*
 从以分号分隔的字符串中剥离出各个子字符串
* @param dst 目的字符串数组
* @param orgStr 源字符串
* @return 子字符串的个数
*/
static int GetSplitStringsFromString(char dst[][256], const char *orgStr, char separator)
{
    if (dst == NULL || orgStr == NULL)
        return 0;

    const size_t orgLength = strlen(orgStr);
    if (orgLength == 0)
        return 0;

    int count = 0;
    int initPos = 0;
    for (int i = 0; orgStr[i] != '\0'; i++)
    {
        if (orgStr[i] == separator)
        {
            memcpy(dst[count], &orgStr[initPos], i - initPos);
            dst[count++][i - initPos] = '\0';
            initPos = i + 1;
        }
    }
    if (initPos != orgLength)
    {
        memcpy(dst[count], &orgStr[initPos], orgLength - initPos);
        dst[count++][orgLength - initPos] = '\0';
    }

    return count;
}

int CL_Query_GetDeviceFeatures(int deviceIndex, const char *kernelFileName, const char* pOutFeatureNames[], char(*pOutFeatureDescs)[256])
{
    if (deviceIndex < 0 || deviceIndex >= sCurrentDeviceCount)
        return 0;

    if (pOutFeatureNames == NULL || pOutFeatureDescs == NULL)
        return 0;

    cl_device_id device = sDevices[deviceIndex];

    cl_int minimumGranularity = 0;
    size_t groupSize = 0;

    char tmpBuffer[2048];
    cl_uint tmpIntValue;
    cl_ulong tmpLongValue;
    cl_bool boolValue;

    // 获取OpenCL C版本号
    clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
    int index = 0;
    while (tmpBuffer[index] != '\0')
    {
        if (tmpBuffer[index] >= '0' && tmpBuffer[index] <= '9')
            break;
        index++;
    }
    
    const double clVersion = atof(&tmpBuffer[index]);

    /* 以下代码用于获得最小线程并行粒度 */
    cl_context context = NULL;
    cl_command_queue commandQueue = NULL;
    cl_program program = NULL;
    cl_mem outputMemObj = NULL;
    cl_kernel kernel = NULL;

    do
    {
        context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
        commandQueue = clCreateCommandQueue(context, device, 0, NULL);

        FILE *fp = fopen(kernelFileName, "r");
        if (fp == NULL)
            break;
        fseek(fp, 0, SEEK_END);
        size_t kernelLength = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *kernelCodeBuffer = malloc(kernelLength + 1);
        fread(kernelCodeBuffer, 1, kernelLength, fp);
        kernelCodeBuffer[kernelLength] = '\0';
        fclose(fp);

        const char *aSource = kernelCodeBuffer;
        program = clCreateProgramWithSource(context, 1, &aSource, &kernelLength, NULL);

        free(kernelCodeBuffer);

        // 我们这里顺便再演示一下各个不同OpenCL C语言版本的兼容写法
        const char *options = clVersion < 2.0? NULL : "-cl-std=CL2.0";
        cl_int status = clBuildProgram(program, 1, &device, options, NULL, NULL);
        if (status != CL_SUCCESS)
            break;

        outputMemObj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(minimumGranularity), NULL, NULL);

        kernel = clCreateKernel(program, "QueryMinimumGranularity", NULL);

        cl_int inputArg = 1000;     // 这里设置内核程序里循环1000次进行延迟等待
        status = clSetKernelArg(kernel, 0, sizeof(inputArg), &inputArg);
        status |= clSetKernelArg(kernel, 1, sizeof(outputMemObj), &outputMemObj);
        if (status != CL_SUCCESS)
            break;

        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(groupSize), &groupSize, NULL);
        
        status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, (size_t[]){ groupSize }, (size_t[]){ groupSize }, 0, NULL, NULL);
        if (status != CL_SUCCESS)
            break;

        clFinish(commandQueue);

        status = clEnqueueReadBuffer(commandQueue, outputMemObj, CL_TRUE, 0, sizeof(minimumGranularity), &minimumGranularity, 0, NULL, NULL);
        if (status != CL_SUCCESS)
            break;
    } 
    while (false);

    clReleaseMemObject(outputMemObj);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    // 下面开始正式查询
    index = 0;

    pOutFeatureNames[index] = "Device Type";
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    switch (tmpLongValue)
    {
    case CL_DEVICE_TYPE_CPU:
        strcpy(pOutFeatureDescs[index], "CPU");
        break;

    case CL_DEVICE_TYPE_GPU:
        strcpy(pOutFeatureDescs[index], "GPU");
        break;

    case CL_DEVICE_TYPE_ACCELERATOR:
        strcpy(pOutFeatureDescs[index], "Accelerator");
        break;

    case CL_DEVICE_TYPE_DEFAULT:
        strcpy(pOutFeatureDescs[index], "Default");
        break;

    case CL_DEVICE_TYPE_CUSTOM:
        strcpy(pOutFeatureDescs[index], "Custom");
        break;
    }
    index++;

    pOutFeatureNames[index] = "Device Name";
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "Vendor";
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "Vendor ID";
    clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%0.8X", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Device Profile";
    clGetDeviceInfo(device, CL_DEVICE_PROFILE, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "Driver Version";
    clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "Device Version";
    clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "OpenCL C Version";
    clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
    strcpy(pOutFeatureDescs[index], tmpBuffer);
    index++;

    pOutFeatureNames[index] = "Max Compute Units";
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Max work-item Dimensions";
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Max work-item Sizes";
    size_t maxWorkItemSizes[3];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), maxWorkItemSizes, NULL);
    sprintf(pOutFeatureDescs[index], "%ux%ux%u", maxWorkItemSizes[0], maxWorkItemSizes[1], maxWorkItemSizes[2]);
    index++;

    pOutFeatureNames[index] = "Max work-group Size";
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    pOutFeatureNames[index] = "Work-items for Minimum Parallel Grain";
    sprintf(pOutFeatureDescs[index], "%d", minimumGranularity);
    index++;

    pOutFeatureNames[index] = "Preferred char Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred short Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred int Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred long Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred float Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred double Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Preferred half Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native char Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native short Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native int Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native long Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native float Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native double Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Native half Vector Width";
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Max Clock Frequency";
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u MHz", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Address Bits";
    clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Max Size of Memory Object Allocation";
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", tmpLongValue / (1024 * 1024));
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Support Image";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", tmpIntValue == 0? "NO" : "YES");
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Image Objects Arguments with read_only";
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Image Objects Arguments with write_only";
    clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Image Objects Arguments with read_write";
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Width of 2D or 1D Image in pixels";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Height of 2D Image in pixels";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Width of 3D Image";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Height of 3D Image";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Depth of 3D Image";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Number of Pixels of a 1D image";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Number of Images in a 1D or 2D Array";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpLongValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of samplers";
    clGetDeviceInfo(device, CL_DEVICE_MAX_SAMPLERS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Row Pitch Alignment Size in pixels for 2D Images";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_PITCH_ALIGNMENT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Image Base Address Alignment in pixels";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Pipe Objects as Kernel Arguments";
    clGetDeviceInfo(device, CL_DEVICE_MAX_PIPE_ARGS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Pipe Active Reservations";
    clGetDeviceInfo(device, CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Size of Pipe Packet";
    clGetDeviceInfo(device, CL_DEVICE_PIPE_MAX_PACKET_SIZE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", tmpIntValue / (1024 * 1024));
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Size of All Arguments Passed to a Kernel";
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u bytes", tmpLongValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Memory Base Address Alignment";
    clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u bytes", tmpIntValue / 8);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Single Precision float-point Capability";
    clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_FP_DENORM) != 0)
        strcpy(pOutFeatureDescs[index], "denorms are supported");
    else
        strcpy(pOutFeatureDescs[index], "denorms are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_INF_NAN) != 0)
        strcpy(pOutFeatureDescs[index], "INF and quiet NaNs are supported");
    else
        strcpy(pOutFeatureDescs[index], "INF and quiet NaNs are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_NEAREST) != 0)
        strcpy(pOutFeatureDescs[index], "round to nearest even rounding mode supported");
    else
        strcpy(pOutFeatureDescs[index], "round to nearest even rounding mode not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_ZERO) != 0)
        strcpy(pOutFeatureDescs[index], "round to zero rounding mode supported");
    else
        strcpy(pOutFeatureDescs[index], "round to zero rounding mode not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_INF) != 0)
        strcpy(pOutFeatureDescs[index], "round to +/- infinity rounding modes are supported");
    else
        strcpy(pOutFeatureDescs[index], "round to +/- infinity rounding modes are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_FMA) != 0)
        strcpy(pOutFeatureDescs[index], "fused multiply-add supported");
    else
        strcpy(pOutFeatureDescs[index], "fused multiply-add not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) != 0)
        strcpy(pOutFeatureDescs[index], "divide and sqrt are correctly rounded");
    else
        strcpy(pOutFeatureDescs[index], "divide and sqrt are not correctly rounded");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_SOFT_FLOAT) != 0)
        strcpy(pOutFeatureDescs[index], "Basic FP operations implemented in software");
    else
        strcpy(pOutFeatureDescs[index], "Basic FP operations implemented in hardware");
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Double Precision float-point Capability";
    clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_FP_DENORM) != 0)
        strcpy(pOutFeatureDescs[index], "denorms are supported");
    else
        strcpy(pOutFeatureDescs[index], "denorms are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_INF_NAN) != 0)
        strcpy(pOutFeatureDescs[index], "INF and quiet NaNs are supported");
    else
        strcpy(pOutFeatureDescs[index], "INF and quiet NaNs are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_NEAREST) != 0)
        strcpy(pOutFeatureDescs[index], "round to nearest even rounding mode supported");
    else
        strcpy(pOutFeatureDescs[index], "round to nearest even rounding mode not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_ZERO) != 0)
        strcpy(pOutFeatureDescs[index], "round to zero rounding mode supported");
    else
        strcpy(pOutFeatureDescs[index], "round to zero rounding mode not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_ROUND_TO_INF) != 0)
        strcpy(pOutFeatureDescs[index], "round to +/- infinity rounding modes are supported");
    else
        strcpy(pOutFeatureDescs[index], "round to +/- infinity rounding modes are not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_FMA) != 0)
        strcpy(pOutFeatureDescs[index], "fused multiply-add supported");
    else
        strcpy(pOutFeatureDescs[index], "fused multiply-add not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_FP_SOFT_FLOAT) != 0)
        strcpy(pOutFeatureDescs[index], "Basic FP operations implemented in software");
    else
        strcpy(pOutFeatureDescs[index], "Basic FP operations implemented in hardware");
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Global Memory Cache Type";
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    switch (tmpIntValue)
    {
    case CL_READ_ONLY_CACHE:
        strcpy(pOutFeatureDescs[index], "Read-Only Cache");
        break;

    case CL_READ_WRITE_CACHE:
        strcpy(pOutFeatureDescs[index], "Read-Write Cache");
        break;

    default:
        strcpy(pOutFeatureDescs[index], "Global Memory Cache not supported");
        break;
    }
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Global Memory Cache Line Size";
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u Bytes", tmpIntValue);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Global Memory Cache Size";
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u KB", tmpLongValue / 1024);
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Global Memory Size";
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", tmpLongValue / (1024 * 1024));
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Max Constant Buffer Size";
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u KB", tmpLongValue / 1024);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Constant Arguments";
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    groupSize = 0;
    pOutFeatureNames[index] = "Max Global Variable Size";
    clGetDeviceInfo(device, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE, sizeof(groupSize), &groupSize, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", groupSize / (1024 * 1024));
    index++;

    groupSize = 0;
    pOutFeatureNames[index] = "Max Global Variable Preferred Total Size";
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE, sizeof(groupSize), &groupSize, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", groupSize / (1024 * 1024));
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Local Memory Type";
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    switch (tmpIntValue)
    {
    case CL_LOCAL:
        strcpy(pOutFeatureDescs[index], "Dedicated Local Memory Storage");
        break;

    case CL_GLOBAL:
        strcpy(pOutFeatureDescs[index], "Global Memory");
        break;

    default:
        strcpy(pOutFeatureDescs[index], "Local Memory not supported");
        break;
    }
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Local Memory Size";
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(tmpLongValue), &tmpLongValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u KB", tmpLongValue / 1024);
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "Error Correction Support";
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue? "YES" : "NO");
    index++;

    groupSize = 0;
    pOutFeatureNames[index] = "Profiling Timer Resolution";
    clGetDeviceInfo(device, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(groupSize), &groupSize, NULL);
    sprintf(pOutFeatureDescs[index], "%u ns", groupSize);
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "Device Endian";
    clGetDeviceInfo(device, CL_DEVICE_ENDIAN_LITTLE, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue ? "Little Endian" : "Big Endian");
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "Device Available";
    clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue ? "YES" : "NO");
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "Device Compiler Available";
    clGetDeviceInfo(device, CL_DEVICE_COMPILER_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue ? "YES" : "NO");
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "Device Linker Available";
    clGetDeviceInfo(device, CL_DEVICE_LINKER_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue ? "YES" : "NO");
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Command Queue on Host Properties";
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0)
        strcpy(pOutFeatureDescs[index], "Out-of-Order exec Mode enabled");
    else
        strcpy(pOutFeatureDescs[index], "Out-of-Order exec Mode disabled");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_QUEUE_PROFILING_ENABLE) != 0)
        strcpy(pOutFeatureDescs[index], "Queue Profiling enabled");
    else
        strcpy(pOutFeatureDescs[index], "Queue Profiling disabled");
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Command Queue on Device Properties";
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0)
        strcpy(pOutFeatureDescs[index], "Out-of-Order exec Mode enabled");
    else
        strcpy(pOutFeatureDescs[index], "Out-of-Order exec Mode disabled");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_QUEUE_PROFILING_ENABLE) != 0)
        strcpy(pOutFeatureDescs[index], "Queue Profiling enabled");
    else
        strcpy(pOutFeatureDescs[index], "Queue Profiling disabled");
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Command Queue Preferred Size on Device";
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u KB", tmpIntValue / 1024);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Command Queue Max Size on Device";
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u KB", tmpIntValue / 1024);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Command Queues on Device Per Context";
    clGetDeviceInfo(device, CL_DEVICE_MAX_ON_DEVICE_QUEUES, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Events on a Device Queue";
    clGetDeviceInfo(device, CL_DEVICE_MAX_ON_DEVICE_EVENTS, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    pOutFeatureNames[index] = "Built-in Kernels";
    clGetDeviceInfo(device, CL_DEVICE_BUILT_IN_KERNELS, sizeof(tmpBuffer), tmpBuffer, NULL);
    tmpIntValue = GetSplitStringsFromString(&pOutFeatureDescs[index], tmpBuffer, ';');
    index++;
    for (cl_uint i = 1; i < tmpIntValue; i++)
        pOutFeatureNames[index++] = " ";

    pOutFeatureNames[index] = "Device Extensions";
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(tmpBuffer), tmpBuffer, NULL);
    tmpIntValue = GetSplitStringsFromString(&pOutFeatureDescs[index], tmpBuffer, ' ');
    index++;
    for (cl_uint i = 1; i < tmpIntValue; i++)
        pOutFeatureNames[index++] = " ";

    groupSize = 0;
    pOutFeatureNames[index] = "Device PRINTF Buffer Size";
    clGetDeviceInfo(device, CL_DEVICE_PRINTF_BUFFER_SIZE, sizeof(groupSize), &groupSize, NULL);
    sprintf(pOutFeatureDescs[index], "%u MB", groupSize / (1024 * 1024));
    index++;

    boolValue = false;
    pOutFeatureNames[index] = "The device’s preference is for the user to be responsible for synchronization";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, sizeof(boolValue), &boolValue, NULL);
    sprintf(pOutFeatureDescs[index], "%s", boolValue ? " YES" : "NO");
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Max Number of Sub-devices for Partitioning";
    clGetDeviceInfo(device, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u", tmpIntValue);
    index++;

    cl_device_partition_property partitionProperties[8] = { 0 };
    pOutFeatureNames[index] = "Partition Properties";
    clGetDeviceInfo(device, CL_DEVICE_PARTITION_PROPERTIES, sizeof(partitionProperties), partitionProperties, NULL);
    for (int i = 0; i < 7; i++)
    {
        if (partitionProperties[i] == 0)
            break;

        switch (partitionProperties[i])
        {
        case CL_DEVICE_PARTITION_EQUALLY:
            strcpy(pOutFeatureDescs[index], "Partition Equally");
            break;

        case CL_DEVICE_PARTITION_BY_COUNTS:
            strcpy(pOutFeatureDescs[index], "Partition By Counts");
            break;

        case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
            strcpy(pOutFeatureDescs[index], "Partition By Affinity Domain");
            break;

        default:
            strcpy(pOutFeatureDescs[index], "Unknown Property");
            break;
        }

        index++;

        pOutFeatureNames[index] = " ";
    }

    if (partitionProperties[0] == 0)
        index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Device Partition Affinity Domain";
    clGetDeviceInfo(device, CL_DEVICE_PARTITION_AFFINITY_DOMAIN, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_NUMA) != 0)
        strcpy(pOutFeatureDescs[index], "NUMA supported");
    else
        strcpy(pOutFeatureDescs[index], "NUMA not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) != 0)
        strcpy(pOutFeatureDescs[index], "L4 Cache supported");
    else
        strcpy(pOutFeatureDescs[index], "L4 Cache not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) != 0)
        strcpy(pOutFeatureDescs[index], "L3 Cache supported");
    else
        strcpy(pOutFeatureDescs[index], "L3 Cache not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE) != 0)
        strcpy(pOutFeatureDescs[index], "L2 Cache supported");
    else
        strcpy(pOutFeatureDescs[index], "L2 Cache not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) != 0)
        strcpy(pOutFeatureDescs[index], "L1 Cache supported");
    else
        strcpy(pOutFeatureDescs[index], "L1 Cache not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) != 0)
        strcpy(pOutFeatureDescs[index], "Next Partitionalble");
    else
        strcpy(pOutFeatureDescs[index], "Next not Partitionalble");
    index++;

    tmpLongValue = 0;
    pOutFeatureNames[index] = "Device SVM Capabilities";
    clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(tmpLongValue), &tmpLongValue, NULL);
    if ((tmpLongValue & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) != 0)
        strcpy(pOutFeatureDescs[index], "Coarse Grain Buffer supported");
    else
        strcpy(pOutFeatureDescs[index], "Coarse Grain Buffer not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) != 0)
        strcpy(pOutFeatureDescs[index], "Fine Grain Buffer supported");
    else
        strcpy(pOutFeatureDescs[index], "Fine Grain Buffer not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) != 0)
        strcpy(pOutFeatureDescs[index], "Fine Grain System Buffer supported");
    else
        strcpy(pOutFeatureDescs[index], "Fine Grain System Buffer not supported");
    index++;

    pOutFeatureNames[index] = " ";
    if ((tmpLongValue & CL_DEVICE_SVM_ATOMICS) != 0)
        strcpy(pOutFeatureDescs[index], "SVM Atomics supported");
    else
        strcpy(pOutFeatureDescs[index], "SVM Atomics not supported");
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Preferred Platform Atomic Alignement";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u Bytes", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Preferred Global Atomic Alignement";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u Bytes", tmpIntValue);
    index++;

    tmpIntValue = 0;
    pOutFeatureNames[index] = "Preferred Local Atomic Alignement";
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT, sizeof(tmpIntValue), &tmpIntValue, NULL);
    sprintf(pOutFeatureDescs[index], "%u Bytes", tmpIntValue);
    index++;

    return index;
}

