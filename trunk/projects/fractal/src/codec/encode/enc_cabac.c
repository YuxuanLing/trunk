/*****************************************************************************
* enc_cabac.c: h264 arithmetic coder
*****************************************************************************
* Copyright (C) Cisco 2018
*
* Authors: Yuxuan Ling<yuxling@cisco.com>
*
*****************************************************************************/

#include <limits.h>
#include <memory.h>
#include "com_mbutils.h"
#include "enc_cabac_engine.h"
#include "enc_cabac.h"
#include "encoder.h"
#include "enc.h"
#include "enc_cabac_contex_ini.h"

typedef enum
{
	IS_LUMA = 0,
	IS_CHROMA = 1
} Component_Type;

//! Pixel position for checking neighbors
typedef struct pix_pos
{
	int   available;
	int   mb_addr;
	int   x;      //pos in mb
	int   y;      
	int   pos_x;  //pos in pic
	int   pos_y;
} PixelPos;

static const unsigned char subblk_offset_x[4] =
{
    0, 4, 0, 4
};

static const unsigned char subblk_offset_y[4] =
{
	0, 0, 4, 4
};


static const uint8_t maxpos[] = { 15, 14, 63, 31, 31, 15,  3, 14,  7, 15, 15, 14, 63, 31, 31, 15, 15, 14, 63, 31, 31, 15 };
static const uint8_t c1isdc[] = { 1,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1 };

static const uint8_t type2ctx_bcbp[] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20 };
static const uint8_t type2ctx_map[] = { 0,  1,  2,  3,  4,  5,  6,  7,  6,  6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 8
static const uint8_t type2ctx_last[] = { 0,  1,  2,  3,  4,  5,  6,  7,  6,  6, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 8
static const uint8_t type2ctx_one[] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20 }; // 7
static const uint8_t type2ctx_abs[] = { 0,  1,  2,  3,  3,  4,  5,  6,  5,  5, 10, 11, 12, 13, 13, 14, 16, 17, 18, 19, 19, 20 }; // 7
static const uint8_t max_c2[] = { 4,  4,  4,  4,  4,  4,  3,  4,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 }; // 9

//===== position -> ctx for MAP =																														   //--- zig-zag scan ----
static const uint8_t  pos2ctx_map8x8[] = {
	0,  1,  2,  3,  4,  5,  5,  4,  4,  3,  3,  4,  4,  4,  5,  5,
	4,  4,  4,  4,  3,  3,  6,  7,  7,  7,  8,  9, 10,  9,  8,  7,
	7,  6, 11, 12, 13, 11,  6,  7,  8,  9, 14, 10,  9,  8,  6, 11,
	12, 13, 11,  6,  9, 14, 10,  9, 11, 12, 13, 11 ,14, 10, 12, 14
}; // 15 CTX

static const uint8_t  pos2ctx_map8x4[] = {
	0,  1,  2,  3,  4,  5,  7,  8,  9, 10, 11,  9,  8,  6,  7,  8,
	9, 10, 11,  9,  8,  6, 12,  8,  9, 10, 11,  9, 13, 13, 14, 14
}; // 15 CTX

static const uint8_t  pos2ctx_map4x4[] = {
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14
}; // 15 CTX

static const uint8_t  pos2ctx_map2x4c[] = {
	0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

static const uint8_t  pos2ctx_map4x4c[] = {
	0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

const uint8_t* pos2ctx_map[] = {
	pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8, pos2ctx_map8x4,
	pos2ctx_map8x4, pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
	pos2ctx_map2x4c, pos2ctx_map4x4c,
	pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
	pos2ctx_map8x4, pos2ctx_map4x4,  //Cb component
	pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
	pos2ctx_map8x4,pos2ctx_map4x4  //Cr component
};




//===== position -> ctx for LAST =====
static const uint8_t  pos2ctx_last8x8[] = {
	0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,
	5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8
}; //  9 CTX

static const uint8_t  pos2ctx_last8x4[] = {
	0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
	3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8
}; //  9 CTX

static const uint8_t  pos2ctx_last4x4[] = {
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
}; // 15 CTX

static const uint8_t  pos2ctx_last2x4c[] = {
	0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

static const uint8_t  pos2ctx_last4x4c[] = {
	0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

const uint8_t* pos2ctx_last[] = {
	pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8, pos2ctx_last8x4,
	pos2ctx_last8x4, pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last4x4,
	pos2ctx_last2x4c, pos2ctx_last4x4c,
	pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
	pos2ctx_last8x4, pos2ctx_last4x4,  //Cb component
	pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
	pos2ctx_last8x4, pos2ctx_last4x4 //Cr component
};



static const short block_size[8][2] =
{
	{ 16, 16 },
	{ 16, 8 },
	{ 8, 16 },
	{ 8, 8 },
	{ 8, 4 },
	{ 4, 8 },
	{ 4, 4 }
};



static inline int get_bit(int64_t x, int n)
{
	return (int)(((x >> n) & 1));
}


static inline int iabs(int x)
{
	static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
	int y = x >> INT_BITS;
	return (x ^ y) - y;
}

static inline int imin(int a, int b)
{
	return ((a) < (b)) ? (a) : (b);
}


/*!
************************************************************************
*    Returns the number of currently written bits
************************************************************************
*/
static inline int algorithm_enc_bits_written(EncodingEnvironmentPtr eep)
{
	return (((*eep->Ecodestrm_len) + eep->Epbuf + 1) << 3) + (eep->Echunks_outstanding * BITS_TO_LOAD) + BITS_TO_LOAD - eep->Ebits_to_go;
}


/*!
***************************************************************************
*    arithmetically encode the macroblock's
*    type info of a given MB in an I Slice.
***************************************************************************
*/
void write_mb_I_type_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);
	int a = 0, b = 0;
	int act_ctx = 0;
	int act_sym;
	int mode_sym = 0;
	int mbx = currMB->mbx, mby = currMB->mby, mbxmax = frameinfo->width/16;
	mbinfo_store_t *mbA = currMB->mb_left, *mbB = currMB->mb_up;
	motionInfoContext_t *ctx = &(currSlice->mot_ctx);

	if (mbB == NULL)
		b = 0;
	else
	{
		b = (mbB->mbtype != I_4x4) ? 1 : 0;
	}

	if (mbA == NULL)
		a = 0;
	else
	{
		a = (mbA->mbtype != I_4x4) ? 1 : 0;
	}

	
	act_ctx = a + b;
	act_sym = se->value1;
	se->context = act_ctx; // store context

	if (act_sym == 0) // 4x4 Intra
	{
		binary_encode_symbol(eep, 0, ctx->mb_type_contexts[0] + act_ctx);
	}
	else if (act_sym == 25) // PCM-MODE,todo: not support now
	{
		assert(0);
		binary_encode_symbol(eep, 1, ctx->mb_type_contexts[0] + act_ctx);
		binary_encode_symbol_final(eep, 1);
	}
	else // 16x16 Intra
	{
		binary_encode_symbol(eep, 1, ctx->mb_type_contexts[0] + act_ctx);

		binary_encode_symbol_final(eep, 0);

		mode_sym = act_sym - 1; // Values in the range of 0...23
		act_ctx = 4;
		act_sym = mode_sym / 12;
		binary_encode_symbol(eep, act_sym, ctx->mb_type_contexts[0] + act_ctx); // coding of luma AC/no AC
		mode_sym = mode_sym % 12;
		act_sym = mode_sym / 4; // coding of cbp: 0,1,2
		act_ctx = 5;
		if (act_sym == 0)
		{
			binary_encode_symbol(eep, 0, ctx->mb_type_contexts[0] + act_ctx);
		}
		else
		{
			binary_encode_symbol(eep, 1, ctx->mb_type_contexts[0] + act_ctx);
			act_ctx = 6;
			binary_encode_symbol(eep, (act_sym != 1), ctx->mb_type_contexts[0] + act_ctx);
		}
		mode_sym = mode_sym & 0x03; // coding of I pred-mode: 0,1,2,3
		act_sym = mode_sym >> 1;
		act_ctx = 7;
		binary_encode_symbol(eep, act_sym, ctx->mb_type_contexts[0] + act_ctx);
		act_ctx = 8;
		act_sym = mode_sym & 0x01;
		binary_encode_symbol(eep, act_sym, ctx->mb_type_contexts[0] + act_ctx);
	}

	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}




/*!
***************************************************************************
*    arithmetically encode the macroblock's
*    type info of a given MB in P frame.
***************************************************************************
*/

void write_mb_P_type_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);

	BiContextType *mb_type_contexts = &(currSlice->mot_ctx.mb_type_contexts[1][0]);

	int act_ctx = 0;
	int mode_sym = 0;
	int act_sym = se->value1;
	int mode16x16 = 7;                                              //todo: check the mode is right here

	if (act_sym >= mode16x16)
	{
		mode_sym = act_sym - mode16x16;
		act_sym = mode16x16; // 16x16 mode info
	}

	switch (act_sym)
	{
	case 0:
		break;
	case 1:
		binary_encode_symbol(eep, 0, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 0, &mb_type_contexts[5]);
		binary_encode_symbol(eep, 0, &mb_type_contexts[6]);
		break;
	case 2:
		binary_encode_symbol(eep, 0, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 1, &mb_type_contexts[5]);
		binary_encode_symbol(eep, 1, &mb_type_contexts[7]);
		break;
	case 3:
		binary_encode_symbol(eep, 0, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 1, &mb_type_contexts[5]);
		binary_encode_symbol(eep, 0, &mb_type_contexts[7]);
		break;
	case 4:
	case 5:
		binary_encode_symbol(eep, 0, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 0, &mb_type_contexts[5]);
		binary_encode_symbol(eep, 1, &mb_type_contexts[6]);
		break;
	case 6:
		binary_encode_symbol(eep, 1, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 0, &mb_type_contexts[7]);
		break;
	case 7:
		binary_encode_symbol(eep, 1, &mb_type_contexts[4]);
		binary_encode_symbol(eep, 1, &mb_type_contexts[7]);
		break;
	default:
		printf("Unsupported MB-MODE in writeMB_I_typeInfo_CABAC!\n");
		assert(0);                                             //todo
	}

	if (act_sym == mode16x16) // additional info for 16x16 Intra-mode
	{
		if (mode_sym == 24)
		{
			binary_encode_symbol_final(eep, 1);
			se->len = (algorithm_enc_bits_written(eep) - curr_len);
			return;
		}
		binary_encode_symbol_final(eep, 0);

		act_ctx = 8;
		act_sym = mode_sym / 12;
		binary_encode_symbol(eep, act_sym, mb_type_contexts + act_ctx); // coding of AC/no AC
		mode_sym = mode_sym % 12;

		act_sym = mode_sym >> 2; // coding of cbp: 0,1,2
		act_ctx = 9;
		if (act_sym == 0)
		{
			binary_encode_symbol(eep, 0, mb_type_contexts + act_ctx);
		}
		else
		{
			binary_encode_symbol(eep, 1, mb_type_contexts + act_ctx);
			binary_encode_symbol(eep, (act_sym != 1), mb_type_contexts + act_ctx);
		}

		mode_sym = mode_sym & 0x03; // coding of I pred-mode: 0,1,2,3
		act_ctx = 10;
		act_sym = mode_sym >> 1;
		binary_encode_symbol(eep, act_sym, mb_type_contexts + act_ctx);
		act_sym = (mode_sym & 0x01);
		binary_encode_symbol(eep, act_sym, mb_type_contexts + act_ctx);
	}

	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}




/*!
****************************************************************************
*    arithmetically encode a pair of
*    intra prediction modes of a given MB.
****************************************************************************
*/
void write_intraPredMode_cabac(slice_enc_t *currSlice, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);
	BiContextType *ipr_contexts = &(currSlice->text_ctx.ipr_contexts[0]);

	// use_most_probable_mode
	if (se->value1 == -1)
		binary_encode_symbol(eep, 1, ipr_contexts);
	else
	{
		binary_encode_symbol(eep, 0, ipr_contexts);

		// remaining_mode_selector
		binary_encode_symbol(eep, (se->value1 & 0x1), ipr_contexts + 1);
		binary_encode_symbol(eep, ((se->value1 & 0x2) >> 1), ipr_contexts + 1);
		binary_encode_symbol(eep, ((se->value1 & 0x4) >> 2), ipr_contexts + 1);
	}

	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}


