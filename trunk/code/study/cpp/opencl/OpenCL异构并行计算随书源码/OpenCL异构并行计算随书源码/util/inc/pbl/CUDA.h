/**
 * @file CUDA.h
 * @author yyfn
 * @date 20120709
 *
 * @brief this file list all CUDA fuctions and some utilities
 **/

#pragma once

#ifdef __CUDACC__

#include "CUDA/Device.h"
#include "CUDA/Env.h"
#include "CUDA/yAtomic.h"
#include "CUDA/CUDAFunction.h"
#include "CUDA/cudaBlas.h"
#include "CUDA/cudaFft.h"
#include "CUDA/Curand.h"

#endif
