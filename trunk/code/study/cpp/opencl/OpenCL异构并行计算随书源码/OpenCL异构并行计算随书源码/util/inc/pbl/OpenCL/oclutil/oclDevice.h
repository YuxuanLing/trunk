#ifndef H_OCL_DEVICES
#define H_OCL_DEVICES

/**
 * @brief get devices in platform, and allocate space for storing devices
 * @warning please free space of devices allocated in this function
 *
 * @param p		input, specify platform, device will be retrived from this platform
 * @param type		input, specify device type
 * @param numDevices	output, return device number with type in platform p
 * @param devices	output, return devices
 *
 * @return
 * 	0 means sucess
 * 	-1 means fail
 *
 */
extern PBLStatus_t pblOCLGetDevices(cl_platform_id p, cl_device_type type, cl_uint *numDevices, cl_device_id **devices){
	if(NULL == numDevices || NULL != *devices) return PBL_BAD_PARAM;

	cl_uint num;
	cl_int err = clGetDeviceIDs(p, type, 0, NULL, &num);
	if(CL_SUCCESS != err) return pblMapOCLErrorToPBLStatus(err);

	*numDevices = num;

	cl_device_id *did = (cl_device_id*) malloc(num*sizeof(cl_device_id));
	if(NULL == did) return PBL_FAIL_TO_ALLOC;

	err = clGetDeviceIDs(p, type, num, did, NULL);
	if(CL_SUCCESS != err){
		free(did);
		return pblMapOCLErrorToPBLStatus(err);
	}

	*devices = did;

	return PBL_SUCCESS;
}
#define V_Q_Info(device, info, type, str, modifier) {\
	type v;\
	cl_int err = clGetDeviceInfo(device, info, sizeof(type), &v, NULL);\
	if(CL_SUCCESS != err){\
		checkCLError(err);\
		return pblMapOCLErrorToPBLStatus(err);\
	}\
\
	printf(str"%"modifier"\n", v);\
}

#define B_Q_Info(device, info, str) {\
	cl_bool v;\
	cl_int err = clGetDeviceInfo(device, info, sizeof(cl_bool), &v, NULL);\
	if(CL_SUCCESS != err){\
		checkCLError(err);\
		return pblMapOCLErrorToPBLStatus(err);\
	}\
\
	printf(str"%s\n", v? "Yes" : "NO");\
}

#define STR_Q_Info(device, info, str) {\
	size_t len;\
	cl_int err = clGetDeviceInfo(device, info, 0, NULL, &len);\
	if(CL_SUCCESS != err){\
		checkCLError(err);\
		return pblMapOCLErrorToPBLStatus(err);\
	}\
\
	char* v = (char*) malloc(len+1);\
	err = clGetDeviceInfo(device, info, len, v, NULL);\
	if(CL_SUCCESS != err){\
		checkCLError(err);\
		return pblMapOCLErrorToPBLStatus(err);\
	}\
\
	v[len] = '\0';\
	printf(str"%s\n", v);\
	free(v);\
\
}


/**
 * @brief list all attributes of device
 *
 * @param device		input
 *
 * @return
 * 	0 : success
 * 	-1: error
 */
extern PBLStatus_t pblOCLListDeviceInfo(cl_device_id device){
	printf("..........Device Info..............\n");
	STR_Q_Info(device, CL_DEVICE_NAME, "Device name : ");
	V_Q_Info(device, CL_DEVICE_ADDRESS_BITS, cl_uint, "Address Bits : ", "u");
	B_Q_Info(device, CL_DEVICE_AVAILABLE, "Device Available : ");
	B_Q_Info(device, CL_DEVICE_COMPILER_AVAILABLE, "Device Compiler Available : ");
	B_Q_Info(device, CL_DEVICE_ENDIAN_LITTLE, "Device is little Endian : ");
	B_Q_Info(device, CL_DEVICE_ERROR_CORRECTION_SUPPORT, "ECC Supported : ");
	STR_Q_Info(device, CL_DEVICE_EXTENSIONS, "Device Extensions : ");
	STR_Q_Info(device, CL_DEVICE_OPENCL_C_VERSION, "OpenCL C Version : ");
	STR_Q_Info(device, CL_DEVICE_PROFILE, "Device Profile : ");
	V_Q_Info(device, CL_DEVICE_PROFILING_TIMER_RESOLUTION, size_t, "Timer Resolution : ", "ld");
	{ cl_device_fp_config v;
		cl_int err = clGetDeviceInfo(device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &v, NULL);
		if(CL_SUCCESS != err){
			checkCLError(err);
			return pblMapOCLErrorToPBLStatus(err);
		}
	
		if(v & CL_FP_DENORM){printf("Device Support Denorm Single Float \n");}
		if(v & CL_FP_INF_NAN){printf("Device Support Single Float INF NAN\n");}
		if(v & CL_FP_ROUND_TO_NEAREST){printf("Device Support Single Float Round to Nearest\n");}
		if(v & CL_FP_ROUND_TO_ZERO){printf("Device Support Single Float Round to Zero \n");}
		if(v & CL_FP_ROUND_TO_INF){printf("Device Support Single Float Round to Inf\n");}
		if(v & CL_FP_FMA){printf("Device Support Single Float FMA\n");}
		if(v & CL_FP_SOFT_FLOAT){printf("Device does not Support Hardware Single Float\n");}
	}

	STR_Q_Info(device, CL_DEVICE_VENDOR, "Device Vendor : ");
	V_Q_Info(device, CL_DEVICE_VENDOR_ID, cl_uint, "Device Vendor ID : ", "u");
	STR_Q_Info(device, CL_DEVICE_VERSION, "Device Version : ");
	STR_Q_Info(device, CL_DRIVER_VERSION, "Driver Version : ");
	B_Q_Info(device, CL_DEVICE_HOST_UNIFIED_MEMORY, "Unified Memory Supported : ");
	V_Q_Info(device, CL_DEVICE_MAX_PARAMETER_SIZE, size_t, "Max Parameter Size : ", "ld");

	printf("..............Global Memory Configuration.............\n");
	V_Q_Info(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, cl_ulong, "Max Memory Allocate Size : ", "lu");
	V_Q_Info(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, cl_uint, "Max Base Address Align Size : ", "u");
	V_Q_Info(device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, cl_uint, "Min Data Type align Size :", "u");

	V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, cl_ulong, "Global Memory Cache Size : ", "lu");
	{ cl_device_mem_cache_type v;
		cl_int err = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &v, NULL);
		if(CL_SUCCESS != err){
			checkCLError(err);
			return pblMapOCLErrorToPBLStatus(err);
		}
		switch(v) {
		case CL_NONE: printf("Global Memory does not have Cache \n"); break;
		case CL_READ_ONLY_CACHE : printf("Global Memory has Readonly Cache \n"); break;
		case CL_READ_WRITE_CACHE : printf("Global Memory has Read Write Cache \n"); break;
		}
	}

	V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, cl_uint, "Global Memory, Cacheline Size : ", "u");
	V_Q_Info(device, CL_DEVICE_GLOBAL_MEM_SIZE, cl_ulong, "Global Memory Size : ", "lu");