/*!
************************************************************************
*    Writes 4x4 intra prediction modes for a macroblock luma
************************************************************************
*/
int write_intra4x4_modes(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB,  EncodingEnvironmentPtr eep)
{
	int  rate = 0;
	static const int block_to_raster_idx[16] = {
		0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
	};
	SyntaxElement se;
	int imode_offset = currMB->mbpos * NUM_4x4_BLOCKS_Y;
	uint8_t *intra_modes = frameinfo->intra_modes;

	imode4_t * pred_intra_modes = &currMB->pred_intra_modes[0];
	for (int k = 0; k < 16; k++)
	{
		int raster_idx = block_to_raster_idx[k];
		imode4_t pred_mode = pred_intra_modes[raster_idx];
		uint8_t mode = intra_modes[imode_offset + raster_idx];

		se.context = k;

		if (pred_mode == mode)
		{
			se.value1 = -1;
			se.value2 = 0;
		}
		else
		{
			int rem_intra4x4_pred_mode = mode;
			if (mode >= pred_mode)
				rem_intra4x4_pred_mode -= 1;
			se.value1 = rem_intra4x4_pred_mode;
			se.value2 = 0;

		}

		write_intraPredMode_cabac(currSlice, &se, eep);
		rate += se.len;
	}

	return rate;

}



/*!
************************************************************************
*    Writes intra prediction modes for a macroblock luma, only support 16x16 and 4x4
************************************************************************
*/
int write_intra_modes(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{
	switch (currMB->mbtype)
	{
	case I_4x4:
		return write_intra4x4_modes(frameinfo, currSlice, currMB, eep);
		break;
	default:     //16x16
		return 0;
	}
}


/*!
************************************************************************
*    Unary binarization and encoding of a symbol
************************************************************************
*/
void unary_bin_max_encode(EncodingEnvironmentPtr eep_dp, unsigned int symbol, BiContextTypePtr ctx, int ctx_offset, unsigned int max_symbol)
{
	if (symbol == 0)
	{
		binary_encode_symbol(eep_dp, 0, ctx);
		return;
	}
	else
	{
		unsigned int l = symbol;
		binary_encode_symbol(eep_dp, 1, ctx);

		ctx += ctx_offset;
		while ((--l)>0)
			binary_encode_symbol(eep_dp, 1, ctx);
		if (symbol < max_symbol)
			binary_encode_symbol(eep_dp, 0, ctx);
	}
}



/*!
****************************************************************************
*    This function is used to arithmetically encode the chroma
*    intra prediction mode of an 8x8 block
****************************************************************************
*/
void write_chromaIntraPredMode_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);
	BiContextType *ctx_cipr = &(currSlice->text_ctx.cipr_contexts[0]);
	int                 act_sym = se->value1;
	int a, b , mbx = currMB->mbx, mby = currMB->mby, mbxmax = frameinfo->width / 16;
	mbinfo_store_t *mbA = currMB->mb_left, *mbB = currMB->mb_up;

	b = (mbB != NULL) ? (((mbB->best_i8x8_mode_chroma != 0) && (mbB->mbtype != I_PCM)) ? 1 : 0) : 0;
	a = (mbA != NULL) ? (((mbA->best_i8x8_mode_chroma != 0) && (mbA->mbtype != I_PCM)) ? 1 : 0) : 0;

	int act_ctx = a + b;

	if (act_sym == 0)
		binary_encode_symbol(eep, 0, ctx_cipr + act_ctx);
	else
	{
		binary_encode_symbol(eep, 1, ctx_cipr + act_ctx);
		unary_bin_max_encode(eep, (unsigned int)(act_sym - 1), ctx_cipr + 3, 0, 2);
	}

	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}


