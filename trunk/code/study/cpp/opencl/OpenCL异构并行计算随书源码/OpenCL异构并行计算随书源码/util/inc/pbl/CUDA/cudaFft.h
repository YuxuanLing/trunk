/**
 * @file
 * @author yyfn
 * @brief wrap for cufft
 *
 **/
#ifdef __CUDACC__
#pragma once
#include <cufft.h>
#include "../CUtil.h"

#define checkCufftError(errno) {\
	if(CUFFT_SUCCESS == errno){}\
	else if(CUFFT_INVALID_PLAN == errno){\
		printError("CUFFT was passed an invalid plan handle");\
		exit(0);\
	}else if(CUFFT_ALLOC_FAILED == errno){\
		printError("CUFFT failed to allocate GPU or CPU memory");\
		exit(0);\
	}else if(CUFFT_INVALID_VALUE == errno){\
		printError("User specified an invalid pointer or parameter");\
		exit(0);\
	}else if(CUFFT_INTERNAL_ERROR == errno){\
		printError("Used for all driver and internal CUFFT library errors");\
		exit(0);\
	}else if(CUFFT_EXEC_FAILED == errno){\
		printError("CUFFT failed to execute an FFT on the GPU");\
		exit(0);\
	}else if(CUFFT_SETUP_FAILED == errno){\
		printError("The CUFFT library failed to initialize");\
		exit(0);\
	}else if(CUFFT_INVALID_SIZE == errno){\
		printError("User specified an invalid transform size");\
		exit(0);\
	}\
}

#define cufftHandle_t cufftHandle

#define CufftCreateMany(handler, rank, n, inembed, istride, idist, onembed, ostride, odist, type, batch) \
		checkCufftError(cufftPlanMany(handler, rank, n, inembed, istride, idist, onembed, ostride, odist, type, batch))
#define CufftCreate1D(handler, nx, type, batch) checkCufftError(cufftPlan1d(handler, nx, type, batch))
#define CufftCreate2D(handler, nx, ny, type) checkCufftError(cufftPlan2d(handler, nx, ny, type))
#define CufftCreate3D(handler, nx, ny, nz, type) checkCufftError(cufftPlan3d(handler, nx, ny, nz, type))

#define CufftDestroy(handler) checkCufftError(cufftDestroy(handler))

#define CufftExecC2C(handler, idata, odata, direction) checkCufftError(cufftExecC2C(handler, idata, odata, direction))
#define CufftExecZ2Z(handler, idata, odata, direction) checkCufftError(cufftExecZ2Z(handler, idata, odata, direction))

#define CufftExecR2C(handler, idata, odata) checkCufftError(cufftExecR2C(handler, idata, odata))
#define CufftExecD2Z(handler, idata, odata) checkCufftError(cufftExecD2Z(handler, idata, odata))
#define CufftExecC2R(handler, idata, odata) checkCufftError(cufftExecC2R(handler, idata, odata))
#define CufftExecZ2D(handler, idata, odata) checkCufftError(cufftExecZ2D(handler, idata, odata))

#define CufftSetStream(handler, stream) checkCufftError(cufftSetStream(handler, stream))

#define CufftSetCompatibilityMode(handler, mode) checkCufftError(cufftSetCompatibilityMode(handler, mode))

namespace Cufft {

typedef cufftType Type;

class fft {
protected:
	cufftHandle_t h;
	fft() {
	}

public:
	virtual void c2c(cufftComplex *idata, cufftComplex *odata, int direction) {
		CufftExecC2C(h, idata, odata, direction);
	}

	virtual void z2z(cufftDoubleComplex *idata, cufftDoubleComplex *odata,
			int direction) {
		CufftExecZ2Z(h, idata, odata, direction);
	}

	virtual void r2c(cufftReal *idata, cufftComplex *odata) {
		CufftExecR2C(h, idata, odata);
	}

	virtual void d2z(cufftDoubleReal *idata, cufftDoubleComplex *odata) {
		CufftExecD2Z(h, idata, odata);
	}

	virtual void c2r(cufftComplex *idata, cufftReal *odata) {
		CufftExecC2R(h, idata, odata);
	}

	virtual void z2d(cufftDoubleComplex *idata, cufftDoubleReal *odata) {
		CufftExecZ2D(h, idata, odata);
	}

	virtual void setStream(cudaStream_t s) {
		CufftSetStream(h, s);
	}

	virtual void setCompatibilityMode(cufftCompatibility mode) {
		CufftSetCompatibilityMode(h, mode);
	}

};

class FFT1D: public fft {
public:
	FFT1D(int nx, Type type, int batch) {
		CufftCreate1D(&h, nx, type, batch);
	}

	FFT1D(int nx, Type type) {
		CufftCreate1D(&h, nx, type, 1);
	}
	~FFT1D() {
		CufftDestroy(h);
	}
};

class FFT2D: public fft {
public:
	FFT2D(int nx, int ny, Type type) {
		CufftCreate2D(&h, nx, ny, type);
	}

	~FFT2D() {
		CufftDestroy(h);
	}
};

class FFT3D: public fft {
public:
	FFT3D(int nx, int ny, int nz, Type type) {
		CufftCreate3D(&h, nx, ny, nz, type);
	}

	~FFT3D() {
		CufftDestroy(h);
	}
};

class FFTBatcher: public fft {
public:
	FFTBatcher(int rank, int *n, int *inembed, int istride, int idist,
			int *onembed, int ostride, int odist, Type type, int batch) {
		CufftCreateMany(&h, rank, n, inembed, istride, idist, onembed,
				ostride, odist, type, batch);
	}

	~FFTBatcher() {
		CufftDestroy(h);
	}
};

}

#endif
