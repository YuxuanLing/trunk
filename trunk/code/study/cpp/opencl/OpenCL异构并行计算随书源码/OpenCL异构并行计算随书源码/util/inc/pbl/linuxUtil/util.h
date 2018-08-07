#ifndef H_LINUXUTIL_UTIL
#define H_LINUXUTIL_UTIL
/**
 * @file
 **/

#pragma once
#include <time.h>
#include <sys/time.h>

long getMicroSecond(){
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}
#endif