/*!
************************************************************************
*    Write chroma intra prediction mode of current mb.
************************************************************************
*/
int write_chroma_intra_pred_mode(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{
	SyntaxElement   se;
	int             rate = 0;

	//===== BITS FOR CHROMA INTRA PREDICTION MODES
	se.value1 = currMB->best_i8x8_mode_chroma;
	se.value2 = 0;
	se.type = SE_INTRAPREDMODE;

	write_chromaIntraPredMode_cabac(frameinfo, currSlice, currMB, &se, eep);

	rate += se.len;

	return rate;
}



/*!
****************************************************************************
*    This function is used to get neighbour block pos 
*    xN yN is the pos related to current mb
****************************************************************************
*/
void get4x4Neighbour(frameinfo_t * frameinfo, mbinfo_t *currMB, int xN, int yN, int comp_type, PixelPos *pix)
{
	//BlockPos *PicPos = currMB->p_Vid->PicPos;
	int mb_size[2][2] = { {16,16}, {8, 8}}; // [luma/chroma](x, y)
	if (xN < 0)
	{
		if (yN < 0)
		{			
			pix->available = (currMB->mb_up_left != NULL)?1:0 ;                   //mb_D
			pix->mb_addr = pix->available? currMB->mb_up_left->mbpos:-1;
		}
		else if ((yN >= 0) && (yN < mb_size[comp_type][1]))
		{
			pix->available = (currMB->mb_left != NULL) ? 1 : 0;                   //mb_A
			pix->mb_addr = pix->available ? currMB->mb_left->mbpos:-1;
		}
		else
		{
			pix->available = false;
		}
	}
	else if ((xN >= 0) && (xN < mb_size[comp_type][0]))
	{
		if (yN < 0)
		{
			pix->available = (currMB->mb_up != NULL) ? 1 : 0;                     //mb_B
			pix->mb_addr = pix->available ? currMB->mb_up->mbpos : -1;
		}
		else if (((yN >= 0) && (yN < mb_size[comp_type][1])))                                //current block is in mb_X
		{
			pix->mb_addr = currMB->mbpos;
			pix->available = true;
		}
		else
		{
			pix->available = false;
		}
	}
	else if ((xN >= mb_size[comp_type][0]) && (yN < 0))
	{
		pix->available = (currMB->mb_up_right != NULL) ? 1 : 0;                     //mb_C
		pix->mb_addr = pix->available ? currMB->mb_up_right->mbpos : -1;
	}
	else
	{
		pix->available = false;
	}

	if (pix->available)
	{
		pix->x = (xN & (mb_size[comp_type][0] - 1));
		pix->y = (yN & (mb_size[comp_type][1] - 1));
		pix->pos_x = (short)(pix->x + (frameinfo->mbinfo_stored[pix->mb_addr]).mbx * mb_size[comp_type][0]);
		pix->pos_y = (short)(pix->y + (frameinfo->mbinfo_stored[pix->mb_addr]).mby * mb_size[comp_type][1]);
		pix->x >>= 2;
		pix->y >>= 2;
		pix->pos_x >>= 2;
		pix->pos_y >>= 2;
	}
}



/*!
****************************************************************************
*    This function is used to arithmetically encode the coded
*    block pattern of an 8x8 block
****************************************************************************
*/
void write_cbp_bit_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t* currMB, int b8, int bit, int cbp, EncodingEnvironmentPtr eep_dp, textureInfoContexts_t *ctx)
{
	PixelPos block_a;
	int a = 0, b = 0;

	int mb_x = (b8 & 0x01) << 1;
	int mb_y = (b8 >> 1) << 1;

	int mbx = currMB->mbx, mby = currMB->mby, mbxmax = frameinfo->width / 16;
	mbinfo_store_t *mbA = currMB->mb_left, *mbB = currMB->mb_up;

	if (mb_y == 0)
	{
		if (!((mbB == NULL) || (mbB->mbtype == I_PCM)))
		{
			b = (((mbB->cbp & (1 << (2 + (mb_x >> 1)))) == 0) ? 1 : 0);   //VG-ADD
		}

	}
	else
		b = (((cbp & (1 << (mb_x >> 1))) == 0) ? 1 : 0);

	if (mb_x == 0)
	{
		get4x4Neighbour(frameinfo, currMB, (mb_x << 2) - 1, (mb_y << 2), IS_LUMA, &block_a);

		//if (block_a.available && (!(p_Vid->mb_data[block_a.mb_addr].mb_type == I_PCM)))
		if (block_a.available)
		{
			if (block_a.mb_addr == currMB->mbpos)
			{
				a = (((currMB->cbp & (1 << (2 * (block_a.y >> 1) + 1))) == 0) ? 1 : 0);                                   //todo: check carefully
			}
			else
			{
				a = (((frameinfo->mbinfo_stored[block_a.mb_addr].cbp & (1 << (2 * (block_a.y >> 1) + 1))) == 0) ? 1 : 0); //todo: check carefully
			}
		}
	}
	else
		a = (((cbp & (1 << mb_y)) == 0) ? 1 : 0);

	//===== WRITE BIT =====
	binary_encode_symbol(eep_dp, bit, ctx->cbp_contexts[0] + a + 2 * b);
}


/*!
****************************************************************************
*    This function is used to arithmetically encode the coded
*    block pattern of a macroblock
****************************************************************************
*/
void write_cbp_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{

	int curr_len = algorithm_enc_bits_written(eep);
	textureInfoContexts_t *ctx = &(currSlice->text_ctx);

	int a0 = 0, a1 = 0, b0 = 0, b1 = 0;
	int curr_cbp_ctx;
	int cbp = se->value1; // symbol to encode
	int cbp_bit;
	int b8;

	int mbx = currMB->mbx, mby = currMB->mby, mbxmax = frameinfo->width / 16;
	mbinfo_store_t *mbA = currMB->mb_left, *mbB = currMB->mb_up;

	for (b8 = 0; b8 < 4; ++b8)
	{
		write_cbp_bit_cabac(frameinfo, currSlice, currMB, b8, cbp&(1 << b8), cbp, eep, ctx);
	}

	// coding of chroma part
	if (mbB != NULL)
	{
		if (mbB->mbtype == I_PCM || (mbB->cbp > 15))
			b0 = 2;
	}

	if (mbA != NULL)
	{
		if (mbA->mbtype == I_PCM || (mbA->cbp > 15))
			a0 = 1;
	}

	curr_cbp_ctx = a0 + b0;
	cbp_bit = (cbp > 15) ? 1 : 0;
	binary_encode_symbol(eep, cbp_bit, ctx->cbp_contexts[1] + curr_cbp_ctx);

	if (cbp > 15)
	{
		if (mbB != NULL)
		{
			if (mbB->mbtype == I_PCM ||
				((mbB->cbp > 15) && ((mbB->cbp >> 4) == 2)))
				b1 = 2;
		}

		if (mbA != NULL)
		{
			if (mbA->mbtype == I_PCM ||
				((mbA->cbp > 15) && ((mbA->cbp >> 4) == 2)))
				a1 = 1;
		}

		curr_cbp_ctx = a1 + b1;
		cbp_bit = ((cbp >> 4) == 2) ? 1 : 0;
		binary_encode_symbol(eep, cbp_bit, ctx->cbp_contexts[2] + curr_cbp_ctx);
	}
	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}


/*!
****************************************************************************
*    This function is used to arithmetically encode the given delta quant.
****************************************************************************
*/
void write_dquant_cabac(slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);

	textureInfoContexts_t *ctx = &(currSlice->text_ctx);

	int dquant = se->value1;
	int sign = (dquant <= 0) ? 0 : -1;
	int act_sym = (iabs(dquant) << 1) + sign;
	int act_ctx = ((currMB->prev_dqp != 0) ? 1 : 0);

	if (act_sym == 0)
	{
		binary_encode_symbol(eep, 0, ctx->delta_qp_contexts + act_ctx);
	}
	else
	{
		binary_encode_symbol(eep, 1, ctx->delta_qp_contexts + act_ctx);
		act_ctx = 2;
		act_sym--;
		unary_bin_encode(eep, act_sym, ctx->delta_qp_contexts + act_ctx, 1);
	}
	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}


/*!
************************************************************************
*   CBP, DQUANT of an macroblock
************************************************************************
*/

