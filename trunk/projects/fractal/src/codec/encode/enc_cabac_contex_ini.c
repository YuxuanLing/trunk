/*****************************************************************************
* enc_cabac_contex_ini.c:  h264 arithmetic coder context init
*****************************************************************************
* Copyright (C) Cisco 2018
*
* Authors: Yuxuan Ling<yuxling@cisco.com>
*
*****************************************************************************/


#include "enc_cabac_ctx_tables.h"
#include "enc_cabac_contex_ini.h"


static inline int imax(int a, int b)
{
	return ((a) > (b)) ? (a) : (b);
}

#define BIARI_CTX_INIT2(qp, ii,jj,ctx,tab) \
{ \
  for (i=0; i<ii; i++) \
  for (j=0; j<jj; j++) \
  { \
    binary_init_context (qp, &(ctx[i][j]), &(tab[i][j][0])); \
  } \
}


static inline void binary_context_init1(int qp, int jj, BiContextType *ctx, const char table[][2])
{
	int j;
	for (j = 0; j < jj; j++)
	{
		binary_init_context(qp, &(ctx[j]), &(table[j][0]));
	}
}

static inline void binary_context_init2(int qp, int ii, int jj, BiContextType ctx[][11], const char table[][11][2])
{
	int i, j;
	for (i = 0; i < ii; i++)
	{
		for (j = 0; j < jj; j++)
		{
			binary_init_context(qp, &(ctx[i][j]), &(table[i][j][0]));
		}
	}
}



void enc_init_cabac_contexts(slice_enc_t *currSlice)
{
	motionInfoContext_t*  mc = &currSlice->mot_ctx;
	textureInfoContexts_t* tc = &currSlice->text_ctx;
	int model_number = currSlice->model_number;
	int qp = imax(0, currSlice->qp);
	int i = 0, j = 0;

	if (currSlice->slice_type == I_SLICE)
	{
		//--- motion coding contexts ---
		BIARI_CTX_INIT2(qp, 3, NUM_MB_TYPE_CTX, mc->mb_type_contexts, INIT_MB_TYPE_I[model_number]);

		//--- texture coding contexts ---
		binary_context_init1(qp, NUM_IPR_CTX, tc->ipr_contexts, INIT_IPR_I[model_number][0]);                      //intra pred ctx
		binary_context_init1(qp, NUM_CIPR_CTX, tc->cipr_contexts, INIT_CIPR_I[model_number][0]);                   //chroma intra pred ctx
		BIARI_CTX_INIT2(qp, 3, NUM_CBP_CTX, tc->cbp_contexts, INIT_CBP_I[model_number]);                           //cbp ctx
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_BCBP_CTX, tc->bcbp_contexts, INIT_BCBP_I[model_number]);          //bit cbp ctx
		binary_context_init1(qp, NUM_DELTA_QP_CTX, tc->delta_qp_contexts, INIT_DELTA_QP_I[model_number][0]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_MAP_CTX, tc->map_contexts[0], INIT_MAP_I[model_number]);          //significance map
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->last_contexts[0], INIT_LAST_I[model_number]);       //significance map last
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_ONE_CTX, tc->one_contexts, INIT_ONE_I[model_number]);             //significant coefficients greate than one or not
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_ABS_CTX, tc->abs_contexts, INIT_ABS_I[model_number]);             //significant coefficients abs level ctx
	}
	else
	{
		//--- motion coding contexts ---
		BIARI_CTX_INIT2(qp, 3, NUM_MB_TYPE_CTX, mc->mb_type_contexts, INIT_MB_TYPE_P[model_number]);
		BIARI_CTX_INIT2(qp, 2, NUM_MV_RES_CTX, mc->mv_res_contexts, INIT_MV_RES_P[model_number]);
		//--- texture coding contexts ---
		binary_context_init1(qp, NUM_IPR_CTX, tc->ipr_contexts, INIT_IPR_P[model_number][0]);
		binary_context_init1(qp, NUM_CIPR_CTX, tc->cipr_contexts, INIT_CIPR_P[model_number][0]);
		BIARI_CTX_INIT2(qp, 3, NUM_CBP_CTX, tc->cbp_contexts, INIT_CBP_P[model_number]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_BCBP_CTX, tc->bcbp_contexts, INIT_BCBP_P[model_number]);
		binary_context_init1(qp, NUM_DELTA_QP_CTX, tc->delta_qp_contexts, INIT_DELTA_QP_P[model_number][0]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_MAP_CTX, tc->map_contexts[0], INIT_MAP_P[model_number]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->last_contexts[0], INIT_LAST_P[model_number]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_ONE_CTX, tc->one_contexts, INIT_ONE_P[model_number]);
		BIARI_CTX_INIT2(qp, NUM_BLOCK_TYPES, NUM_ABS_CTX, tc->abs_contexts, INIT_ABS_P[model_number]);
	}
}
