#ifndef OCL_QUERY_HEADER
#define OCL_QUERY_HEADER

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include "cl/opencl.h"
#endif

/*
 对OpenCL平台进行初始化，获得当前环境可用的所有OpenCL平台
 @return 返回当前所获得OpenCl平台个数
*/
extern int CL_Query_InitPlatforms(void);

/*
 获得指定索引的平台名称
 @return 如果指定索引超出范围，返回空；否则返回相应OpenCL平台名称的字符串
*/
extern const char* CL_Query_GetPlatformName(int index);

/*
 根据当前选择的OpenCL平台索引初始化OpenCL设备对象列表
@return 如果指定索引超出范围，或设备获得失败，返回0；否则返回当前平台能支持的OpenCL设备
*/
extern int CL_Query_InitDevices(int platformIndex);

/*
 获得指定索引的设备名称
@return 如果指定索引超出范围，返回空；否则返回相应设备名称字符串
*/
extern const char* CL_Query_GetDeviceName(int index);

/*
 获得当前OpenCL设备的OpenCL特征
*/
extern int CL_Query_GetDeviceFeatures(int deviceIndex, const char *kernelFileName, const char* pOutFeatureNames[], char(*pOutFeatureDescs)[256]);

#endif