int write_cbp_and_dquant(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{
	int             rate = 0;
	SyntaxElement   se;	

	int  cbp = currMB->cbp;     //((cbp_chroma << 4) | cbp_luma)  

	se.value2 = 0; // Initialize value2 to avoid problems

	if (currMB->mbtype != I_16x16)
	{
		se.value1 = cbp;
		se.type = SE_CBP;

		write_cbp_cabac(frameinfo, currSlice, currMB, &se, eep);
		rate += se.len;
	}

	//=====   DQUANT   =====
	//----------------------
	if (cbp != 0 || (currMB->mbtype == I_16x16))
	{
		const int min_quant = 0;
		const int max_quant = 51;
		const int quant_levels = (max_quant - min_quant) + 1;
		const int min_dquant = -(quant_levels / 2);
		const int max_dquant = (quant_levels / 2) - 1;

		int delta_qp = currMB->mquant - currMB->prev_qp;
		//*last_coded_qp = (uint8_t)mb->mquant;
		//todo: update the last_coded_qp outside

		if (delta_qp < min_dquant)
			delta_qp += quant_levels;
		else if (delta_qp > max_dquant)
			delta_qp -= quant_levels;
		
		se.value1 = delta_qp;
		se.type = SE_DELTA_QUANT;

		write_dquant_cabac(currSlice, currMB, &se, eep);
		rate += se.len;
	}

	return rate;
}




/*!
************************************************************************
*    Converts macroblock type to coding value
************************************************************************
*/
int MBType2Value(slice_enc_t *currSlice, mbinfo_t *currMB)
{

	switch (currMB->mbtype)
	{
		case I_4x4:
			return ((currSlice->slice_type == I_SLICE) ? 0: 6);
			break;
		case I_16x16:
			assert(currMB->i16offset != -1);
			return ((currSlice->slice_type == I_SLICE) ?0:6) + currMB->i16offset;
			break;
		case I_PCM:
			assert(0);
			return ((currSlice->slice_type == I_SLICE) ? 25: 31);
			break;
		case P_8x8:
		{
			if (currSlice->symbol_mode == CABAC)
				return 4;
			else
				return 5;
		}
		case P_SKIP:
			return 0;
		    break;
		case P_16x16:
		case P_16x8:
		case P_8x16:
			return currMB->mbtype + 1;
			break;
		default:
			return currMB->mbtype;
			break;
	}
}

//flatten currmb's mvd , using for cabac encoding
void mvd_flatten(mbinfo_t *currMB)
{
	int mbtype = currMB->mbtype;
	int x, y, mvd_x1, mvd_y1, mvd_x2,mvd_y2;

	mvd_x1 = currMB->mv1.x - currMB->pred1.x;
	mvd_y1 = currMB->mv1.y - currMB->pred1.y;

	mvd_x2 = currMB->mv2.x - currMB->pred2.x;
	mvd_y2 = currMB->mv2.y - currMB->pred2.y;

	if (mbtype == P_16x16)
	{
		for (y = 0; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				currMB->mvd[y][x][0] = mvd_x1;
				currMB->mvd[y][x][1] = mvd_y1;
			}
	}
	else if (mbtype == P_8x16)
	{

		for (y = 0; y < 4; y++)
			for (x = 0; x < 2; x++)
			{
				currMB->mvd[y][x][0] = mvd_x1;
				currMB->mvd[y][x][1] = mvd_y1;
			}

		for (y = 0; y < 4; y++)
			for (x = 2; x < 4; x++)
			{
				currMB->mvd[y][x][0] = mvd_x2;
				currMB->mvd[y][x][1] = mvd_y2;
			}

	}
	else if (mbtype == P_16x8)
	{
		for (y = 0; y < 2; y++)
			for (x = 0; x < 4; x++)
			{
				currMB->mvd[y][x][0] = mvd_x1;
				currMB->mvd[y][x][1] = mvd_y1;
			}

		for (y = 2; y < 4; y++)
			for (x = 0; x < 4; x++)
			{
				currMB->mvd[y][x][0] = mvd_x2;
				currMB->mvd[y][x][1] = mvd_y2;
			}

	}
	else
	{
		memset(&currMB->mvd[0][0][0], 0, sizeof(currMB->mvd));
		assert(0);// not support
	}

}


int enc_update_mb_assist_info(const frameinfo_t * frameinfo, const slice_enc_t *currSlice, mbinfo_t *currMB)
{
	mbinfo_store_t *mbA = NULL, *mbB = NULL, *mbC = NULL, *mbD = NULL;
	int mbx = currMB->mbx, mby = currMB->mby, mbxmax = frameinfo->width / 16;;
	bool left, up, up_left, up_right, prev_mb;
	int  cbp_luma = taa_h264_get_cbp_luma(currMB);
	int  cbp_chroma = taa_h264_get_cbp_chroma(currMB);
	int  cbp = 0;

	frameinfo->mbinfo_stored[currMB->mbpos].mbx = currMB->mbx;
	frameinfo->mbinfo_stored[currMB->mbpos].mby = currMB->mby;

	if (currMB->mbtype == I_16x16)
	{
		const int ycbp = currMB->mbcoeffs.luma_cbp;
		if (ycbp != 0)cbp_luma = 0xF;		
	}

	cbp = ((cbp_chroma << 4) | cbp_luma);					//todo: check this carefully for I_16x16
	currMB->cbp = cbp;
	currMB->cbp_bits[0] = currMB->cbp_bits[1] = currMB->cbp_bits[2] = 0;

	//todo: check Neighbour carefully here
	left    = MB_LEFT(currMB->avail_flags);
	up      = MB_UP(currMB->avail_flags);
	up_left = MB_UP_LEFT(currMB->avail_flags);
	up_right = MB_UP_RIGHT(currMB->avail_flags);
	prev_mb = currMB->mbpos > currSlice->first_mb_in_slice;  //if not the same slice , prev_mb not exist, todo: may be bug check carefully

	if (left)
	{
		mbA = &(frameinfo->mbinfo_stored[mby*mbxmax + mbx - 1]);
	}

	if (up)
	{
		mbB = &(frameinfo->mbinfo_stored[(mby - 1)*mbxmax + mbx]);
	}

	if (up_left)
	{
		mbD = &(frameinfo->mbinfo_stored[(mby - 1)*mbxmax + mbx - 1]);
	}

	if (up_right)
	{
		mbC = &(frameinfo->mbinfo_stored[(mby - 1)*mbxmax + mbx + 1]);
	}

	currMB->mb_left = mbA;
	currMB->mb_up = mbB;
	currMB->mb_up_left = mbC;
	currMB->mb_up_right = mbD;

	if (prev_mb)
	{
	  assert(currMB->mbpos > 0);
	  currMB->prevMb = &(frameinfo->mbinfo_stored[currMB->mbpos - 1]);
	  currMB->prev_qp = currMB->prevMb->qp;
	  currMB->prev_dqp = currMB->prev_qp - currMB->prevMb->prev_qp;
	}else {
		currMB->prevMb = NULL;
		currMB->prev_qp = currSlice->qp;
		currMB->prev_dqp = 0;	
	}

	if (currMB->mbtype == I_16x16)
	{
		currMB->i16offset = (cbp & 0xf ? 13 : 1) + currMB->best_i16x16_mode + ((cbp & 0x30) >> 2);
	}
	else {
		currMB->i16offset = -1;
	}

	if (!MB_TYPE_IS_INTRA(currMB->mbtype))
		mvd_flatten(currMB);

	return  0;
}



/*!
****************************************************************************
*    Write Significance MAP
****************************************************************************
*/
void write_significance_map(mbinfo_t *currMB, EncodingEnvironmentPtr eep, int type, int coeff[], int coeff_ctr, textureInfoContexts_t*  tex_ctx)
{
	int   k;
	int   sig, last;
	int   k0 = 0;
	int   k1 = maxpos[type];

	int               fld = 0;
	BiContextTypePtr  map_ctx = tex_ctx->map_contexts[fld][type2ctx_map[type]];
	BiContextTypePtr  last_ctx = tex_ctx->last_contexts[fld][type2ctx_last[type]];
	const uint8_t *pos2ctxmap =  pos2ctx_map[type];
	const uint8_t *pos2ctxlast = pos2ctx_last[type];

	if (!c1isdc[type])
	{
		++k0;
		++k1;
		--coeff;
	}

	for (k = k0; k < k1; ++k) // if last coeff is reached, it has to be significant
	{
		sig = (coeff[k] != 0);
		binary_encode_symbol(eep, sig, map_ctx + pos2ctxmap[k]);
		if (sig)
		{
			last = (--coeff_ctr == 0);
			binary_encode_symbol(eep, last, last_ctx + pos2ctxlast[k]);
			if (last)
				return;
		}
	}
}




/*!
****************************************************************************
*    Write CBP4-BIT
****************************************************************************
*/
void write_and_store_cbp_block_bit(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep, int type, int cbp_bit, textureInfoContexts_t*  tex_ctx)
{
	mbinfo_store_t *mb_data = frameinfo->mbinfo_stored;
	int y_ac = (type == LUMA_16x16AC_CABAC_BLK || type == LUMA_4x4_CABAC_BLK);
	int y_dc = (type == LUMA_16x16DC_CABAC_BLK);
	int u_ac = (type == CHROMA_AC_CABAC_BLK && !currMB->is_v_block);
	int v_ac = (type == CHROMA_AC_CABAC_BLK &&  currMB->is_v_block);
	int chroma_dc = (type == CHROMA_DC_CABAC_BLK);
	int u_dc = (chroma_dc && !currMB->is_v_block);
	int v_dc = (chroma_dc &&  currMB->is_v_block);
	int y = (y_ac || u_ac || v_ac ? currMB->subblock_y : 0);
	int x = (y_ac || u_ac || v_ac ? currMB->subblock_x : 0);
	int bit = (y_dc ? 0 : y_ac ? 1 : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 : 23);
	int default_bit = (currMB->is_intra_block ? 1 : 0);
	int upper_bit = default_bit;
	int left_bit = default_bit;
	int ctx;

	int bit_pos_a = 0;
	int bit_pos_b = 0;

	PixelPos block_a, block_b;

	if (y_ac || y_dc)
	{
		get4x4Neighbour(frameinfo, currMB, x - 1, y, IS_LUMA, &block_a);
		get4x4Neighbour(frameinfo, currMB, x, y - 1, IS_LUMA, &block_b);
		if (y_ac)
		{
			if (block_a.available)
				bit_pos_a = 4 * block_a.y + block_a.x;
			if (block_b.available)
				bit_pos_b = 4 * block_b.y + block_b.x;
		}
	}
	else
	{
		get4x4Neighbour(frameinfo, currMB, x - 1, y, IS_CHROMA, &block_a);
		get4x4Neighbour(frameinfo, currMB, x, y - 1, IS_CHROMA, &block_b);

		if (u_ac || v_ac)
		{
			if (block_a.available)
				bit_pos_a = (block_a.y << 2) + block_a.x;
			if (block_b.available)
				bit_pos_b = (block_b.y << 2) + block_b.x;
		}
	}

	bit = (y_dc ? 0 : y_ac ? 1 + y + (x >> 2) : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 + y + (x >> 2) : 35 + y + (x >> 2));
	//--- set bits for current block ---
	if (cbp_bit)
	{
		if (type == LUMA_8x8_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[0] |= ((int64_t)0x33 << bit);
		}
		else if (type == CB_8x8_CABAC_BLK)
		{
			assert(0); //not support now
			//currMB->cbp_bits_8x8[1] |= ((int64_t)0x33 << bit);
			currMB->cbp_bits[1] |= ((int64_t)0x33 << bit);
		}
		else if (type == CR_8x8_CABAC_BLK)
		{
			assert(0); //not support now
			//currMB->cbp_bits_8x8[2] |= ((int64_t)0x33 << bit);
			currMB->cbp_bits[2] |= ((int64_t)0x33 << bit);
		}
		else if (type == LUMA_8x4_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[0] |= ((int64_t)0x03 << bit);
		}
		else if (type == LUMA_4x8_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[0] |= ((int64_t)0x11 << bit);
		}
		else if (type == CB_8x4_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[1] |= ((int64_t)0x03 << bit);
		}
		else if (type == CR_8x4_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[2] |= ((int64_t)0x03 << bit);
		}
		else if (type == CB_4x8_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[1] |= ((int64_t)0x11 << bit);
		}
		else if (type == CR_4x8_CABAC_BLK)
		{
			assert(0); //not support now
			currMB->cbp_bits[2] |= ((int64_t)0x11 << bit);
		}
		else if ((type == CB_4x4_CABAC_BLK) || (type == CB_16AC_CABAC_BLK) || (type == CB_16DC_CABAC_BLK))
		{
			assert(0); //not support now
			currMB->cbp_bits[1] |= ((int64_t)0x01 << bit);
		}
		else if ((type == CR_4x4_CABAC_BLK) || (type == CR_16AC_CABAC_BLK) || (type == CR_16DC_CABAC_BLK))
		{
			assert(0); //not support now
			currMB->cbp_bits[2] |= ((int64_t)0x01 << bit);
		}
		else
		{
			currMB->cbp_bits[0] |= ((int64_t)0x01 << bit);
		}
	}

	bit = (y_dc ? 0 : y_ac ? 1 : u_dc ? 17 : v_dc ? 18 : u_ac ? 19 : 35);

	if (type != LUMA_8x8_CABAC_BLK)
	{
		if (block_b.available)
		{			
			if (block_b.mb_addr == currMB->mbpos)
			{
				if (currMB->mbtype == I_PCM)
					upper_bit = 1;
				else
					upper_bit = get_bit(currMB->cbp_bits[0], bit + bit_pos_b);
			}
			else
			{
				if (mb_data[block_b.mb_addr].mbtype == I_PCM)
					upper_bit = 1;
				else
					upper_bit = get_bit(mb_data[block_b.mb_addr].cbp_bits[0], bit + bit_pos_b);			
			}			
		}

		if (block_a.available)
		{
			
			if (block_a.mb_addr == currMB->mbpos)
			{
				if (currMB->mbtype == I_PCM)
					left_bit = 1;
				else
					left_bit = get_bit(currMB->cbp_bits[0], bit + bit_pos_a);
			}
			else 
			{
				if (mb_data[block_a.mb_addr].mbtype == I_PCM)
					left_bit = 1;
				else
					left_bit = get_bit(mb_data[block_a.mb_addr].cbp_bits[0], bit + bit_pos_a);
			
			}
				
		}
		ctx = (upper_bit << 1) + left_bit;
		binary_encode_symbol(eep, cbp_bit, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]] + ctx);
	}
}




