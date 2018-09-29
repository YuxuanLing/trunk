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

} SyntaxElement;

// CABAC block types
enum {
	LUMA_16x16DC_CABAC_BLK = 0,
	LUMA_16x16AC_CABAC_BLK = 1,
	LUMA_8x8_CABAC_BLK = 2,
	LUMA_8x4_CABAC_BLK = 3,
	LUMA_4x8_CABAC_BLK = 4,
	LUMA_4x4_CABAC_BLK = 5,
	CHROMA_DC_CABAC_BLK = 6,
	CHROMA_AC_CABAC_BLK = 7,
	CHROMA_DC_2x4_CABAC_BLK = 8,
	CHROMA_DC_4x4_CABAC_BLK = 9,
	CB_16DC_CABAC_BLK = 10,
	CB_16AC_CABAC_BLK = 11,
	CB_8x8_CABAC_BLK = 12,
	CB_8x4_CABAC_BLK = 13,
	CB_4x8_CABAC_BLK = 14,
	CB_4x4_CABAC_BLK = 15,
	CR_16DC_CABAC_BLK = 16,
	CR_16AC_CABAC_BLK = 17,
	CR_8x8_CABAC_BLK = 18,
	CR_8x4_CABAC_BLK = 19,
	CR_4x8_CABAC_BLK = 20,
	CR_4x4_CABAC_BLK = 21
} CABACBlockTypes;


int write_chroma_intra_pred_mode(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);

void enc_cabac_start_encoding(EncodingEnvironmentPtr eep, bitwriter_t *writer,
	unsigned char *code_buffer,
	int *code_len);

void enc_init_slice(const sequence_t * sequence, slice_enc_t *currSlice,
	int first_mb_in_slice,
	short                slice_type,
	int                  last_coded_qp);

int  write_i_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);
int  write_p_slice_mb_layer_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep);
int  terminate_slice_cabac(frameinfo_t * frameinfo, slice_enc_t *currSlice, mbinfo_t *currMB, EncodingEnvironmentPtr eep, int lastslice);
void reset_pic_bin_count(frameinfo_t * frameinfo);
int  enc_update_mb_assist_info(const frameinfo_t * frameinfo, const slice_enc_t *currSlice, mbinfo_t *currMB);
void write_terminating_bit(EncodingEnvironmentPtr eep, int bit);

#endif

