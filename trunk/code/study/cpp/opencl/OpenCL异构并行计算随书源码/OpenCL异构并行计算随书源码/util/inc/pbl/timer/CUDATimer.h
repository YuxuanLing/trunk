/**
 * @file
 * @author yyfn
 *
 * @brief CUDA timer using cuda event
 *
 **/

#pragma once

#ifdef __CUDACC__

#include "../CUDA/CUDAFunction.h"
#include <stdio.h>
#include <stdlib.h>

class CUDATimer{
    cudaEvent_t s;
    cudaEvent_t e;
    float elapsedTime;
		
    public:
    CUDATimer(){ 
        CudaEventCreate(&s); 
        CudaEventCreate(&e);
	}
	
	void start(){
        CudaEventRecord(s,0);
    }

    ~CUDATimer(){
        CudaEventDestroy(s);
        CudaEventDestroy(e);
    }

    void stop(){ 
        CudaEventRecord(e, 0);
        CudaEventSynchronize(e);
        CudaEventElapsedTime(&elapsedTime, s, e);
    }
	
    float getElapsedSeconds(){ 
        return elapsedTime/1000.0f;
    }
	
	float getElapsedMilliSeconds(){
		return elapsedTime;
	}
	float getElapsedMicroSeconds(){
		return elapsedTime*1000.0f;
	}
	void printElapsedSeconds(const char* s = NULL){
	    if(NULL == s){
            printf("use time = %f s\n", getElapsedSeconds());
	    }else{
            printf("%s, use time = %f s\n", s, getElapsedSeconds());
        }
    }
	
	void printElapsedMilliSeconds(const char* s = NULL){
	    if(NULL == s){
            printf("use time = %f ms\n", getElapsedMilliSeconds());
	    }else{
            printf("%s, use time = %f ms\n", s, getElapsedMilliSeconds());
        }
	}
	void printElapsedMicroSeconds(const char* s = NULL){
	    if(NULL == s){
		    printf("%s, use time = %f us\n", s, getElapsedMicroSeconds());
	    }else{
            printf("use time = %f us\n", getElapsedMicroSeconds());
        }
	}
};
#endif