/*!
************************************************************************
*    Exp Golomb binarization and encoding
************************************************************************
*/
void exp_golomb_encode_eq_prob(EncodingEnvironmentPtr eep_dp,
	unsigned int symbol,
	int k)
{
	for (;;)
	{
		if (symbol >= (unsigned int)(1 << k))
		{
			binary_encode_symbol_eq_prob(eep_dp, 1);   //first unary part
			symbol = symbol - (1 << k);
			k++;
		}
		else
		{
			binary_encode_symbol_eq_prob(eep_dp, 0);   //now terminated zero of unary part
			while (k--)                               //next binary part
				binary_encode_symbol_eq_prob(eep_dp, ((symbol >> k) & 1));
			break;
		}
	}
}



/*!
************************************************************************
*    Exp-Golomb for Level Encoding
************************************************************************/
static void unary_exp_golomb_level_encode(EncodingEnvironmentPtr eep,
	unsigned int symbol,
	BiContextTypePtr ctx)
{
	if (symbol == 0)
	{
		binary_encode_symbol(eep, 0, ctx);
		return;
	}
	else
	{
		unsigned int l = symbol;
		unsigned int k = 1;

		binary_encode_symbol(eep, 1, ctx);
		while (((--l)>0) && (++k <= 13))
			binary_encode_symbol(eep, 1, ctx);
		if (symbol < 13)
			binary_encode_symbol(eep, 0, ctx);
		else
			exp_golomb_encode_eq_prob(eep, symbol - 13, 0);
	}
}


/*!
************************************************************************
*    Exp-Golomb for MV Encoding
************************************************************************/
static void unary_exp_golomb_mv_encode(EncodingEnvironmentPtr eep,
	unsigned int symbol,
	BiContextTypePtr ctx,
	unsigned int max_bin)
{
	if (symbol == 0)
	{
		binary_encode_symbol(eep, 0, ctx);
		return;
	}
	else
	{
		unsigned int bin = 1;
		unsigned int l = symbol, k = 1;
		binary_encode_symbol(eep, 1, ctx++);

		while (((--l) > 0) && (++k <= 8))
		{
			binary_encode_symbol(eep, 1, ctx);
			if ((++bin) == 2)
				++ctx;
			if (bin == max_bin)
				++ctx;
		}
		if (symbol < 8)
			binary_encode_symbol(eep, 0, ctx);
		else
			exp_golomb_encode_eq_prob(eep, symbol - 8, 3);
	}
}



/*!
****************************************************************************
*    Write Levels
****************************************************************************
*/
void write_significant_coefficients(EncodingEnvironmentPtr eep, int type, int coeff[], textureInfoContexts_t*  tex_ctx, int coeff_cnt)
{
	BiContextType *one_contexts = tex_ctx->one_contexts[type2ctx_one[type]];
	BiContextType *abs_contexts = tex_ctx->abs_contexts[type2ctx_abs[type]];
	int   absLevel;
	int   ctx;
	short sign;
	int greater_one;
	int   c1 = 1;
	int   c2 = 0;
	int   i;

	for (i = maxpos[type]; (i >= 0) && (coeff_cnt); i--)
	{
		if (coeff[i] != 0)
		{
			coeff_cnt--;

			if (coeff[i] > 0)
			{
				absLevel = coeff[i];
				sign = 0;
			}
			else
			{
				absLevel = -coeff[i];
				sign = 1;
			}

			greater_one = (absLevel > 1);

			//--- if coefficient is one ---
			ctx = imin(c1, 4);
			binary_encode_symbol(eep, greater_one, one_contexts + ctx);

			if (greater_one)
			{
				ctx = imin(c2++, max_c2[type]);
				unary_exp_golomb_level_encode(eep, absLevel - 2, abs_contexts + ctx);
				c1 = 0;
			}
			else if (c1)
			{
				c1++;
			}
			binary_encode_symbol_eq_prob(eep, sign);
		}
	}
}



