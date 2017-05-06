#include "cinclude.h"
#include <iostream>


#ifdef APPLE
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

void displayDeviceDetails(cl_device_id id, cl_device_info param_name, const char* paramNameAsStr);

//根据参数，判断设备类别。是CPU、GPU、ACCELERATOR或其他设备
const char* GetDeviceType(cl_device_type it)
{
	if(it == CL_DEVICE_TYPE_CPU)
		return "CPU";
	else if(it== CL_DEVICE_TYPE_GPU)
		return "GPU";
	else if(it==CL_DEVICE_TYPE_ACCELERATOR)
		return "ACCELERATOR";
	else
		return "DEFAULT";

}


int getPlatFormSimpleDeviceInfo()
{
	char dname[512];
	cl_device_id devices[20];
	cl_platform_id* platform_id = NULL;
	cl_uint num_devices;
	cl_device_type int_type;
	cl_ulong long_entries;
	cl_uint num_platform;
	cl_int err;

	//查询系统上可用的计算平台，可以理解为初始化
	err = clGetPlatformIDs(0, NULL, &num_platform);

	if (err != CL_SUCCESS)
	{
		cout << "clGetPlatformIDs error" << endl;
		return 0;
	}

	cout << "PlatForm num:" << num_platform << endl;

	unsigned int st = 0;

	platform_id = new cl_platform_id[num_platform];

	err = clGetPlatformIDs(num_platform, platform_id, NULL);

	if (err != CL_SUCCESS)
	{
		cout << "clGetPlatformIDs error" << endl;
		return 0;
	}


	for (st = 0; st < num_platform; st++)
	{
		cout << endl << "----------------------------------" << endl;
		cout << "Platform " << st + 1 << endl;

		//获取可用计算平台的名称
		clGetPlatformInfo(platform_id[st], CL_PLATFORM_NAME, 512, dname, NULL);
		cout << "CL_PLATFORM_NAME:" << dname << endl;

		//获取可用计算平台的版本号,即OpenCL的版本号
		clGetPlatformInfo(platform_id[st], CL_PLATFORM_VENDOR, 512, dname, NULL);
		cout << "CL_PLATFORM_VERSION:" << dname << endl;

		//获取可用计算平台的设备数目
		clGetDeviceIDs(platform_id[st], CL_DEVICE_TYPE_ALL, 20, devices, &num_devices);
		cout << "Device num:" << num_devices << endl << endl;

		unsigned int n = 0;

		//循环两次，检测两个设备的属性
		for (n = 0; n < num_devices; n++)
		{
			cout << endl << "Device " << n + 1 << endl;
			//获取设备名称
			clGetDeviceInfo(devices[n], CL_DEVICE_NAME, 512, dname, NULL);
			cout << "Device :" << dname << endl;

			//获取设备类别
			clGetDeviceInfo(devices[n], CL_DEVICE_TYPE, sizeof(cl_device_type), &int_type, NULL);
			cout << "Device Type:" << GetDeviceType(int_type) << endl;

			//获取设备版本号
			clGetDeviceInfo(devices[n], CL_DRIVER_VERSION, 512, dname, NULL);
			cout << "Device version:" << dname << endl;

			//获取设备全局内存大小
			clGetDeviceInfo(devices[n], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device global mem(MB):" << long_entries / 1024 / 1024 << endl;

			//获取设备CACHE内存大小
			clGetDeviceInfo(devices[n], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device global mem cache(KB):" << long_entries / 1024 << endl;

			//获取本地内存大小
			clGetDeviceInfo(devices[n], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device Locale mem(KB) :" << long_entries / 1024 << endl;

			//获取设备频率
			clGetDeviceInfo(devices[n], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device Max clock(MHz) :" << long_entries << endl;

			//获取最大工作组数
			clGetDeviceInfo(devices[n], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device Max Group size :" << long_entries << endl;

			//获取最大计算核心数
			clGetDeviceInfo(devices[n], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_ulong), &long_entries, NULL);
			cout << "Device Max parallel cores:" << long_entries << endl;
		}
	}

	return 1;
}




void displayPlatformInfo(cl_platform_id id,
	cl_platform_info param_name,
	const char* paramNameAsStr) {
	cl_int error = 0;
	size_t paramSize = 0;
	error = clGetPlatformInfo(id, param_name, 0, NULL, &paramSize);
	char* moreInfo = (char*)alloca(sizeof(char) * paramSize);
	error = clGetPlatformInfo(id, param_name, paramSize, moreInfo, NULL);
	if (error != CL_SUCCESS) {
		perror("Unable to find any OpenCL platform information");
		return;
	}
	printf("%s: %s\n", paramNameAsStr, moreInfo);
}

void displayDeviceInfo(cl_platform_id id,
	cl_device_type dev_type) {
	/* OpenCL 1.1 device types */
	cl_int error = 0;
	cl_uint numOfDevices = 0;

	/* Determine how many devices are connected to your platform */
	error = clGetDeviceIDs(id, dev_type, 0, NULL, &numOfDevices);
	if (error != CL_SUCCESS) {
		perror("Unable to obtain any OpenCL compliant device info");
		exit(1);
	}
	cl_device_id* devices = (cl_device_id*)alloca(sizeof(cl_device_id) * numOfDevices);

	/* Load the information about your devices into the variable 'devices' */
	error = clGetDeviceIDs(id, dev_type, numOfDevices, devices, NULL);
	if (error != CL_SUCCESS) {
		perror("Unable to obtain any OpenCL compliant device info");
		exit(1);
	}
	printf("Number of detected OpenCL devices: %d\n", numOfDevices);
	/* We attempt to retrieve some information about the devices. */
	for (cl_uint i = 0; i < numOfDevices; ++i) {
		displayDeviceDetails(devices[i], CL_DEVICE_TYPE, "CL_DEVICE_TYPE");
		displayDeviceDetails(devices[i], CL_DEVICE_NAME, "CL_DEVICE_NAME");
		displayDeviceDetails(devices[i], CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR");
		displayDeviceDetails(devices[i], CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID");
		displayDeviceDetails(devices[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, "CL_DEVICE_MAX_MEM_ALLOC_SIZE");
		displayDeviceDetails(devices[i], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE");
		displayDeviceDetails(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE, "CL_DEVICE_GLOBAL_MEM_SIZE");
		displayDeviceDetails(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, "CL_DEVICE_MAX_COMPUTE_UNITS");
		displayDeviceDetails(devices[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS");
		displayDeviceDetails(devices[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, "CL_DEVICE_MAX_WORK_ITEM_SIZES");
		displayDeviceDetails(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, "CL_DEVICE_MAX_WORK_GROUP_SIZE");
	}
}

void displayDeviceDetails(cl_device_id id,
	cl_device_info param_name,
	const char* paramNameAsStr) {
	cl_int error = 0;
	size_t paramSize = 0;

	error = clGetDeviceInfo(id, param_name, 0, NULL, &paramSize);
	if (error != CL_SUCCESS) {
		perror("Unable to obtain device info for param\n");
		return;
	}

	/* the cl_device_info are preprocessor directives defined in cl.h */
	switch (param_name) {
	case CL_DEVICE_TYPE: {
		cl_device_type* devType = (cl_device_type*)alloca(sizeof(cl_device_type) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, devType, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device info for param\n");
			return;
		}
		switch (*devType) {
		case CL_DEVICE_TYPE_CPU: printf("CPU detected\n"); break;
		case CL_DEVICE_TYPE_GPU: printf("GPU detected\n"); break;
		case CL_DEVICE_TYPE_ACCELERATOR: printf("Accelerator detected\n"); break;
		case CL_DEVICE_TYPE_DEFAULT: printf("default detected\n"); break;
		}
	}break;
	case CL_DEVICE_VENDOR_ID:
	case CL_DEVICE_MAX_COMPUTE_UNITS:
	case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: {
		cl_uint* ret = (cl_uint*)alloca(sizeof(cl_uint) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, ret, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device info for param\n");
			return;
		}
		switch (param_name) {
		case CL_DEVICE_VENDOR_ID: printf("\tVENDOR ID: 0x%x\n", *ret); break;
		case CL_DEVICE_MAX_COMPUTE_UNITS: printf("\tMaximum number of parallel compute units: %d\n", *ret); break;
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: printf("\tMaximum dimensions for global/local work-item IDs: %d\n", *ret); break;
		}
	}break;
	case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
		cl_uint maxWIDimensions;
		size_t* ret = (size_t*)alloca(sizeof(size_t) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, ret, NULL);

		error = clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &maxWIDimensions, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device info for param\n");
			return;
		}
		printf("\tMaximum number of work-items in each dimension: ( ");
		for (cl_uint i = 0; i < maxWIDimensions; ++i) {
			printf("%d ", ret[i]);
		}
		printf(" )\n");
	}break;
	case CL_DEVICE_MAX_WORK_GROUP_SIZE: {
		size_t* ret = (size_t*)alloca(sizeof(size_t) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, ret, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device info for param\n");
			return;
		}
		printf("\tMaximum number of work-items in a work-group: %d\n", *ret);
	}break;
	case CL_DEVICE_NAME:
	case CL_DEVICE_VENDOR: {
		char data[48];
		error = clGetDeviceInfo(id, param_name, paramSize, data, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device name/vendor info for param\n");
			return;
		}
		switch (param_name) {
		case CL_DEVICE_NAME: printf("\tDevice name is %s\n", data); break;
		case CL_DEVICE_VENDOR: printf("\tDevice vendor is %s\n", data); break;
		}
	} break;
	case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE: {
		cl_uint* size = (cl_uint*)alloca(sizeof(cl_uint) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, size, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device name/vendor info for param\n");
			return;
		}
		printf("\tDevice global cacheline size: %d bytes\n", (*size)); break;
	} break;
	case CL_DEVICE_GLOBAL_MEM_SIZE:
	case CL_DEVICE_MAX_MEM_ALLOC_SIZE: {
		cl_ulong* size = (cl_ulong*)alloca(sizeof(cl_ulong) * paramSize);
		error = clGetDeviceInfo(id, param_name, paramSize, size, NULL);
		if (error != CL_SUCCESS) {
			perror("Unable to obtain device name/vendor info for param\n");
			return;
		}
		switch (param_name) {
		case CL_DEVICE_GLOBAL_MEM_SIZE: printf("\tDevice global mem: %lld mega-bytes\n", (*size) >> 20); break;
		case CL_DEVICE_MAX_MEM_ALLOC_SIZE: printf("\tDevice max memory allocation: %lld mega-bytes\n", (*size) >> 20); break;
		}
	} break;

	} //end of switch

}




int getPlatFormDetialDeviceInfo()
{
	/* OpenCL 1.1 data structures */
	cl_platform_id* platforms;

	/* OpenCL 1.1 scalar data types */
	cl_uint numOfPlatforms;
	cl_int  error;

	/*
	Get the number of platforms
	Remember that for each vendor's SDK installed on the computer,
	the number of available platform also increased.
	*/
	error = clGetPlatformIDs(0, NULL, &numOfPlatforms);
	if (error != CL_SUCCESS) {
		perror("Unable to find any OpenCL platforms");
		exit(1);
	}

	// Allocate memory for the number of installed platforms.
	// alloca(...) occupies some stack space but is automatically freed on return
	platforms = (cl_platform_id*)alloca(sizeof(cl_platform_id) * numOfPlatforms);
	printf("Number of OpenCL platforms found: %d\n", numOfPlatforms);

	error = clGetPlatformIDs(numOfPlatforms, platforms, NULL);
	if (error != CL_SUCCESS) {
		perror("Unable to find any OpenCL platforms");
		exit(1);
	}
	// We invoke the API 'clPlatformInfo' twice for each parameter we're trying to extract
	// and we use the return value to create temporary data structures (on the stack) to store
	// the returned information on the second invocation.
	for (cl_uint i = 0; i < numOfPlatforms; ++i) {
		displayPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE");
		displayPlatformInfo(platforms[i], CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION");
		displayPlatformInfo(platforms[i], CL_PLATFORM_NAME, "CL_PLATFORM_NAME");
		displayPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR");
		displayPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS");
		// Assume that we don't know how many devices are OpenCL compliant, we locate everything !
		displayDeviceInfo(platforms[i], CL_DEVICE_TYPE_ALL);
	}

	return 1;
}