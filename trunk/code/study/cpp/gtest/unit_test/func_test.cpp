#include "gtest/gtest.h"
#include <stdlib.h>

int add(int a, int b)
{
	return a + b;
}


TEST(func_test_suit, func_add)
{
	EXPECT_EQ(14, add(4, 10));
}



#define  NATIVE_ALIGN  (64)
/****************************************************************************
* x264_malloc:
****************************************************************************/
void *x264_malloc(int i_size)
{
	uint8_t *align_buf = NULL;

	uint8_t *buf = (uint8_t *)malloc(i_size + (NATIVE_ALIGN - 1) + sizeof(void **));
	if (buf)
	{
		align_buf = buf + (NATIVE_ALIGN - 1) + sizeof(void **);
		align_buf -= (intptr_t)align_buf & (NATIVE_ALIGN - 1);
		*((void **)(align_buf - sizeof(void **))) = buf;
	}

	return align_buf;
}

/****************************************************************************
* x264_free:
****************************************************************************/
void x264_free(void *p)
{
	if (p)
	{
		free(*(((void **)p) - 1));
	}
}



TEST(x264_mem_test, x264_malloc)
{
	void *align_64_buf = x264_malloc(1024);
	x264_free(align_64_buf);

}