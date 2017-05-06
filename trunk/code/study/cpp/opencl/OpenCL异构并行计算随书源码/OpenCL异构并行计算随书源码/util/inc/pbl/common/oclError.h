#ifndef H_CL_ERROR
#define H_CL_ERROR

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/**
 * @file
 */
/**
 *
 * @brief check OpenCL Error Code and print the error message
 *
 */
#define checkCLError(err)  {\
	int error = err;\
	if(CL_SUCCESS != error){\
		printMessage(oclGetErrorString(error));\
	}\
}

/**
 * @brief obtain the string representaion of error code
 *
 * @param err		input, error code
 */ 
const char* oclGetErrorString(int err){
	cl_int error = err;
	const char* ret;

	switch(error){
	case CL_SUCCESS:
        	ret = "Success!"; break;
        case CL_DEVICE_NOT_FOUND:
		ret = "Device not found"; break;
        case CL_DEVICE_NOT_AVAILABLE:
		ret = "Device not available"; break;
        case CL_COMPILER_NOT_AVAILABLE:
		ret = "Compiler not available"; break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
		ret = "Memory object allocation failure"; break;
        case CL_OUT_OF_RESOURCES:
		ret = "Out of resources"; break;
        case CL_OUT_OF_HOST_MEMORY:
		ret = "Out of host memory"; break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:
		ret = "Profiling information not available"; break;
        case CL_MEM_COPY_OVERLAP:
		ret = "Memory copy overlap"; break;
        case CL_IMAGE_FORMAT_MISMATCH:
		ret = "Image format mismatch"; break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		ret = "Image format not supported"; break;
        case CL_BUILD_PROGRAM_FAILURE:
		ret = "Program build failure"; break;
        case CL_MAP_FAILURE:
		ret = "Map failure"; break;
#ifdef CL_VERSION_1_1
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:
		ret = "Misaligned sub buffer offset"; break;
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
		ret = "Execuation status error for events wait list"; break;
#endif
#ifdef CL_VERSION_1_2
	case CL_COMPILE_PROGRAM_FAILURE:
		ret = "Compile program failure"; break; 
	case CL_LINKER_NOT_AVAILABLE: 
		ret = "Linker not available"; break;
	case CL_LINK_PROGRAM_FAILURE:
		ret = "Link program failure"; break;
	case CL_DEVICE_PARTITION_FAILED:
		ret = "Device partition failed"; break;
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: 
		ret = "Kernel arg info not availabel"; break;
#endif
        case CL_INVALID_VALUE:
		ret = "Invalid value"; break;
        case CL_INVALID_DEVICE_TYPE:
		ret = "Invalid device type"; break;
        case CL_INVALID_PLATFORM:
		ret = "Invalid platform"; break;
        case CL_INVALID_DEVICE:
		ret = "Invalid device"; break;
        case CL_INVALID_CONTEXT:
		ret = "Invalid context"; break;
        case CL_INVALID_QUEUE_PROPERTIES:
		ret = "Invalid queue properties"; break;
        case CL_INVALID_COMMAND_QUEUE:
		ret = "Invalid command queue"; break;
        case CL_INVALID_HOST_PTR:
		ret = "Invalid host pointer"; break;
        case CL_INVALID_MEM_OBJECT:
		ret = "Invalid memory object"; break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
		ret = "Invalid image format descriptor"; break;
        case CL_INVALID_IMAGE_SIZE:
		ret = "Invalid image size"; break;
        case CL_INVALID_SAMPLER:
		ret = "Invalid sampler"; break;
        case CL_INVALID_BINARY:
		ret = "Invalid binary"; break;
        case CL_INVALID_BUILD_OPTIONS:
		ret = "Invalid build options"; break;
        case CL_INVALID_PROGRAM:
		ret = "Invalid program"; break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
		ret = "Invalid program executable"; break;
        case CL_INVALID_KERNEL_NAME:
		ret = "Invalid kernel name"; break;
        case CL_INVALID_KERNEL_DEFINITION:
		ret = "Invalid kernel definition"; break;
        case CL_INVALID_KERNEL:
		ret = "Invalid kernel"; break;
        case CL_INVALID_ARG_INDEX:
		ret = "Invalid argument index"; break;
        case CL_INVALID_ARG_VALUE:
		ret = "Invalid argument value"; break;
        case CL_INVALID_ARG_SIZE:
		ret = "Invalid argument size"; break;
        case CL_INVALID_KERNEL_ARGS:
		ret = "Invalid kernel arguments"; break;
        case CL_INVALID_WORK_DIMENSION:
		ret = "Invalid work dimension"; break;
        case CL_INVALID_WORK_GROUP_SIZE:
		ret = "Invalid work group size"; break;
        case CL_INVALID_WORK_ITEM_SIZE:
		ret = "Invalid work item size"; break;
        case CL_INVALID_GLOBAL_OFFSET:
		ret = "Invalid global offset"; break;
        case CL_INVALID_EVENT_WAIT_LIST:
		ret = "Invalid event wait list"; break;
        case CL_INVALID_EVENT:
		ret = "Invalid event"; break;
        case CL_INVALID_OPERATION:
		ret = "Invalid operation"; break;
        case CL_INVALID_GL_OBJECT:
		ret = "Invalid OpenGL object"; break;
        case CL_INVALID_BUFFER_SIZE:
		ret = "Invalid buffer size"; break;
        case CL_INVALID_MIP_LEVEL:
		ret = "Invalid mip-map level"; break;
	case CL_INVALID_GLOBAL_WORK_SIZE:
		ret = "Invalid global work size"; break;
#ifdef CL_VERSION_1_1
	case CL_INVALID_PROPERTY:
		ret = "Invalid property"; break;
#endif
#ifdef CL_VERSION_1_2
	case CL_INVALID_IMAGE_DESCRIPTOR:
		ret = "Invalid image descriptor"; break;
	case CL_INVALID_COMPILER_OPTIONS:
		ret = "Invalid compiler options"; break;
	case CL_INVALID_LINKER_OPTIONS:
		ret = "Invalid linker options"; break;
	case CL_INVALID_DEVICE_PARTITION_COUNT:
		ret = "Invalid device partition count"; break;
#endif

        default: 
		ret = "Unknown"; break;
	}

	return ret;
}
#endif
