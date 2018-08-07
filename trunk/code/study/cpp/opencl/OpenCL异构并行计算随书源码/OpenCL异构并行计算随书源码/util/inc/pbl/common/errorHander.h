#ifdef __CUDACC__
#pragma once
#include <stdio.h>

    class ErrorHander{//file:file name;line:lineNo
	    public :

	    //can't use it at asynchronize situation
		    void static printfError(cudaError_t cet){
			    if(cudaSuccess != cet){
				    printf("error:%s\n",cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }
	    //can use it at anywhere
		    void static printfLastError(){
		        cudaThreadSynchronize();
			    cudaError_t cet = cudaGetLastError();
			    if(cudaSuccess != cet){
				    printf("error:%s!\n",cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }
		
		    void static printfError(int line,cudaError_t cet){
			    if(cudaSuccess !=cet){
				    printf("line:%d,error:%s\n",line,cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }
		
		    void static printfLastError(int line){
			    cudaThreadSynchronize();
		        cudaError_t cet = cudaGetLastError();
			    if(cudaSuccess != cet){
				    printf("line:%d,error:%s!\n",line,cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }
		
		    void static printfError(char *file,int line,cudaError_t cet){
			    if(cudaSuccess != cet){
				    printf("%s,line:%d,error:%s\n",file,line,cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }
		
		    void static printfLastError(char *file,int line){
		        cudaThreadSynchronize();
			    cudaError_t cet = cudaGetLastError();
			    if(cudaSuccess != cet){
				    printf("%s,line:%d,error:%s!\n",file,line,cudaGetErrorString(cet));
				    fflush(stdout);
				    exit(1);
			    }
		    }

    };
#endif
