#ifndef PBL_OCL_BUFFER_H_
#define PBL_OCL_BUFFER_H_

extern "C" PBLStatus_t pblOCLCreateBuffer(cl_context context, cl_mem_flags flag, size_t size, void* ptr, cl_mem *buf) {
    int err;
    *buf = clCreateBuffer(context, flag, size, ptr, &err);

    return pblMapOCLErrorToPBLStatus(err);
}

#endif
