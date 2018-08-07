#ifndef H_CUTIL_MALLOC
#define H_CUTIL_MALLOC


/**
 * @file
 * @author yyfn
 * @date 20120713
 * @brief memroy alloca function
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include "util.h"

/**
 * @brief wrap malloc, so use Malloc don't need to check malloc failed
 * @param len memory size in byte
 *
 * @return memory pointer
 */

inline void* Malloc(size_t len) {
	void* ret = malloc(len);
	if (NULL == ret) {
		printError("malloc failed, NULL");
	}
	return ret;
}


#endif
