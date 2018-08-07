/*-------------------------------------------------------------------------
Copyright 2016 
---------------------------------------------------------------------------
Description:    opecl get device info, nothing more.
Author:         yxling
---------------------------------------------------------------------------*/


#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

const char* GetDeviceType(cl_device_type it);

void displayDeviceInfo(cl_platform_id id, cl_device_type dev_type);
void displayPlatformInfo(cl_platform_id id,
	cl_platform_info param_name,
	const char* paramNameAsStr);


int getPlatFormDetialDeviceInfo();

int getPlatFormSimpleDeviceInfo();