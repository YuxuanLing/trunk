#include "gtest/gtest.h"

#include <limits.h>  // For INT_MAX.
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <map>
#include <vector>
#include <ostream>

#include "gtest/gtest-spi.h"
#include "commonDef.h"
#include "bitstream.h"


TEST(ComOutputBitStreamTest, writeTest) {
	UINT8 result[9] = {0xd1,0xb,0x1e,0x69,0xf1,0x98,0x70,0x3b,0x7f};
	ComOutputBitStream bs;
	INT32 i = 0;

	bs.write(6, 3);
	bs.write(8, 4);
	bs.write(16, 5);
	bs.write(11, 4);
	bs.write(0, 3);
	bs.write(7, 3);
	bs.write(4, 3);
	bs.write(26, 5);
	bs.write(31, 6);
	bs.write(12, 7);
	bs.write(12, 4);
	bs.write(0, 1);
	bs.write(7, 4);
	bs.writeAlignZero();
	bs.write(0x3b, 8);
	bs.write(0, 1);
	bs.writeAlignOne();

	const std::vector<UINT8>& fifo = bs.getFIFO();
	for (std::vector<UINT8>::const_iterator it = fifo.begin(); it != fifo.end();)
	{
		EXPECT_EQ(*it, result[i]);
		it++;
		i++;
	}

}