#ifndef PBL_COMMON_H_
#define PBL_COMMON_H_ 

#define printMessage(msg) printf("%s:%d: %s\n", __FILE__, __LINE__, msg)

typedef enum {
    PBL_SUCCESS = 0,
    PBL_NOT_SUPPORTED = 1,
    PBL_NOT_IMPLEMENTED = 2,
    PBL_BAD_PARAM = 3,
    PBL_FAIL_TO_ALLOC = 4,

    PBL_FAIL_TO_OPEN_FILE ,
    PBL_FAIL_TO_SEEK_END ,
    PBL_FAIL_TO_READ_DATA ,
    PBL_UNKOWN_ERROR
} PBLStatus_t;

//TODO
void pblCheckError(PBLStatus_t status) {
    return ;
}

#ifdef PBL_USE_OPENCL
#include <CL/cl.h>
//TODO
PBLStatus_t pblMapOCLErrorToPBLStatus(int err) {
    if(CL_SUCCESS != err) return PBL_UNKOWN_ERROR;
    return PBL_SUCCESS;
}
#endif

#include "common/loadFileContent.h"

//TODO
//remove this 
//
#include "common/oclError.h"

#endif
