#ifndef _ENC_MBUTILS_H_
#define _ENC_MBUTILS_H_

#include "com_common.h"
#include "com_intmath.h"
#include "com_mbutils.h"
#include "enc_typedefs.h"
#include "encoder.h"


void taa_h264_enc_store_mbinfo(
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
	mbinfo_store_t * mb);



#endif