/*!
****************************************************************************
*    Write Block-Transform Coefficients
****************************************************************************
*/
void write_run_level_cabac(frameinfo_t * frameinfo,  slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{
	int curr_len = algorithm_enc_bits_written(eep);

	//--- accumulate run-level information ---
	if (se->value1 != 0)
	{
		currSlice->pos += se->value2;
		currSlice->coeff[currSlice->pos++] = se->value1;
		++currSlice->coeff_ctr;
	}
	else
	{
		textureInfoContexts_t *tex_ctx = &(currSlice->text_ctx);
		//=====  CBP-BIT =====
		if (currSlice->coeff_ctr > 0)
		{
			write_and_store_cbp_block_bit(frameinfo, currSlice, currMB, eep, se->context, 1, tex_ctx);
			//=====  significance map =====
			write_significance_map(currMB, eep, se->context, currSlice->coeff, currSlice->coeff_ctr, tex_ctx);
			//=====  significant coefficients =====
			write_significant_coefficients(eep, se->context, currSlice->coeff, tex_ctx, currSlice->coeff_ctr);

			memset(currSlice->coeff, 0, 64 * sizeof(int));
		}
		else
			write_and_store_cbp_block_bit(frameinfo, currSlice, currMB, eep, se->context, 0, tex_ctx);

		//--- reset counters ---
		currSlice->pos = currSlice->coeff_ctr = 0;
	}
	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}




/*!
************************************************************************
*    Writes Coeffs of an 4x4 block, just for luma
************************************************************************
*/
int write_coeff4x4_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, int plane, int b8, int b4, int is_intra, EncodingEnvironmentPtr eep)
{
	int             rate = 0;
	SyntaxElement   se;
	int k;
	const coeffs4_t * coeffs4 = &currMB->mbcoeffs.luma_ac[b8 * 4 + b4];
	const uint8_t * run = &coeffs4->run[0];
	const int8_t *  sign = &coeffs4->sign[0];
	const int16_t * level = &coeffs4->level[0];
	const int nnz = coeffs4->num_nonzero;

	int pl_off = plane << 2;

	currMB->subblock_x = ((b8 & 0x1) == 0) ? (((b4 & 0x1) == 0) ? 0 : 4) : (((b4 & 0x1) == 0) ? 8 : 12);   // horiz. position for coeff_count context
	currMB->subblock_y = (b8<2) ? ((b4<2) ? 0 : 4) : ((b4<2) ? 8 : 12);                                    // vert.  position for coeff_count context
	currMB->is_intra_block = is_intra;
	se.context = ((plane == 0) ? (LUMA_4x4_CABAC_BLK) : ((plane == 1) ? CB_4x4_CABAC_BLK : CR_4x4_CABAC_BLK));

	// DC
	se.type = (is_intra ? SE_LUM_DC_INTRA : SE_LUM_DC_INTER);
	se.value1 = (sign[0] == 0)?level[0]:-(int)level[0];  // level
	se.value2 = run[0];                                  // run

	write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
	rate += se.len;

	// AC coeff
	se.type = (is_intra ? SE_LUM_AC_INTRA : SE_LUM_AC_INTER);

	for (k = 1; k <= nnz; k++)
	{
		if (k == nnz) 
		{
			se.value1 = 0; // level
			se.value2 = 0; // run   
			//terminate and really encoding
			write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
		}
		else
		{
			se.value1 = (sign[k] == 0) ? level[k] : -(int)level[k]; // level
			se.value2 = run[k]; // run    
			write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
		}
		
		rate += se.len;
	}
	return rate;
}


/*!
************************************************************************
*    Writes Luma Coeff of an 8x8 block
************************************************************************
*/
int write_coeff8x8_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, int plane, int b8, int is_intra, EncodingEnvironmentPtr eep)
{
	int  rate = 0;
	
	rate += write_coeff4x4_cabac(frameinfo, currSlice, currMB, plane, b8, 0, is_intra, eep);
	rate += write_coeff4x4_cabac(frameinfo, currSlice, currMB, plane, b8, 1, is_intra, eep);
	rate += write_coeff4x4_cabac(frameinfo, currSlice, currMB, plane, b8, 2, is_intra, eep);
	rate += write_coeff4x4_cabac(frameinfo, currSlice, currMB, plane, b8, 3, is_intra, eep);

	return rate;
}


/*!
************************************************************************
*    Write Luma 16x16 Coeffcients 
************************************************************************
*/
int write_coeff16x16_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, int plane, EncodingEnvironmentPtr eep)
{
	int             cbp = currMB->cbp;
	int             mb_x, mb_y, i, j, k;
	int             rate = 0;
	int             block_rate = 0;
	SyntaxElement   se;
	int   b8, b4, is_intra;

	is_intra = (MB_TYPE_IS_INTRA(currMB->mbtype)); 
	if (currMB->mbtype != I_16x16)
	{
		//=====  L U M I N A N C E   =====
		if (cbp != 0)
		{
			for (b8 = 0; b8 < 4; b8++)
			{
				if (cbp & (1 << b8))
				{
					rate += write_coeff8x8_cabac(frameinfo, currSlice, currMB, plane, b8 , is_intra, eep);
				}
			}
		}
	}
	else
	{
		//=====  L U M I N A N C E   f o r   1 6 x 1 6   =====
		const coeffs4_t * coeffs4 = &currMB->mbcoeffs.luma_dc;
		const uint8_t * run = &coeffs4->run[0];
		const int8_t * sign = &coeffs4->sign[0];
		const int16_t * level = &coeffs4->level[0];
		int nnz = coeffs4->num_nonzero;


		currMB->is_intra_block = 1;
		se.context = LUMA_16x16DC_CABAC_BLK;
		se.type = SE_LUM_DC_INTRA;   // element is of type DC

		//Luma DC
		for (k = 0; k <= nnz; k++)
		{
			if (k == nnz)
			{
				se.value1 = 0; // level
				se.value2 = 0; // run   
				//terminate and really encoding
				write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
			}
			else
			{
				se.value1 = (sign[k] == 0) ? level[k] : -(int)level[k]; // level
				se.value2 = run[k];                                     // run    
				write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
			}
			block_rate += se.len;
		}

		rate += block_rate;

		// Luma AC
		if (cbp & 15)
		{
			se.context = LUMA_16x16AC_CABAC_BLK;
			se.type = SE_LUM_AC_INTRA;

			block_rate = 0;

			for (mb_y = 0; mb_y < 4; mb_y += 2)
			{
				for (mb_x = 0; mb_x < 4; mb_x += 2)                // 8x8
				{
					for (j = mb_y; j < mb_y + 2; j++)
					{
						int j1 = 2 * (j >> 1);
						int j2 = 2 * (j & 0x01);
						currMB->subblock_y = (j << 2);

						for (i = mb_x; i < mb_x + 2; i++)         //4x4 in 8x8
						{
							currMB->subblock_x = (short)(i << 2);
							b8 = j1 + (i >> 1);
							b4 = j2 + (i & 0x01);

							coeffs4 = &currMB->mbcoeffs.luma_ac[b8 * 4 + b4];
							run = &coeffs4->run[1];
							sign = &coeffs4->sign[1];
							level = &coeffs4->level[1];
							nnz = coeffs4->num_nonzero;

							for (k = 0; k <= nnz; k++)
							{
								if (k == nnz)
								{
									se.value1 = 0; // level
									se.value2 = 0; // run   
									//terminate and really encoding
									write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
								}
								else
								{
									se.value1 = (sign[k] == 0) ? level[k] : -(int)level[k]; // level , for luma/chroma ac , coeff start from pos 1
									se.value2 = run[k];                                     // run 
									write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
								}
								block_rate += se.len;
							}
						}
					}
				}
			}
			rate += block_rate;
		}
	}

	return rate;
}



