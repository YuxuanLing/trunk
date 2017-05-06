/**
 * @file
 * @author yyfn
 * @date 20120713
 *
 * @brief  Env class list driver versin and runtime version, I want to cover more
 **/

#ifdef __CUDACC__

#pragma once
#include "CUDAFunction.h"
/**
 * @class Env
 *
 */
class Env {
public:
	/**

	 * @brief get CUDA driver version
	 *
	 * @return driver version, 2020 if 2.2
	 *

	 **/
	static int getDriverVersion() {
		int driverVersion;
		CudaDriverGetVersion(&driverVersion);

		return driverVersion;
	}

	/**
	 * @brief get CUDA runtime version
	 * @return runtime version, 2020 if 2.2
	 *
	 **/
	static int getRuntimeVersion() {
		int runtimeVersion;
		CudaRuntimeGetVersion(&runtimeVersion);

		return runtimeVersion;
	}
}; //end of class

#endif
