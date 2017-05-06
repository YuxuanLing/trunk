#include "CL/cl.h"
#include "build_opencl_program.h"
#include "kernel_queue.h"


cl_int kernel_queue_hello_sample(const char *file_names[], int NUMBER_OF_FILES)
{
	  /* OpenCL 1.1 data structures */
   cl_platform_id* platforms;
   cl_program program;
   cl_device_id device;
   cl_context context;

   /* OpenCL 1.1 scalar data types */
   cl_uint numOfPlatforms;
   cl_int  error = CL_SUCCESS;
   if (NUMBER_OF_FILES <= 0) return CL_FALSE;
   /* 
      Get the number of platforms 
      Remember that for each vendor's SDK installed on the computer,
      the number of available platform also increased. 
    */
   error = clGetPlatformIDs(0, NULL, &numOfPlatforms);
   if(error != CL_SUCCESS ) {			
      perror("Unable to find any OpenCL platforms");
	  return error;
   }

   platforms = (cl_platform_id*) alloca(sizeof(cl_platform_id) * numOfPlatforms);
   printf("Number of OpenCL platforms found: %d\n", numOfPlatforms);

   error = clGetPlatformIDs(numOfPlatforms, platforms, NULL);
   if(error != CL_SUCCESS ) {			
      perror("Unable to find any OpenCL platforms");
	  return error;
   }
   // Search for a CPU/GPU device through the installed platforms
   // Build a OpenCL program and do not run it.
   char** buffer = new char*[NUMBER_OF_FILES];
   size_t *sizes = new size_t[NUMBER_OF_FILES];
   for(cl_uint i = 0; i < numOfPlatforms; i++ ) {
       // Get the GPU device
       error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
       if(error != CL_SUCCESS) {
          // Otherwise, get the CPU
          error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_CPU, 1, &device, NULL);
       }
        if(error != CL_SUCCESS) {
            perror("Can't locate any OpenCL compliant device");
            return error;
        }
        /* Create a context */
        context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
        if(error != CL_SUCCESS) {
            perror("Can't create a valid OpenCL context");
            return error;
        }

        /* Load the two source files into temporary datastores */
        loadProgramSource(file_names, NUMBER_OF_FILES, buffer, sizes);

        /* Create the OpenCL program object */
        program = clCreateProgramWithSource(context, NUMBER_OF_FILES, (const char**)buffer, sizes, &error);				
	    if(error != CL_SUCCESS) {
	      perror("Can't create the OpenCL program object");
		  return error;
	    }
        /* Build OpenCL program object and dump the error message, if any */
        char *program_log;
        size_t log_size;
        error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);		
	    if(error != CL_SUCCESS) {
	      // If there's an error whilst building the program, dump the log
	      clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	      program_log = (char*) malloc(log_size+1);
	      program_log[log_size] = '\0';
	      clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 
	            log_size+1, program_log, NULL);
	      printf("\n=== ERROR ===\n\n%s\n=============\n", program_log);
	      free(program_log);
		  return error;
	    }
  
        /* Query the program as to how many kernels were detected */
        cl_uint numOfKernels;
        error = clCreateKernelsInProgram(program, 0, NULL, &numOfKernels);
        if (error != CL_SUCCESS) {
            perror("Unable to retrieve kernel count from program");
            exit(1);
        }
        cl_kernel* kernels = (cl_kernel*) alloca(sizeof(cl_kernel) * numOfKernels);
        error = clCreateKernelsInProgram(program, numOfKernels, kernels, NULL);
        for(cl_uint i = 0; i < numOfKernels; i++) {
            char kernelName[32];
            cl_uint argCnt;
            clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, sizeof(kernelName), kernelName, NULL);
            clGetKernelInfo(kernels[i], CL_KERNEL_NUM_ARGS, sizeof(argCnt), &argCnt, NULL);
            printf("Kernel name: %s with arity: %d\n", kernelName, argCnt);
            printf("About to create command queue and enqueue this kernel...\n");

            /* Create a command queue */
            cl_command_queue cQ = clCreateCommandQueue(context, device, 0, &error);
            if (error != CL_SUCCESS) { 
                perror("Unable to create command-queue");
				return error;
            }
            /* Create a OpenCL buffer object */
            cl_mem strObj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(char) * 11, "dummy value", NULL);

            /* Let OpenCL know that the kernel is suppose to receive an argument */
            error = clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &strObj);
            if (error != CL_SUCCESS) { 
                perror("Unable to create buffer object");
				return error;
            }

            /* Enqueue the kernel to the command queue */
            error = clEnqueueTask(cQ, kernels[i], 0, NULL, NULL);
            if (error != CL_SUCCESS) { 
                perror("Unable to enqueue task to command-queue");
				return error;
            }
            printf("Task has been enqueued successfully!\n");

            /* Release the command queue */
            clReleaseCommandQueue(cQ);
        }

        /* Clean up */
        
        for(cl_uint ii = 0; ii < numOfKernels; ii++) { clReleaseKernel(kernels[ii]); }
        for(cl_uint ii =0; ii< NUMBER_OF_FILES; ii++) { free(buffer[ii]); }
        clReleaseProgram(program);
        clReleaseContext(context);
   }
   delete buffer;
   delete sizes;

   return error;
}