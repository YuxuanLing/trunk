#include "gtest/gtest.h"
#include "struc_test.h"
#include "cinclude.h"


TEST(test_struct_unit, struct_bits_test)
{
	stru_bits test_bits;
	SIGN8 b8 = -1;
	REAL32 zero = 0.0;
	REAL32 real32_min = 0.0000000000000001;
	UNSG32 real_to_uint = real32_min;   // *((UNSG32 *)(&real32_min));
	UNSG32 real_mem_to_uint = *((UNSG32 *)(&real32_min));
	
	test_bits.b9 = -2;
	b8 = (SIGN8)test_bits.b9;

	EXPECT_EQ(b8, -2);
}



