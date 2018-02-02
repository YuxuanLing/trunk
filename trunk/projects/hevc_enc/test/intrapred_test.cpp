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
#include "intrapred.h"


TEST(IntraPredictionTest, intraPredAllTest) {
	Pixel *srcPix = (Pixel *)malloc(4096);
	Pixel *dst = (Pixel *)malloc(4096);
	INT32 dstStride = 0;
	INT32 dirMode = 1, bFilter = 1, width = 4;

	//adding intra pred test code here
	//intra_pred_dc_c<4>(dst,  dstStride, srcPix, dirMode, bFilter);

}