//CL_DEVICE_HALF_FP_CONFIG

	printf("..................Image Information...................\n");
	B_Q_Info(device, CL_DEVICE_IMAGE_SUPPORT, "Image Supported : ");
	V_Q_Info(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, size_t, "2D Image Max Height : ", "ld");
	V_Q_Info(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, size_t, "2D Image Max Width : ", "ld");
	V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, size_t, "3D Image Max Depth : ", "ld");
	V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, size_t, "3D Image Max Height : ", "ld");
	V_Q_Info(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, size_t, "3D Image Max Width : ", "ld");
	V_Q_Info(device, CL_DEVICE_MAX_READ_IMAGE_ARGS, cl_uint, "Max Read Image Args : ", "u");
	V_Q_Info(device, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, cl_uint, "Max Write Image Args : ", "u");
	V_Q_Info(device, CL_DEVICE_MAX_SAMPLERS, cl_uint, "Max Samples : ", "u");

	printf(".................Local Memory...............................\n");
	V_Q_Info(device, CL_DEVICE_LOCAL_MEM_SIZE, cl_ulong, "Local Memory Size : ", "lu");
	{ cl_device_local_mem_type v;
		cl_int err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &v, NULL);
		if(CL_SUCCESS != err){
			checkCLError(err);
			return pblMapOCLErrorToPBLStatus(err);
		}
        switch(v) {
            case CL_LOCAL: printf("Device has Dedicate Local Memory\n"); break;
            case CL_GLOBAL : printf("Local Memory uses Global Memory\n"); break;
            default:
                             printMessage("error");
        }
	}

	printf("...................CU Information...........................\n");
	V_Q_Info(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, cl_uint, "Max Clock Frequency : ", "u");
	V_Q_Info(device, CL_DEVICE_MAX_COMPUTE_UNITS, cl_uint, "Max Compute Units : ", "u");

	printf(".................Constant Memory Information.............\n");
	V_Q_Info(device, CL_DEVICE_MAX_CONSTANT_ARGS, cl_uint, "Max Constant Args : ", "u");
	V_Q_Info(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, cl_ulong, "Max Constant Buffer Size : ", "lu");

	printf("...................ND Range Information........................\n");
	V_Q_Info(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, size_t, "Max Work Group Size : ", "ld");
	V_Q_Info(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, cl_uint, "Work Item Dimensions : ", "u");

	{ size_t v[3];
		cl_int err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t)*3, &v, NULL);
		if(CL_SUCCESS != err){
			checkCLError(err);
			return pblMapOCLErrorToPBLStatus(err);
		}
		printf("Max Work Item size : %ld %ld %ld\n", v[0], v[1], v[2]);
	}

	printf(".....................Vector Information..................\n");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, cl_uint, "Native Vector Width Char : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, cl_uint, "Native Vector Width Short : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, cl_uint, "Native Vector Width Int : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, cl_uint, "Native Vector Width Long : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, cl_uint, "Native Vector Width Float : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, cl_uint, "Native Vector Width Double : ", "u");
	V_Q_Info(device, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, cl_uint, "Native Vector Width Half : ", "u");

	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, cl_uint, "Preferred Vector Width Char : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, cl_uint, "Preferred Vector Width Short : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, cl_uint, "Preferred Vector Width Int : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, cl_uint, "Preferred Vector Width Long : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, cl_uint, "Preferred Vector Width Float : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, cl_uint, "Preferred Vector Width Double : ", "u");
	V_Q_Info(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, cl_uint, "Preferred Vector Width Half : ", "u");

}
#endif