/*!
************************************************************************
*    Writes chroma coefficients
************************************************************************
*/
int write_chroma_coeff(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{

	int             rate = 0;
	int             block_rate = 0;
	SyntaxElement   se;
	int             cbp = currMB->cbp;
	int   k, uv;
	int   b4;

	currMB->is_intra_block = (currMB->mbtype == I_16x16 || currMB->mbtype == I_4x4);

	//=====   D C - C O E F F I C I E N T S
	if (cbp > 15)  // check if any chroma bits in coded block pattern is set
	{
			se.type = (currMB->is_intra_block ? SE_CHR_DC_INTRA : SE_CHR_DC_INTER);
			// choose the appropriate data partition
			se.context = CHROMA_DC_CABAC_BLK;

			for (uv = 0; uv < 2; ++uv)
			{

				coeffs2_t *   coeffs2 = &currMB->mbcoeffs.chroma_dc[uv];
                uint8_t * run = coeffs2->run;
                int8_t * sign = coeffs2->sign;
                int16_t * level = coeffs2->level;
                int nnz = coeffs2->num_nonzero;
				currMB->is_v_block = uv;

				for (k = 0; k <= nnz; ++k)
				{
					if (k == nnz)
					{
						se.value1 = 0; // level
						se.value2 = 0; // run   
						//terminate and really encoding
						write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
					}
					else
					{
						se.value1 = (sign[k] == 0) ? level[k] : (-level[k]); // level
						se.value2 = run[k]; // run 
						write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
					}
					block_rate += se.len;
				}


			}
			rate += block_rate;
	}

	//=====   A C - C O E F F I C I E N T S
	uv = -1;
	if (cbp >> 4 == 2) // check if chroma bits in coded block pattern = 10b
	{

			block_rate = 0;
			se.context = CHROMA_AC_CABAC_BLK;
			se.type = (currMB->is_intra_block ? SE_CHR_AC_INTRA : SE_CHR_AC_INTER);
			// choose the appropriate data partition

			for (uv = 0; uv < 2; uv++)
			{				
				for (b4 = 0; b4 < 4; b4++)
				{

					coeffs4_t *   coeffs4 = &currMB->mbcoeffs.chroma_ac[uv*4 + b4];
					uint8_t * run = &coeffs4->run[1];
					int8_t * sign = &coeffs4->sign[1];
					int16_t * level = &coeffs4->level[1];
					int nnz = coeffs4->num_nonzero;
					currMB->is_v_block = uv;

					currMB->subblock_y = subblk_offset_y[b4];
					currMB->subblock_x = subblk_offset_x[b4];

					for (k = 0; k <= nnz; k++)
					{
						if (k == nnz)
						{
							se.value1 = 0; // level
							se.value2 = 0; // run   
							//terminate and really encoding
							write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
						}
						else
						{
							se.value1 = (sign[k] == 0) ? level[k] : (-level[k]); // level , for luma/chroma ac , coeff start from pos 1
							se.value2 = run[k];                                  // run  
							write_run_level_cabac(frameinfo, currSlice, currMB, &se, eep);
						}
						block_rate += se.len;
					}
				}
			}
			rate += block_rate;
	}

	return rate;
}



/*!
************************************************************************
*    Codes macroblock header in p slice
************************************************************************
*/
int write_i_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{

	int             mb_nr = currMB->mbpos;
	SyntaxElement   se;
	int             no_bits = 0;

	//========= write mb_type (I_SLICE) =========
	se.value1 = MBType2Value(currSlice, currMB);
	se.value2 = 0;
	se.type = SE_MBTYPE;

	write_mb_I_type_cabac(frameinfo, currSlice, currMB, &se, eep);

	no_bits += se.len;

	if (currMB->mbtype == I_PCM)
	{
		assert(0);
		return no_bits;
	}

	//===== BITS FOR INTRA PREDICTION MODES ====
	no_bits += write_intra_modes(frameinfo, currSlice, currMB, eep);

	//===== BITS FOR CHROMA INTRA PREDICTION MODE ====
	no_bits += write_chroma_intra_pred_mode(frameinfo, currSlice, currMB, eep);

	no_bits += write_cbp_and_dquant(frameinfo, currSlice, currMB, eep);

    //===== LUMA CHROMA COEFF ====
	no_bits += write_coeff16x16_cabac(frameinfo, currSlice, currMB, 0, eep);

	no_bits += write_chroma_coeff(frameinfo, currSlice, currMB, eep);
	printf("range: %d\n", (eep)->Erange);

	return no_bits;
}




/*!
***************************************************************************
*    This function is used to arithmetically encode the mb_skip_flag.
***************************************************************************
*/
void write_mb_pskip_flag_cabac(slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{

	int curr_len = algorithm_enc_bits_written(eep);
	motionInfoContext_t *ctx = &(currSlice->mot_ctx);
	int        curr_mb_type = se->value1;
	int a = (currMB->mb_left != NULL && (currMB->mb_left->skip == 0)) ? 1 : 0;
	int b = (currMB->mb_up != NULL && (currMB->mb_up->skip == 0)) ? 1 : 0;
	int act_ctx = a + b;

	if (curr_mb_type == 0) // SKIP
		binary_encode_symbol(eep, 1, &ctx->mb_type_contexts[1][act_ctx]);
	else
		binary_encode_symbol(eep, 0, &ctx->mb_type_contexts[1][act_ctx]);

	currMB->skip_flag = (curr_mb_type == 0) ? 1 : 0;

	se->context = act_ctx;
	se->value1 = 1 - currMB->skip_flag;

	se->len = (algorithm_enc_bits_written(eep) - curr_len);
}



/*!
****************************************************************************
*    This function is used to arithmetically encode the motion
*    vector data of a MB.
****************************************************************************
*/
int write_mvd_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, SyntaxElement *se, EncodingEnvironmentPtr eep)
{

	int curr_len = algorithm_enc_bits_written(eep);
	motionInfoContext_t  *ctx = &currSlice->mot_ctx;

	int i = currMB->subblock_x;
	int j = currMB->subblock_y;
	int a, b;
	int act_ctx;
	int act_sym;
	int mv_pred_res;
	int mv_local_err;
	int mv_sign;
	int list_idx = se->value2 & 0x01;
	int k = (se->value2 >> 1); // MVD component

	PixelPos block_a, block_b;


	get4x4Neighbour(frameinfo, currMB, i - 1, j, IS_LUMA,  &block_a);
	get4x4Neighbour(frameinfo, currMB, i, j - 1, IS_LUMA,  &block_b);

	if (block_b.available)
	{
		int x;
		x = (block_b.mb_addr == currMB->mbpos) ? currMB->mvd[block_a.y][block_a.x][k] : frameinfo->mbinfo_stored[block_a.mb_addr].mvd[block_a.y][block_a.x][k];
		b = iabs(frameinfo->mbinfo_stored[block_b.mb_addr].mvd[block_b.y][block_b.x][k]);
	}
	else
		b = 0;

	if (block_a.available)
	{
		int x = 0;
		x = (block_a.mb_addr == currMB->mbpos) ? currMB->mvd[block_a.y][block_a.x][k] : frameinfo->mbinfo_stored[block_a.mb_addr].mvd[block_a.y][block_a.x][k];
		a = iabs(x);
	}
	else
		a = 0;

	if ((mv_local_err = a + b)<3)
		act_ctx = 5 * k;
	else
	{
		if (mv_local_err>32)
			act_ctx = 5 * k + 3;
		else
			act_ctx = 5 * k + 2;
	}

	mv_pred_res = se->value1;
	se->context = act_ctx;

	act_sym = iabs(mv_pred_res);

	if (act_sym == 0)
		binary_encode_symbol(eep, 0, &ctx->mv_res_contexts[0][act_ctx]);
	else
	{
		binary_encode_symbol(eep, 1, &ctx->mv_res_contexts[0][act_ctx]);
		act_sym--;
		act_ctx = 5 * k;
		unary_exp_golomb_mv_encode(eep, act_sym, ctx->mv_res_contexts[1] + act_ctx, 3);
		mv_sign = (mv_pred_res<0) ? 1 : 0;
		binary_encode_symbol_eq_prob(eep, mv_sign);
	}

	se->len = (algorithm_enc_bits_written(eep) - curr_len);

	return se->len;
}


/*!
************************************************************************
*    Writes motion info (p slice)
************************************************************************
*/
int write_p_slice_motion_info(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{
	int   no_bits = 0;
	//=== If multiple ref. frames, write reference frame for the MB ===
	if (currMB->mbtype != I_4x4  && currMB->mbtype != I_16x16 && currMB->mbtype != I_PCM  && currMB->mbtype != P_SKIP)
	{
		const int num_parts[] = { 1, 2, 2 };
		int   step_h0 = (block_size[currMB->mbtype][0] >> 2);
		int   step_v0 = (block_size[currMB->mbtype][1] >> 2);
		SyntaxElement se;

		// Not support P8x8 and below mode now
		// Since we always have only one active ref frame we don't need to send ref_idx_l0, only the vectors. comment out following code when you need multi ref 

		/*
		if ((currMB->mb_type != P8x8) || !ZeroRef(currMB) || currSlice->symbol_mode == CABAC)
		{
			for (j0 = 0; j0 < 4; j0 += step_v0)
			{
				for (i0 = 0; i0 < 4; i0 += step_h0)
				{
					k = j0 + (i0 >> 1);
					if (currMB->b8x8[k].mode != 0 && (currMB->b8x8[k].pdir == 0 || currMB->b8x8[k].pdir == 2))
					{
						no_bits += writeReferenceFrame(currMB, i0, j0, LIST_0, motion[currMB->block_y + j0][currMB->block_x + i0].ref_idx[LIST_0]);
					}
				}
			}
		}
		*/

		//===== write forward motion vectors =====
		assert(currMB->mv1.ref_num == 0);

		se.value1 = currMB->mv1.x - currMB->pred1.x;
		se.value2 = 0; // identifies the component and the direction; only used for context determination
		no_bits += write_mvd_cabac(frameinfo, currSlice, currMB, &se, eep);

		se.value1 = currMB->mv1.y - currMB->pred1.y;
		se.value2 = 1<<1; // identifies the component and the direction; only used for context determination
		no_bits += write_mvd_cabac(frameinfo, currSlice, currMB, &se, eep);

		if (num_parts[currMB->mbtype] == 2)
		{
			assert(currMB->mv2.ref_num == 0);

			se.value1 = currMB->mv2.x - currMB->pred2.x;
			se.value2 = 0; // identifies the component and the direction; only used for context determination
			no_bits += write_mvd_cabac(frameinfo, currSlice, currMB, &se, eep);

			se.value1 = currMB->mv2.y - currMB->pred2.y;
			se.value2 = 1 << 1; // identifies the component and the direction; only used for context determination
			no_bits += write_mvd_cabac(frameinfo, currSlice, currMB, &se, eep);
		}

	}

	return no_bits;
}



/*!
************************************************************************
*    Codes macroblock header in p slice
************************************************************************
*/
int write_p_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep)
{

	SyntaxElement   se;
	int             no_bits = 0;
	int             skip = (currMB->mbtype == P_SKIP);
	int             prevMbSkipped = 0;
	int             WriteFrameFieldMBInHeader = 0;
	int             coeff_rate = 0;

	int mb_type = MBType2Value(currSlice, currMB);

	//========= write mb_skip_flag (CABAC) =========    
	se.value1 = mb_type;
	se.value2 = currMB->cbp;
	se.type = SE_MBTYPE;

	write_mb_pskip_flag_cabac(currSlice, currMB, &se,  eep);

	no_bits += se.len;

	//========= write mb_type (CABAC) =========
	if (currMB->mbtype != P_SKIP)
	{
		se.value1 = mb_type;
		se.value2 = 0;
		se.type = SE_MBTYPE;

		write_mb_P_type_cabac(frameinfo, currSlice, currMB, &se, eep);

		no_bits += se.len;
	}

	if (currMB->mbtype == I_PCM)
	{
		assert(0);
		return no_bits;
	}


	//===== BITS FOR 8x8 SUB-PARTITION MODES, NOT SUPPORT NOW =====
	if (currMB->mbtype == P_8x8)
	{
		/*
		for (i = 0; i < 4; ++i)
		{
			se.value1 = B8Mode2Value();
			se.value2 = 0;
			se.type = SE_MBTYPE;
			writeB8_typeInfo();
			no_bits += se.len;
		}
		*/
		assert(0);
		//no_bits += write_motion_info();
	}


	//===== BITS FOR INTRA PREDICTION MODES ====
	//===== BITS FOR CHROMA INTRA PREDICTION MODE ====
	if (MB_TYPE_IS_INTRA(currMB->mbtype))
	{
		no_bits += write_intra_modes(frameinfo, currSlice, currMB, eep);
		no_bits += write_chroma_intra_pred_mode(frameinfo, currSlice, currMB, eep);
	}

	//----- motion information -----
	if (currMB->mbtype != P_SKIP && currMB->mbtype != P_8x8 && (!MB_TYPE_IS_INTRA(currMB->mbtype)))
	{
		no_bits += write_p_slice_motion_info(frameinfo, currSlice, currMB, eep);
	}

	if (currMB->mbtype != P_SKIP)
	{
		coeff_rate = write_cbp_and_dquant(frameinfo, currSlice, currMB, eep);
		coeff_rate += write_coeff16x16_cabac(frameinfo, currSlice, currMB, 0, eep);
		coeff_rate += write_chroma_coeff(frameinfo, currSlice, currMB, eep);
		no_bits += coeff_rate;
	}

	printf("range: %d\n", (eep)->Erange);
	return no_bits;
}



/*!
************************************************************************
*    add slice bin number to picture bin counter
*    should be only used when slice is terminated
************************************************************************
*/
void set_pic_bin_count(frameinfo_t * frameinfo, EncodingEnvironmentPtr eep)
{
	frameinfo->pic_bin_count += (eep->E << 3) + eep->C; // no of processed bins
}


void reset_pic_bin_count(frameinfo_t * frameinfo)
{
	frameinfo->pic_bin_count = 0;
}

void write_terminating_bit(EncodingEnvironmentPtr eep, int bit)
{
	binary_encode_symbol_final(eep, bit);
}



/*!
************************************************************************
*    This function terminates a slice (but doesn't write it out),
*    the old terminate_slice (0)
* \return
*    0 if OK,                                                         \n
*    1 in case of error
************************************************************************
*/
int terminate_slice_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep, int lastslice)
{

	if (currSlice->symbol_mode == CABAC)
		write_terminating_bit(eep, 1);      // only once, not for all partitions


	// terminate the arithmetic code
	algorithm_enc_done(eep);
	set_pic_bin_count(frameinfo, eep);

	if (lastslice)
	{
		//impl this to Inserting  cabac_zero_word syntax elements/bytes (Clause 7.4.2.10)
		//addCabacZeroWords(); this has been impl in the taa_h264_postprocess_output_buffer
		//assert(0);
	}

	return 0;  //todo: check the return
}





