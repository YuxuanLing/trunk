#include "../inc/yyfnutil.h"

#define NX 20
#define BATCH 1

#define CUDA_SAFE_CALL 
#define warn printf

void runtest1d() {
	float *h_idata = (float*) Malloc(NX*BATCH*sizeof(float));

	for(int i=0;i<NX*BATCH;i++)
	{
		h_idata[i]=i;
		warn("h_idata[%d].r=%f\n",i,h_idata[i]);
	}


 	cufftHandle plan;
	cufftComplex *d_data;
	CUDA_SAFE_CALL(cudaMalloc((void**)&d_data,sizeof(cufftComplex)*NX*BATCH));
	CUDA_SAFE_CALL(cudaMemcpy((cufftReal*)d_data,h_idata,sizeof(float)*NX*BATCH,cudaMemcpyHostToDevice));

	//Real to Complex
	cufftPlan1d(&plan,NX,CUFFT_R2C,BATCH);
	cufftExecR2C(plan,(cufftReal*)d_data,d_data);
	//CUDA_SAFE_CALL(cudaMemcpy(h_odata,d_data,sizeof(cufftComplex)*NX*BATCH,cudaMemcpyDeviceToHost));
  	cufftDestroy(plan);

	//Complex to Real
	cufftPlan1d(&plan,NX,CUFFT_C2R,BATCH);
	cufftExecC2R(plan,d_data,(cufftReal*)d_data);
	CUDA_SAFE_CALL(cudaMemcpy(h_idata,d_data,sizeof(float)*NX*BATCH,cudaMemcpyDeviceToHost));
printf("...............................................................................................\n");
	for(int i=0;i<NX*BATCH;i++)
	{
		//  warn("h_odata[%d].r=%f,h_odata[%d].i=%f",i,h_odata[i].r,i,h_odata[i].i);
		warn("h_idata[%d].r=%f\n",i,h_idata[i]/NX);
	}

	free(h_idata);
								 
/*	
	//need to divide the result by nx
	int nx = 20;
	Cufft::FFT1D *fft = new Cufft::FFT1D(nx, CUFFT_C2C);
	cufftComplex *data;
	size_t len = nx * sizeof(cufftComplex);
	CudaMalloc((void**) &data, len);
	cufftComplex *h_data = (cufftComplex*) Malloc(nx * sizeof(cufftComplex));
	for (int i = 0; i < nx; i++) {
		h_data[i].x = i;
		h_data[i].y = i;
	}
	CudaMemcpy(data, h_data,len, cudaMemcpyHostToDevice);

	fft->c2c(data, data, CUFFT_FORWARD);
	CudaMemcpy(h_data, data, len, cudaMemcpyDeviceToHost);
	for (int i = 0; i < nx; i++) {
		printf("%d, %0.2f, %0.2f\n", i, h_data[i].x, h_data[i].y);
		h_data[i].x /= nx;
		h_data[i].y /= nx;
	}
	CudaMemcpy(data, h_data, len, cudaMemcpHostToDevice);
	
	fft->c2c(data, data, CUFFT_INVERSE);
	CudaMemcpy(h_data, data, len, cudaMemcpyDeviceToHost);
	for (int i = 0; i < nx; i++) {
		printf("%d, %0.2f, %0.2f\n", i, h_data[i].x, h_data[i].y);
	}
	
	delete fft;
*/
}
int main() {
	runtest1d();
	return 0;
}
