#ifndef H_OCL_CONTEXT
#define H_OCL_CONTEXT 

/**
 * @brief create OpenCL Context based on platform
 *
 * @param p		input, context will be created on this platform
 * @param numDevices 	input, context will be created on numDevices cl_device
 * @param devices	input, context created contains devices
 * @param cont		output, return context created
 *
 * @return
 * 	0 means sucess
 * 	-1 means fail
 *
 */
extern PBLStatus_t pblOCLCreateContextWithPlatform(cl_platform_id p, int numDevices, const cl_device_id *devices, cl_context *cont){
	if(0 == numDevices || NULL == devices) return PBL_BAD_PARAM;

	cl_int err;

	cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)p, 0};
	cl_context context = clCreateContext(properties, numDevices, devices, NULL, NULL, &err);
	if(CL_SUCCESS != err){
		checkCLError(err);
		return pblMapOCLErrorToPBLStatus(err);
	}

	*cont = context;

	return PBL_SUCCESS;
}

/**
 * @brief list all information of context
 *
 * @param cont
 * 
 **/
extern PBLStatus_t pblOCLListContextInfo(cl_context cont){
	cl_uint numDevices;
	int err = clGetContextInfo(cont, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL);
	if(CL_SUCCESS != err){
		checkCLError(err);
		return pblMapOCLErrorToPBLStatus(err);
	}else{
		printf("There are %d devices in context\n", numDevices);
		return PBL_SUCCESS;
	}
//CL_CONTEXT_DEVICES
//CL_CONTEXT_PROPERTIES
}


#endif
