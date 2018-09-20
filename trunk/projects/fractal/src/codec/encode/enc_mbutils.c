#include "enc_mbutils.h"



void taa_h264_enc_store_mbinfo( \
	const mbinfo_t *  currMB,
	const int        mbxmax,
	const int        mbx,
	const int        mby,
	const mbtype_t   mbtype,
	const int        flags,
	const uint16_t   luma_cbp,
	const int        cbp,
	const int        qp_in,
	const uint8_t    deblock_disable,
	const uint8_t    filter_offset_a,
	const uint8_t    filter_offset_b,
	const	imode8_t best_i8x8_mode_chroma,
	const mv_t *     motion_vectors,
#ifdef TAA_SAVE_264_MEINFO
	const unsigned   num_bits_current_mb,
#endif
	mbinfo_store_t * mb)
{
	mb->skip = (uint8_t)(mbtype == P_SKIP);
	mb->mbtype = mbtype;
	mb->best_i8x8_mode_chroma = best_i8x8_mode_chroma;
	mb->mbx = currMB->mbx;
	mb->mby = currMB->mby;
	mb->qp = currMB->mquant;
	mb->prev_qp = currMB->prev_qp;
	mb->mbpos = currMB->mbpos;
	mb->cbp_bits[0] = currMB->cbp_bits[0];
	mb->cbp_bits[1] = currMB->cbp_bits[1];
	mb->cbp_bits[2] = currMB->cbp_bits[2];
	const bool left_avail = MB_LEFT(flags);
	const bool top_avail = MB_UP(flags);
	if (left_avail && top_avail)
	{
		const mbinfo_store_t * mb_left = mb - 1;
		const mbinfo_store_t * mb_top = mb - mbxmax;
		if (mb->skip & mb_left->skip & mb_top->skip)
		{
			const mv_t * mv_curr = motion_vectors;
			const mv_t * mv_left = motion_vectors - NUM_4x4_BLOCKS_Y;
			const mv_t * mv_up = motion_vectors - NUM_4x4_BLOCKS_Y * mbxmax;
			if ((abs(mv_curr->x - mv_left->x) < 4) & (abs(mv_curr->y - mv_left->y) < 4)
				&& (abs(mv_curr->x - mv_up->x) < 4) & (abs(mv_curr->y - mv_up->y) < 4))
			{
				mb->skip = 0x3;
				mb->flags = flags;
				mb->cbp_blk = 0;
				mb->qp = (uint8_t)qp_in;
				return;
			}
		}
	}

	uint16_t cbp_blk = luma_cbp;
	int qp = qp_in, y ,x;
	int new_flags = flags;

	if (MB_TYPE_IS_INTRA(mbtype))
		new_flags |= MB_TYPE_INTRA_;

	if (mbtype == P_16x16)
	{
		for(y = 0 ; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				mb->mvd[y][x][0] = currMB->mv1.x - currMB->pred1.x;
				mb->mvd[y][x][0] = currMB->mv1.y - currMB->pred1.y;
			}
	}
	else if (mbtype == P_16x8)
	{

		for (y = 0; y < 4; y++)
			for (x = 0; x < 2; x++)
			{
				mb->mvd[y][x][0] = currMB->mv1.x - currMB->pred1.x;
				mb->mvd[y][x][0] = currMB->mv1.y - currMB->pred1.y;
			}

		for (y = 0; y < 4; y++)
			for (x = 2; x < 4; x++)
			{
				mb->mvd[y][x][0] = currMB->mv2.x - currMB->pred2.x;
				mb->mvd[y][x][0] = currMB->mv2.y - currMB->pred2.y;
			}

	}
	else if (mbtype == P_8x16)
	{
		for (y = 0; y < 2; y++)
			for (x = 0; x < 4; x++)
			{
				mb->mvd[y][x][0] = currMB->mv1.x - currMB->pred1.x;
				mb->mvd[y][x][0] = currMB->mv1.y - currMB->pred1.y;
			}

		for (y = 2; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				mb->mvd[y][x][0] = currMB->mv2.x - currMB->pred2.x;
				mb->mvd[y][x][0] = currMB->mv2.y - currMB->pred2.y;
			}

	}
	else
	{
		memset(&mb->mvd[0][0][0], 0, sizeof(mb->mvd));
	}

	/* HACK: TODO: Do proper initialization of ycbp for skip mbs. */
	if (mbtype == P_SKIP)
		cbp_blk = 0;

	if (deblock_disable == 0)
	{
		/* Only disable out picture boundaries */
		if (mbx > 0)
			new_flags |= MB_LEFT_;
		if (mby > 0)
			new_flags |= MB_UP_;
	}
	else if (deblock_disable == 1)
	{
		/* Disable for whole slice */
		/* Reset the availability flags, not the rest */
		new_flags &= ~MB_ALL_AVAIL;
		qp = 0;
		cbp_blk = 0;
	}
	else
	{
		/* deblock_disable == 2 */
		/* Disable at slice boundaries */
	}

	mb->flags = new_flags;
	mb->qp = (uint8_t)qp;
	mb->cbp_blk = (uint16_t)cbp_blk;

	/* mb->deblock_disable = deblock_disable; */
	mb->filter_offset_a = filter_offset_a;
	mb->filter_offset_b = filter_offset_b;


	//
#ifdef TAA_SAVE_264_MEINFO
	mb->consumed_bits_num = num_bits_current_mb;
#endif
}