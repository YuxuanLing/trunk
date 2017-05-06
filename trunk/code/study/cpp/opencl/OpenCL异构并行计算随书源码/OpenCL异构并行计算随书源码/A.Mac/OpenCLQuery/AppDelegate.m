//
//  AppDelegate.m
//  OpenCLQuery
//
//  Created by Zenny Chen on 13-9-9.
//  Copyright (c) 2013年 Zenny Chen. All rights reserved.
//

#import "AppDelegate.h"
#import <OpenCL/OpenCL.h>

@interface AppDelegate()<NSComboBoxDelegate, NSTableViewDataSource, NSTableViewDelegate>

@end


enum COMBO_CONTROL_TAG
{
    COMBO_CONTROL_TAG_PLATFORM = 100,
    COMBO_CONTROL_TAG_DEVICE,
    COMBO_CONTROL_TAG_CONTENTS
};


@implementation AppDelegate

@synthesize window;

- (void)dealloc
{
    if(mPlatformArray != nil)
    {
        [mPlatformArray removeAllObjects];
        [mPlatformArray release];
    }
    if(mDeviceArray != nil)
    {
        [mDeviceArray removeAllObjects];
        [mDeviceArray release];
    }
    if(mContentKeys != nil)
    {
        [mContentKeys removeAllObjects];
        [mContentKeys release];
    }
    if(mContentValues != nil)
    {
        [mContentValues removeAllObjects];
        [mContentValues release];
    }
    
    [super dealloc];
}

static cl_platform_id platforms[16];
static cl_device_id devices[16];

- (BOOL)initOpenCLQuery
{
    NSString *errString = nil;
    
    do
    {
        /*Step 1: Query the platform.*/
        cl_uint numPlatforms = 0;
        cl_int	status = clGetPlatformIDs(0, NULL, &numPlatforms);
        if (status != CL_SUCCESS)
        {
            errString = @"Error: Getting platforms!";
            break;
        }
        if(numPlatforms > 16)
            numPlatforms = 16;
        
        if(numPlatforms > 0)
            status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        else
        {
            errString = @"Your system does not support OpenCL!";
            break;
        }
        if (status != CL_SUCCESS)
        {
            errString = @"Error: Getting platforms!";
            break;
        }
        
        // Fetch platform names
        for(cl_uint i = 0; i < numPlatforms; i++)
        {
            char strBuf[256];
            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 256, strBuf, NULL);
            [mPlatformArray addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
        }
    }
    while(0);
    
    if(errString != nil)
    {
        NSAlert *alert = [NSAlert alertWithMessageText:@"Warning" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"%@", errString];
        [alert runModal];
        
        return NO;
    }
    
    return YES;
}

- (NSArray*)parseStringArrayWithSeparator:(char)separator string:(const char*)orgString length:(NSUInteger)length
{
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:1024];
    char buf[256];
    
    int bIndex = 0;
    for(NSUInteger i = 0; i < length; i++)
    {
        char c = orgString[i];
        if(c == '\0')
            break;
        
        if(orgString[i] != separator)
            buf[bIndex++] = c;
        else
        {
            if(bIndex == 0)
                break;
            
            buf[bIndex] = '\0';
            bIndex = 0;
            NSString *s = [NSString stringWithCString:buf encoding:NSASCIIStringEncoding];
            [array addObject:s];
        }
    }
    
    return array;
}

- (void)setDeviceInfo:(NSUInteger)currIndex
{
    cl_platform_id platform = platforms[currIndex];
    
    if(mDeviceArray != nil)
        [mDeviceArray removeAllObjects];
    
    /*Step 2:Query the GPU and CPU device*/
    // Set device info
    cl_uint numComputeDevices = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numComputeDevices);
    if(numComputeDevices > 16)
        numComputeDevices = 16;
    
    if (numComputeDevices > 0)
    {
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numComputeDevices, devices, NULL);
        
        for(cl_uint i = 0; i < numComputeDevices; i++)
        {
            char strBuf[256];
            clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 256, strBuf, NULL);
            [mDeviceArray addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
        }
    }
    else
    {
        NSAlert *alert = [NSAlert alertWithMessageText:@"Warning" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"No OpenCL devices found"];
        [alert runModal];
    }
}

