#ifndef PBL_H_
#define PBL_H_
/**
 * @file 
 * @author yyfn
 * @date 2015年 05月 23日 星期六 10:40:59 CST
 * @version 1.0 
 *
 * @brief 
 * <h1> Parallel Base Library</h1>
 *
 * this is the top level header, user just need to include this file,
 * and all functin are in header, I want to use direcory to organize headers.
 */

#include "pbl/common.h"

#ifdef PBL_USE_OPENCL
#include "pbl/OpenCL.h"
#endif

#ifdef PBL_USE_CUDA
#include "pbl/CUDA.h"
#include "pbl/pictureFormat.h"
#ifdef __cplusplus
#include "pbl/random.h"
#include "pbl/timer.h"
#include "pbl/ymath.h"
#endif
#endif

#endif
