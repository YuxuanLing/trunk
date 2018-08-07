#ifndef H_OCL_PLATFORMS
#define H_OCL_PLATFORMS

/**
 * @file
 *
 */
/**
 *
 * @brief 
 *
 * @return 
 * 	-1 error
 * 	0 success
 *
 * @warning please free platforms
 *
 **/
extern PBLStatus_t pblOCLGetPlatforms(cl_uint *numPlatforms, cl_platform_id** platforms){
	if((NULL == numPlatforms) || (NULL != *platforms)) return PBL_BAD_PARAM;

	int err;

	cl_uint num;
	err = clGetPlatformIDs (0, NULL, &num);
	if(CL_SUCCESS != err){
		checkCLError(err);
		return PBL_SUCCESS;
	}else{
		cl_platform_id *p = (cl_platform_id*)malloc(num * sizeof(cl_platform_id));
		if(NULL == p) return PBL_FAIL_TO_ALLOC;

		err = clGetPlatformIDs(num, p, NULL);
		if(CL_SUCCESS != err){
			free(p);
			checkCLError(err);
		return PBL_SUCCESS;
		}

		*numPlatforms = num;
		*platforms = p;

		return PBL_SUCCESS;
	}
}

static int queryPlatformInfo(cl_platform_id p, cl_platform_info param, void** value, size_t *len){
	size_t sizeRet;
	int err = clGetPlatformInfo(p, param, 0, NULL, &sizeRet);
	if(CL_SUCCESS != err){
		checkCLError(err);
		return -1;
	}

	void* data = malloc(sizeRet+1);
	if(NULL == data){
		printMessage("fail to allocate memory\n");
		return -1;
	}

	err = clGetPlatformInfo(p, param, sizeRet+1, data, NULL);
	if(CL_SUCCESS != err){
		free(data);
		checkCLError(err);
		return -1;
	}

	*len = sizeRet;
	*value = data;

	return 0;
}
#define CHAR_PLATFORM_INFO(info, str) {\
	if(0 == queryPlatformInfo(p, info, &value, &len)){\
		char* tmp = (char*) value;\
		tmp[len] = '\0';\
		printf(str": %s\n", tmp);\
		free(value);\
	}else{ return PBL_UNKOWN_ERROR;}\
}

/**
 * @brief list information about platform
 *
 */
extern PBLStatus_t pblOCLListPlatformInfo(cl_platform_id p){
	void* value;
	size_t len;

	CHAR_PLATFORM_INFO(CL_PLATFORM_PROFILE, "\t Profile");
	CHAR_PLATFORM_INFO(CL_PLATFORM_VERSION, "\t Version ");
	CHAR_PLATFORM_INFO(CL_PLATFORM_NAME, "\t Name ");
	CHAR_PLATFORM_INFO(CL_PLATFORM_VENDOR, "\t Vendor ");
	CHAR_PLATFORM_INFO(CL_PLATFORM_EXTENSIONS, "\t Extensions ");

	return PBL_SUCCESS;
}

#endif
