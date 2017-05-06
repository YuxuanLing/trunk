#ifndef PBL_OCL_KERNEL_H_
#define PBL_OCL_KERNEL_H_

extern "C" PBLStatus_t pblOCLCreateKernel(cl_program program, const char* kernelName, cl_kernel *kernel) {
    int err;
    *kernel = clCreateKernel(program, kernelName, &err);

    if(CL_SUCCESS != err) checkCLError(err);

    return pblMapOCLErrorToPBLStatus(err);
}

#endif
