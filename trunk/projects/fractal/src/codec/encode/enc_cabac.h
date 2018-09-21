#ifndef _ENC_CABAC_H_
#define _ENC_CABAC_H_

#include "taah264stdtypes.h"
#include "enc_typedefs.h"

//! Syntax Element
typedef struct syntaxelement_cabac_enc
{
	int                 type;           //!< type of syntax element for data part.
	int                 value1;         //!< numerical value of syntax element
	int                 value2;         //!< for blocked symbols, e.g. run/level
	int                 len;            //!< length of code
	int                 context;        //!< CABAC context

#if TRACE
#define             TRACESTRING_SIZE 100            //!< size of trace string
	char                tracestring[TRACESTRING_SIZE];  //!< trace string
#endif

} SyntaxElement;

// CABAC block types
enum {
	LUMA_16x16DC_CABAC = 0,
	LUMA_16x16AC_CABAC = 1,
	LUMA_8x8 = 2,
	LUMA_8x4 = 3,
	LUMA_4x8 = 4,
	LUMA_4x4_CABAC = 5,
	CHROMA_DC_CABAC = 6,
	CHROMA_AC_CABAC = 7,
	CHROMA_DC_2x4 = 8,
	CHROMA_DC_4x4 = 9,
	CB_16DC = 10,
	CB_16AC = 11,
	CB_8x8 = 12,
	CB_8x4 = 13,
	CB_4x8 = 14,
	CB_4x4_CABAC = 15,
	CR_16DC = 16,
	CR_16AC = 17,
	CR_8x8 = 18,
	CR_8x4 = 19,
	CR_4x8 = 20,
	CR_4x4_CABAC = 21
} CABACBlockTypes;


int write_chroma_intra_pred_mode(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);

void enc_cabac_start_encoding(EncodingEnvironmentPtr eep,
	unsigned char *code_buffer,
	int *code_len);

void enc_init_slice(sequence_t * sequence, slice_enc_t *currSlice,
	int first_mb_in_slice,
	short                slice_type,
	int                  last_coded_qp);

int write_i_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);
int write_p_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);
int terminate_slice_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep, int lastslice);

#endif