- (void)setOpenCLDevieContent:(NSUInteger)currIndex
{
    if(mContentKeys != nil)
        [mContentKeys removeAllObjects];
    else
        mContentKeys = [[NSMutableArray alloc] initWithCapacity:1024];
    
    if(mContentValues != nil)
        [mContentValues removeAllObjects];
    else
        mContentValues = [[NSMutableArray alloc] initWithCapacity:1024];
    
    cl_device_id device = devices[currIndex];
    
    /** Query the minimum parallel granularity */
    
    /*Step 3: Create context.*/
    cl_context context = NULL;       // OpenCL context
    cl_command_queue commandQueue = NULL;
    cl_program program = NULL;       // OpenCL kernel program object that'll be running on the compute device
    cl_mem outputMemObj = NULL;      // output memory object for output
    cl_kernel kernel = NULL;         // kernel object
    
    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    
    /** Prepare for running an OpenCL kernel program to get minimum thread granularity */
    
    /*Step 4: Creating command queue associate with the context.*/
    commandQueue = clCreateCommandQueue(context, device, 0, NULL);
    
    /*Step 5: Create program object */
    // Read the kernel code to the buffer
    NSString *kernelPath = [[NSBundle mainBundle] pathForResource:@"gran" ofType:@"ocl"];
    
    const char *aSource = [[NSString stringWithContentsOfFile:kernelPath encoding:NSUTF8StringEncoding error:nil] UTF8String];
    size_t kernelLength = strlen(aSource);
    program = clCreateProgramWithSource(context, 1, &aSource, &kernelLength, NULL);
    
    /*Step 6: Build program. */
    cl_int status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    /*Step 7: Initial inputs and output for the host and create memory objects for the kernel*/
    cl_int minimumGranularity = 0;
    outputMemObj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(minimumGranularity), NULL, NULL);
    
    /*Step 8: Create kernel object */
    kernel = clCreateKernel(program, "QueryMinimumGranularity", NULL);
    
    /*Step 9: Sets Kernel arguments.*/
    cl_int inputArg = 1000;
    status |= clSetKernelArg(kernel, 0, sizeof(inputArg), &inputArg);
    status |= clSetKernelArg(kernel, 1, sizeof(outputMemObj), &outputMemObj);
    
    /*Step 10: Running the kernel.*/
    size_t groupSize;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(groupSize), &groupSize, NULL);
    size_t global_work_size[1] = { groupSize };
    cl_device_type deviceType;
    clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);
    // As to Intel Processor, the local work size must be ONE
    if(deviceType != CL_DEVICE_TYPE_GPU)
        groupSize = 1;
    size_t local_work_size[1] = { groupSize };
    status |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    clFinish(commandQueue);     // Force wait until the OpenCL kernel is completed
    
    /*Step 11: Read the cout put back to host memory.*/
    status |= clEnqueueReadBuffer(commandQueue, outputMemObj, CL_TRUE, 0, sizeof(minimumGranularity), &minimumGranularity, 0, NULL, NULL);
    
    if(status != CL_SUCCESS)
        NSLog(@"Minimum granularity fetch failed!");
    
    clReleaseMemObject(outputMemObj);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);
    
    /** 一般属性查询 */
    char strBuf[2048];
    cl_bool boolValue;
    cl_uint intValue;
    cl_ulong longValue;
    NSString *contents;
    
    [mContentKeys addObject:@"Device Profile"];
    clGetDeviceInfo(device, CL_DEVICE_PROFILE, 256, strBuf, NULL);
    [mContentValues addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
    
    [mContentKeys addObject:@"Device Type"];
    switch(deviceType)
    {
        case CL_DEVICE_TYPE_CPU:
            contents = @"CPU";
            break;
        
        case CL_DEVICE_TYPE_GPU:
            contents = @"GPU";
            break;
        
        case CL_DEVICE_TYPE_ACCELERATOR:
            contents = @"Accelerator";
            break;
        
        case CL_DEVICE_TYPE_DEFAULT:
        case CL_DEVICE_TYPE_CUSTOM:
            contents = @"Custom";
        break;
        
        default:
            contents = @"Unknown";
            break;
    }
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Device Vendor"];
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, 256, strBuf, NULL);
    [mContentValues addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
    
    [mContentKeys addObject:@"Device Version"];
    clGetDeviceInfo(device, CL_DEVICE_VERSION, 256, strBuf, NULL);
    [mContentValues addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
    
    [mContentKeys addObject:@"Driver Version"];
    clGetDeviceInfo(device, CL_DRIVER_VERSION, 256, strBuf, NULL);
    [mContentValues addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
    
    [mContentKeys addObject:@"OpenCL C Version"];
    clGetDeviceInfo(device, CL_DEVICE_OPENCL_C_VERSION, 256, strBuf, NULL);
    [mContentValues addObject:[NSString stringWithCString:strBuf encoding:NSASCIIStringEncoding]];
    
    [mContentKeys addObject:@"Device Available"];
    clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, 256, &boolValue, NULL);
    contents = boolValue? @"YES" : @"NO";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Compiler Available"];
    clGetDeviceInfo(device, CL_DEVICE_COMPILER_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    contents = boolValue? @"YES" : @"NO";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Linker Available"];
    clGetDeviceInfo(device, CL_DEVICE_LINKER_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    contents = boolValue? @"YES" : @"NO";
    [mContentValues addObject:contents];
    
    cl_device_exec_capabilities capabilities;
    [mContentKeys addObject:@"Execution Capabilities"];
    clGetDeviceInfo(device, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(capabilities), &capabilities, NULL);
    contents = (capabilities & CL_EXEC_KERNEL) != 0? @"The OpenCL device can execute OpenCL kernels" : @"The OpenCL device can not execute OpenCL kernels";
    [mContentValues addObject:contents];
    contents = (capabilities & CL_EXEC_NATIVE_KERNEL) != 0? @"The OpenCL device can execute native kernels" : @"The OpenCL device can not execute native kernels";
    [mContentKeys addObject:@""];
    [mContentValues addObject:contents];
    
    cl_command_queue_properties queueProperties;
    [mContentKeys addObject:@"Queue Properties"];
    clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queueProperties), &queueProperties, NULL);
    contents = (queueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0? @"The commands in the command-queue are executed out-of-order" : @"The commands in the command-queue are executed in-order";
    [mContentValues addObject:contents];
    [mContentKeys addObject:@""];
    contents = (queueProperties & CL_QUEUE_PROFILING_ENABLE) != 0? @"The profiling of commands is enabled" : @"The profiling of commands is disabled";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Device Little Endian"];
    clGetDeviceInfo(device, CL_DEVICE_COMPILER_AVAILABLE, sizeof(boolValue), &boolValue, NULL);
    contents = boolValue? @"YES" : @"NO";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Max Compute Units"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Max Work item Dimensions"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    size_t maxWorkItemSizes[3] = { 0 };
    [mContentKeys addObject:@"Max Work item Sizes"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), maxWorkItemSizes, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"(%lu, %lu, %lu)", maxWorkItemSizes[0], maxWorkItemSizes[1], maxWorkItemSizes[2]]];
    
    [mContentKeys addObject:@"Max Work Group Size"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkItemSizes), maxWorkItemSizes, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%lu", maxWorkItemSizes[0]]];
    
    [mContentKeys addObject:@"Minimum Parallel Granularity"];
    [mContentValues addObject:[NSString stringWithFormat:@"%d", minimumGranularity]];
    
    [mContentKeys addObject:@"Preferred vector width char"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width short"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width int"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width long"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width float"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width double"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Preferred vector width half"];
    clGetDeviceInfo(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width char"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width short"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width int"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width long"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width float"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width double"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Native vector width half"];
    clGetDeviceInfo(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Max Clock Frequency"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%uMHz", intValue]];
    
    [mContentKeys addObject:@"Address Bits"];
    clGetDeviceInfo(device, CL_DEVICE_ADDRESS_BITS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u bits", intValue]];
    
    [mContentKeys addObject:@"Max Memory Alloc Size"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%lluMB", longValue / (1024 * 1024)]];
    
    [mContentKeys addObject:@"Image Support"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(boolValue), &boolValue, NULL);
    contents = boolValue? @"YES" : @"NO";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Max Read Image Args"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Max Write Image Args"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"2D Image Max Width"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"2D Image Max Height"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"3D Image Max Width"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"3D Image Max Height"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"3D Image Max Depth"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"Image Max Buffer Size"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu pixels", longValue]];
    
    [mContentKeys addObject:@"Image Max Array Size"];
    clGetDeviceInfo(device, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu images", longValue]];
    
    [mContentKeys addObject:@"Max Samplers"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_SAMPLERS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Max Parameter Size"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu bytes", longValue]];
    
    [mContentKeys addObject:@"Memory Base Address Alignment"];
    clGetDeviceInfo(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u bytes", intValue / 8]];
    
    cl_device_fp_config fpConfig = 0;
    [mContentKeys addObject:@"Single Float-point Config"];
    clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(fpConfig), &fpConfig, NULL);
    if((fpConfig & CL_FP_DENORM) != 0)
        [mContentValues addObject:@"Denorms are supported"];
    else
        [mContentValues addObject:@"Denorms are not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_INF_NAN) != 0)
        [mContentValues addObject:@"INF and quiet NaNs are supported"];
    else
        [mContentValues addObject:@"INF and quiet NaNs are not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_NEAREST) != 0)
        [mContentValues addObject:@"Round to nearest even rounding mode supported"];
    else
        [mContentValues addObject:@"Round to nearest even rounding mode not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_ZERO) != 0)
        [mContentValues addObject:@"Round to zero rounding mode supported"];
    else
        [mContentValues addObject:@"Round to zero rounding mode not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_INF) != 0)
        [mContentValues addObject:@"Round to positive and negative infinity rounding modes supported"];
    else
        [mContentValues addObject:@"Round to positive and negative infinity rounding modes not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_FMA) != 0)
        [mContentValues addObject:@"IEEE754-2008 fused multiply-add is supported"];
    else
        [mContentValues addObject:@"IEEE754-2008 fused multiply-add is not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) != 0)
        [mContentValues addObject:@"Divide and sqrt are correctly rounded as defined by the IEEE754 specification"];
    else
        [mContentValues addObject:@"Divide and sqrt are not correctly rounded as defined by the IEEE754 specification"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_SOFT_FLOAT) != 0)
        [mContentValues addObject:@"Basic floating-point operations are implemented in software"];
    else
        [mContentValues addObject:@"Basic floating-point operations are implemented in hardware"];
    
    [mContentKeys addObject:@"Double Float-point Config"];
    clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(fpConfig), &fpConfig, NULL);
    if((fpConfig & CL_FP_DENORM) != 0)
        [mContentValues addObject:@"Denorms are supported"];
    else
        [mContentValues addObject:@"Denorms are not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_INF_NAN) != 0)
        [mContentValues addObject:@"INF and quiet NaNs are supported"];
    else
        [mContentValues addObject:@"INF and quiet NaNs are not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_NEAREST) != 0)
        [mContentValues addObject:@"Round to nearest even rounding mode supported"];
    else
        [mContentValues addObject:@"Round to nearest even rounding mode not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_ZERO) != 0)
        [mContentValues addObject:@"Round to zero rounding mode supported"];
    else
        [mContentValues addObject:@"Round to zero rounding mode not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_ROUND_TO_INF) != 0)
        [mContentValues addObject:@"Round to positive and negative infinity rounding modes supported"];
    else
        [mContentValues addObject:@"Round to positive and negative infinity rounding modes not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_FMA) != 0)
        [mContentValues addObject:@"IEEE754-2008 fused multiply-add is supported"];
    else
        [mContentValues addObject:@"IEEE754-2008 fused multiply-add is not supported"];
    [mContentKeys addObject:@""];
    if((fpConfig & CL_FP_SOFT_FLOAT) != 0)
        [mContentValues addObject:@"Basic floating-point operations are implemented in software"];
    else
        [mContentValues addObject:@"Basic floating-point operations are implemented in hardware"];
    
    cl_device_mem_cache_type cacheType = 0;
    [mContentKeys addObject:@"Global Memory Cache Type"];
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cacheType), &cacheType, NULL);
    switch(cacheType)
    {
        case CL_NONE:
        default:
            contents = @"Not supported";
            break;
            
        case CL_READ_ONLY_CACHE:
            contents = @"Read write";
            break;
            
        case CL_READ_WRITE_CACHE:
            contents = @"Read only";
            break;
    }
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Globla Memory Cache line size"];
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u bytes", intValue]];
    
    [mContentKeys addObject:@"Globla Memory Cache size"];
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu Kbytes", longValue / 1024]];
    
    [mContentKeys addObject:@"Globla Memory size"];
    clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu Mbytes", longValue / 1024 / 1024]];
    
    [mContentKeys addObject:@"Max Constant Buffer Size"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu Kbytes", longValue / 1024]];
    
    [mContentKeys addObject:@"Max Constant Buffer Args"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
    
    [mContentKeys addObject:@"Local Memory Type"];
    cl_device_local_mem_type localMemType = 0;
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(localMemType), &localMemType, NULL);
    switch (localMemType)
    {
        case CL_NONE:
        default:
            contents = @"NOt supported";
            break;
            
        case CL_LOCAL:
            contents = @"Dedicated local memory";
            break;
            
        case CL_GLOBAL:
            contents = @"Global memory";
            break;
    }
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Local Memory Size"];
    clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu Kbytes", longValue / 1024]];
    
    [mContentKeys addObject:@"Error Correction Support"];
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(boolValue), &boolValue, NULL);
    if(boolValue)
        contents = @"YES";
    else
        contents = @"NO";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Device-host Unified Memory"];
    clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(boolValue), &boolValue, NULL);
    contents = boolValue? @"Device and the host have a unified memory subsystem" : @"Device and the host do not have a unified memory subsystem";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Device Profiling Timer Resolution"];
    clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu nanoseconds", longValue]];
    
    [mContentKeys addObject:@"Built-in Kernels"];
    clGetDeviceInfo(device, CL_DEVICE_BUILT_IN_KERNELS, sizeof(strBuf), strBuf, NULL);
    NSArray *array = [self parseStringArrayWithSeparator:';' string:strBuf length:strlen(strBuf)];
    NSUInteger length = [array count];
    if(length == 0)
        [mContentValues addObject:@"None"];
    else
    {
        [mContentValues addObject:[array objectAtIndex:0]];
        
        for(NSUInteger i = 1; i < length; i++)
        {
            [mContentKeys addObject:@""];
            [mContentValues addObject:[array objectAtIndex:i]];
        }
    }
    
    [mContentKeys addObject:@"Extensions"];
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(strBuf), strBuf, NULL);
    array = [self parseStringArrayWithSeparator:' ' string:strBuf length:strlen(strBuf)];
    length = [array count];
    if(length == 0)
        [mContentValues addObject:@"None"];
    else
    {
        [mContentValues addObject:[array objectAtIndex:0]];
        
        for(NSUInteger i = 1; i < length; i++)
        {
            [mContentKeys addObject:@""];
            [mContentValues addObject:[array objectAtIndex:i]];
        }
    }
    
    [mContentKeys addObject:@"PRINTF Buffer Size"];
    clGetDeviceInfo(device, CL_DEVICE_PRINTF_BUFFER_SIZE, sizeof(longValue), &longValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%llu MBytes", longValue / 1024 / 1024]];
    
    [mContentKeys addObject:@"Preferred Inter-op User Sync"];
    clGetDeviceInfo(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(boolValue), &boolValue, NULL);
    if(boolValue)
        contents = @"The device’s preference is for the user to be responsible for synchronization, when sharing memory objects between OpenCL and other APIs such as OpenGL and DirectX";
    else
        contents = @"The device implementation has a performant path for performing synchronization of memory object shared between OpenCL and other APIs such as OpenGL and DirectX";
    [mContentValues addObject:contents];
    
    [mContentKeys addObject:@"Partition Max Sub-devices"];
    clGetDeviceInfo(device, CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(intValue), &intValue, NULL);
    [mContentValues addObject:[NSString stringWithFormat:@"%u", intValue]];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    
    // Init OpenCL info
    mPlatformArray = [[NSMutableArray alloc] initWithCapacity:16];
    mDeviceArray = [[NSMutableArray alloc] initWithCapacity:16];
    
    if(![self initOpenCLQuery])
        return;
    
    NSView *baseView = window.contentView;
    [window setStyleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSTexturedBackgroundWindowMask];
    
    NSTextField *platformLabel = [[NSTextField alloc] initWithFrame:CGRectMake(20.0f, baseView.bounds.size.height - 40.0f, 180.0f, 25.0f)];
    platformLabel.stringValue = @"Choose OpenCL platform";
    [platformLabel setEditable:NO];
    [platformLabel setBordered:NO];
    platformLabel.backgroundColor = window.backgroundColor;
    platformLabel.alignment = NSLeftTextAlignment;
    [baseView addSubview:platformLabel];
    [platformLabel release];
    
    NSComboBox *platformCombo = [[NSComboBox alloc] initWithFrame:CGRectMake(20.0f + platformLabel.frame.size.width, platformLabel.frame.origin.y, 400.0f, 25.0f)];
    platformCombo.tag = COMBO_CONTROL_TAG_PLATFORM;
    platformCombo.delegate = self;
    [platformCombo setEditable:NO];
    for(NSString *name in mPlatformArray)
        [platformCombo addItemWithObjectValue:name];
    [baseView addSubview:platformCombo];
    [platformCombo release];
    
    NSTextField *deviceLabel = [[NSTextField alloc] initWithFrame:CGRectMake(20.0f, platformLabel.frame.origin.y - 30.0f - 25.0f, 180.0f, 25.0f)];
    deviceLabel.stringValue = @"Choose OpenCL device";
    [deviceLabel setEditable:NO];
    [deviceLabel setBordered:NO];
    deviceLabel.backgroundColor = window.backgroundColor;
    deviceLabel.alignment = NSLeftTextAlignment;
    [baseView addSubview:deviceLabel];
    [deviceLabel release];
    
    NSComboBox *deviceCombo = [[NSComboBox alloc] initWithFrame:CGRectMake(20.0f + deviceLabel.frame.size.width, deviceLabel.frame.origin.y, 400.0f, 25.0f)];
    deviceCombo.tag = COMBO_CONTROL_TAG_DEVICE;
    deviceCombo.delegate = self;
    [deviceCombo setEditable:NO];
    [baseView addSubview:deviceCombo];
    [deviceCombo release];
    
    mScrollView = [[NSScrollView alloc] initWithFrame:CGRectMake(20.0f, deviceCombo.frame.origin.y - 30.0f - 320.0f, 600.0f, 320.0f)];
    mScrollView.borderType = NSBezelBorder;
    mScrollView.hasHorizontalScroller = YES;
    mScrollView.hasVerticalScroller = YES;
    [baseView addSubview:mScrollView];
    [mScrollView release];
    
    NSTableView *contentView = [[NSTableView alloc] initWithFrame:CGRectMake(0.0f, 0.0f, 600.0f, 320.0f)];
    contentView.tag = COMBO_CONTROL_TAG_CONTENTS;
    contentView.allowsColumnResizing = YES;
    contentView.dataSource = self;
    contentView.delegate = self;
    mScrollView.contentView.documentView = contentView;
    [contentView release];
    
    mFeatureNameCol = [[NSTableColumn alloc] initWithIdentifier:@"Name"];
    [mFeatureNameCol setEditable:NO];
    mFeatureNameCol.maxWidth = 300.0f;
    mFeatureNameCol.width = 200.0f;
    [contentView addTableColumn:mFeatureNameCol];
    [mFeatureNameCol release];
    
    mFeatureValueCol = [[NSTableColumn alloc] initWithIdentifier:@"Value"];
    [mFeatureValueCol setEditable:NO];
    mFeatureValueCol.maxWidth = 1200.0f;
    mFeatureValueCol.width = 400.0f;
    [contentView addTableColumn:mFeatureValueCol];
    [mFeatureValueCol release];
}

- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
    NSComboBox *combo = notification.object;
    
    if(combo.tag == COMBO_CONTROL_TAG_PLATFORM)
    {
        NSComboBox *deviceCombo = (NSComboBox*)[window.contentView viewWithTag:COMBO_CONTROL_TAG_DEVICE];
        [deviceCombo removeAllItems];
        
        [self setDeviceInfo:[combo indexOfSelectedItem]];
        
        for(NSString *name in mDeviceArray)
            [deviceCombo addItemWithObjectValue:name];
    }
    else
    {
        [self setOpenCLDevieContent:[combo indexOfSelectedItem]];
        NSTableView *contentView = mScrollView.contentView.documentView;
        [contentView reloadData];
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [mContentKeys count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if(aTableColumn == mFeatureNameCol)
        return [mContentKeys objectAtIndex:rowIndex];
    else
        return [mContentValues objectAtIndex:rowIndex];
}

@end