/*!
************************************************************************
*    Initializes the EncodingEnvironment for the arithmetic coder
************************************************************************
*/
void enc_cabac_start_encoding(EncodingEnvironmentPtr eep, bitwriter_t *writer,
	unsigned char *code_buffer,
	int *code_len)
{
	eep->w = writer;
	eep->Elow = 0;
	eep->Echunks_outstanding = 0;
	eep->Ebuffer = 0;
	eep->Epbuf = -1;                     // to remove redundant chunk ^^
	eep->Ebits_to_go = BITS_TO_LOAD + 1; // to swallow first redundant bit

	eep->Ecodestrm_len = code_len;

	eep->Erange = HALF;
	eep->E = 0;
	eep->C = 0;


	eep->saved_Elow = eep->Elow;
	eep->saved_Erange = eep->Erange;
	eep->saved_Ebuffer = eep->Ebuffer;
	eep->saved_Ebits_to_go = eep->Ebits_to_go;
	eep->saved_Echunks_outstanding = eep->Echunks_outstanding;
	eep->saved_Epbuf = eep->Epbuf;
	eep->saved_Ecodestrm_len = *(eep->Ecodestrm_len);
	eep->saved_C = eep->C;
	eep->saved_E = eep->E;
}




void enc_init_slice(const sequence_t * sequence, slice_enc_t *currSlice, int first_mb_in_slice, short slice_type, int slice_qp)
{
	memset(currSlice, 0, sizeof(slice_enc_t));
	currSlice->first_mb_in_slice = first_mb_in_slice;
	currSlice->slice_type = slice_type;
	currSlice->qp = slice_qp;
	currSlice->coeff;
	currSlice->coeff_ctr = 0;
	currSlice->pos = 0;
	currSlice->model_number = 0;

	if (sequence->entropy_coding_mode_flag)
	{
		currSlice->symbol_mode = CABAC;
	}
	else
	{
		currSlice->symbol_mode = CAVLC;
	}

	if (currSlice->symbol_mode == CABAC)
	{
		enc_init_cabac_contexts(currSlice);
	}

}




/* Save the state of cabac so we can return to this state later. */
void enc_save_cabac_eep_state(EncodingEnvironmentPtr eep)
{
	eep->saved_Elow                  = eep->Elow;
	eep->saved_Erange                = eep->Erange;
	eep->saved_Ebuffer               = eep->Ebuffer;
	eep->saved_Ebits_to_go           = eep->Ebits_to_go;
	eep->saved_Echunks_outstanding   = eep->Echunks_outstanding;
	eep->saved_Epbuf                 = eep->Epbuf;
	eep->saved_Ecodestrm_len         = *(eep->Ecodestrm_len);
	eep->saved_C                     = eep->C;
	eep->saved_E                     = eep->E;
	
}

void enc_load_cabac_eep_state(EncodingEnvironmentPtr eep)
{

	eep->Elow = eep->saved_Elow;
	eep->Erange = eep->saved_Erange;
	eep->Ebuffer = eep->saved_Ebuffer;
	eep->Ebits_to_go = eep->saved_Ebits_to_go;
	eep->Echunks_outstanding = eep->saved_Echunks_outstanding;
	eep->Epbuf = eep->saved_Epbuf;
	*(eep->Ecodestrm_len) = eep->saved_Ecodestrm_len;
	eep->C = eep->saved_C;
	eep->E = eep->saved_E;

}
