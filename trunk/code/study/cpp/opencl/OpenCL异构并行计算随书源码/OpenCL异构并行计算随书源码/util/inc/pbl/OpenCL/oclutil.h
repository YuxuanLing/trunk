#ifndef H_CL_UTIL
#define H_CL_UTIL

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/**
 * @file oclutil.h
 *
 * @brief OpenCL util lib
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "oclutil/oclProgram.h"
#include "oclutil/oclPlatform.h"
#include "oclutil/oclDevice.h"
#include "oclutil/oclContext.h"
#include "oclutil/oclKernel.h"
#include "oclutil/oclCommandQueue.h"
#include "oclutil/oclBuffer.h"

#endif
