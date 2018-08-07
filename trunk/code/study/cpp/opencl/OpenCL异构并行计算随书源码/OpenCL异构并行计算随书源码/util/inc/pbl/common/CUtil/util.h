#ifndef H_CUtil_UTIL
#define H_CUtil_UTIL
/**
 * @file
 * @author yyfn
 * @date 20120713
 *
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CUDAC
#ifdef __CUDACC__
#define CUDAC __device__ __host__
#else
#define CUDAC
#endif
#endif

#if (_MSC_VER >= 1500)
#define restrict __restrict
#elif defined(__CUDACC__) || defined(__GNUC__) || defined(__GNUG__)
#define restrict __restrict__
#else
#define restrict
#endif

#define printError(str) printf("%s, %d, %s\n", __FILE__, __LINE__, str)

int isAllZeroOrSpace(const char *str) {
	for (int i = 0; str[i] != 0; i++) {
		if (isspace(str[i]) || ('0' == str[i]) || ('.' == str[i])) {
			continue;
		} else {
			return 0;
		}
	}

	return 1;
}
/**
 * @brief wrap for atoi
 */
int Atoi(const char *str) {
	int ret = atoi(str);
	if (0 != ret) {
		return ret;
	} else if (isAllZeroOrSpace(str)) {
		return 0;
	} else {
		printf("%s has no digit character\n", str);
		exit(0);
	}
}

/**
 * @brief wrap for atof
 *
 */
double Atof(const char *str) {
	double ret = atof(str);

	if (ret != 0.0) {
		return ret;
	} else if (isAllZeroOrSpace(str)) {
		return 0.0;
	} else {
		printf("%s has no digit character\n", str);
		exit(0);
	}
}

#define System(command) {\
	if(-1 == system(command)){\
		printError("system invoke error, is not about command\n");\
		exit(0);\
	}\
}

#endif
