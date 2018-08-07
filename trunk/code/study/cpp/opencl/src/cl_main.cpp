#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "cl/cl.h"
#include "opencl_get_info.h"
#include "build_opencl_program.h"
#include "kernel_queue.h"
using namespace std;


int main()
{
	int ret = 0;
	cl_uint error;
	const char *file_names[] = { "..\\..\\src\\hello_world.cl" };
	//ret = getPlatFormDetialDeviceInfo();
	//ret = getPlatFormSimpleDeviceInfo();

	//buildOpenclPrograms();
	error = kernel_queue_hello_sample(file_names, 1);
	
	system("pause");
	return 0;
}