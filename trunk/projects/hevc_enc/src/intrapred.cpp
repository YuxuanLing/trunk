#include "intrapred.h"

static void dc_pred_filter(const Pixel* above, const Pixel* left, Pixel* dst, INT32 dststride, int size)
{
	// boundary pixels processing
	dst[0] = (Pixel)((above[0] + left[0] + 2 * dst[0] + 2) >> 2);

	for (int x = 1; x < size; x++)
		dst[x] = (Pixel)((above[x] + 3 * dst[x] + 2) >> 2);

	dst += dststride;
	for (int y = 1; y < size; y++)
	{
		*dst = (Pixel)((left[y] + 3 * (*dst) + 2) >> 2);
		dst += dststride;
	}
}



template<int width>
void intra_pred_dc_c(Pixel* dst, INT32 dstStride, const Pixel* srcPix, int /*dirMode*/, int bFilter)
{
	int k, l;

	int dcVal = width;
	for (int i = 0; i < width; i++)
		dcVal += srcPix[1 + i] + srcPix[2 * width + 1 + i];

	dcVal = dcVal / (width + width);
	for (k = 0; k < width; k++)
		for (l = 0; l < width; l++)
			dst[k * dstStride + l] = (Pixel)dcVal;

	if (bFilter)
		dc_pred_filter(srcPix + 1, srcPix + (2 * width + 1), dst, dstStride, width);
}



template<int log2Size>
void intra_planar_pred_c(Pixel* dst, INT32 dstStride, const Pixel* srcPix, int /*dirMode*/, int /*bFilter*/)
{
	const int blkSize = 1 << log2Size;

	const Pixel* above = srcPix + 1;
	const Pixel* left = srcPix + (2 * blkSize + 1);

	Pixel topRight = above[blkSize];
	Pixel bottomLeft = left[blkSize];
	for (int y = 0; y < blkSize; y++)
		for (int x = 0; x < blkSize; x++)
			dst[y * dstStride + x] = (Pixel)(((blkSize - 1 - x) * left[y] + (blkSize - 1 - y) * above[x] + (x + 1) * topRight + (y + 1) * bottomLeft + blkSize) >> (log2Size + 1));
}

template<int width>
void intra_pred_ang_c(Pixel* dst, INT32 dstStride, const Pixel *srcPix0, int dirMode, int bFilter)
{
	int width2 = width << 1;
	// Flip the neighbours in the horizontal case.
	int horMode = dirMode < 18;
	Pixel neighbourBuf[129];
	const Pixel *srcPix = srcPix0;

	if (horMode)
	{
		neighbourBuf[0] = srcPix[0];
		for (int i = 0; i < width << 1; i++)
		{
			neighbourBuf[1 + i] = srcPix[width2 + 1 + i];
			neighbourBuf[width2 + 1 + i] = srcPix[1 + i];
		}
		srcPix = neighbourBuf;
	}

	// Intra prediction angle and inverse angle tables.
	const int8_t angleTable[17] = { -32, -26, -21, -17, -13, -9, -5, -2, 0, 2, 5, 9, 13, 17, 21, 26, 32 };
	const int16_t invAngleTable[8] = { 4096, 1638, 910, 630, 482, 390, 315, 256 };

	// Get the prediction angle.
	int angleOffset = horMode ? 10 - dirMode : dirMode - 26;
	int angle = angleTable[8 + angleOffset];

	// Vertical Prediction.
	if (!angle)
	{
		for (int y = 0; y < width; y++)
			for (int x = 0; x < width; x++)
				dst[y * dstStride + x] = srcPix[1 + x];

		if (bFilter)
		{
			int topLeft = srcPix[0], top = srcPix[1];
			for (int y = 0; y < width; y++)
				dst[y * dstStride] = x265_clip((int16_t)(top + ((srcPix[width2 + 1 + y] - topLeft) >> 1)));
		}
	}
	else // Angular prediction.
	{
		// Get the reference Pixels. The reference base is the first Pixel to the top (neighbourBuf[1]).
		Pixel refBuf[64];
		const Pixel *ref;

		// Use the projected left neighbours and the top neighbours.
		if (angle < 0)
		{
			// Number of neighbours projected. 
			int nbProjected = -((width * angle) >> 5) - 1;
			Pixel *ref_pix = refBuf + nbProjected + 1;

			// Project the neighbours.
			int invAngle = invAngleTable[-angleOffset - 1];
			int invAngleSum = 128;
			for (int i = 0; i < nbProjected; i++)
			{
				invAngleSum += invAngle;
				ref_pix[-2 - i] = srcPix[width2 + (invAngleSum >> 8)];
			}

			// Copy the top-left and top Pixels.
			for (int i = 0; i < width + 1; i++)
				ref_pix[-1 + i] = srcPix[i];
			ref = ref_pix;
		}
		else // Use the top and top-right neighbours.
			ref = srcPix + 1;

		// Pass every row.
		int angleSum = 0;
		for (int y = 0; y < width; y++)
		{
			angleSum += angle;
			int offset = angleSum >> 5;
			int fraction = angleSum & 31;

			if (fraction) // Interpolate
				for (int x = 0; x < width; x++)
					dst[y * dstStride + x] = (Pixel)(((32 - fraction) * ref[offset + x] + fraction * ref[offset + x + 1] + 16) >> 5);
			else // Copy.
				for (int x = 0; x < width; x++)
					dst[y * dstStride + x] = ref[offset + x];
		}
	}

	// Flip for horizontal.
	if (horMode)
	{
		for (int y = 0; y < width - 1; y++)
		{
			for (int x = y + 1; x < width; x++)
			{
				Pixel tmp = dst[y * dstStride + x];
				dst[y * dstStride + x] = dst[x * dstStride + y];
				dst[x * dstStride + y] = tmp;
			}
		}
	}
}




template<int log2Size>
void all_angs_pred_c(Pixel *dest, Pixel *refPix, Pixel *filtPix, int bLuma)
{
	const int size = 1 << log2Size;
	for (int mode = 2; mode <= 34; mode++)
	{
		Pixel *srcPix = (g_intraFilterFlags[mode] & size ? filtPix : refPix);
		Pixel *out = dest + ((mode - 2) << (log2Size * 2));

		intra_pred_ang_c<size>(out, size, srcPix, mode, bLuma);

		// Optimize code don't flip buffer
		bool modeHor = (mode < 18);

		// transpose the block if this is a horizontal mode
		if (modeHor)
		{
			for (int k = 0; k < size - 1; k++)
			{
				for (int l = k + 1; l < size; l++)
				{
					Pixel tmp = out[k * size + l];
					out[k * size + l] = out[l * size + k];
					out[l * size + k] = tmp;
				}
			}
		}
	}
}


