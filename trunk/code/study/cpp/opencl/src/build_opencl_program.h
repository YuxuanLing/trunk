#pragma once
#include "cinclude.h"


void loadProgramSource(const char** files,
					   size_t length,
					   char** buffer,
					   size_t* sizes);
void buildOpenclPrograms();
