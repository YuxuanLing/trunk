#ifndef H_OCL_PROGRAM
#define H_OCL_PROGRAM

/**
 * @file
 *
 * @brief util functions for program
 *
 **/

#define checkCLBuildProgramError(err, program, numDevices, devices) {\
	if(CL_SUCCESS != err){\
		for(int i = 0; i < numDevices; i++){\
			size_t buildLogSize;\
			cl_int logStatus = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogSize);\
			if (logStatus != CL_SUCCESS){\
				checkCLError(logStatus);\
			}else{\
				char *buildLog = (char*)malloc(buildLogSize+1);\
				logStatus = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, buildLogSize+1, buildLog, NULL);\
				buildLog[buildLogSize] = '\0';\
				printf("build log of device %d : %s\n", i, buildLog);\
				free(buildLog);\
			}\
		}\
     }\
 }

/**
 * @brief create program then build it
 *
 * @param cont		input, specify associate context
 * @param source	input, source code
 * @param length	inptu, length of source
 * @param numDevices	input, specify number of devices
 * @param devices	input, source will be built on devices
 * @param options	input, options for compiling source
 * @param prog		output, created and built program
 *
 * @return
 * 	-1 means fail
 * 	0 means success
 *
 */
extern PBLStatus_t pblOCLCreateBuildProgramWithSource(cl_context& cont, const char* source, size_t length, int numDevices, cl_device_id *devices, const char* options, cl_program *prog){
	if((NULL == source) || (0 == length) || (NULL == devices) || (0 == numDevices)) return PBL_BAD_PARAM;
	
	int err;

	cl_program program = clCreateProgramWithSource(cont, 1, &source, &length, &err);
	if(CL_SUCCESS != err){
		checkCLError(err);
		return pblMapOCLErrorToPBLStatus(err);
	}

	err = clBuildProgram(program, numDevices, devices, options, NULL, NULL);                   
	if(CL_SUCCESS != err){
		checkCLBuildProgramError(err, program, numDevices, devices);
		return pblMapOCLErrorToPBLStatus(err);
	}

	*prog = program;

	return PBL_SUCCESS;
}

#endif
