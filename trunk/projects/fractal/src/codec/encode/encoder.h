#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "taah264enc.h"
#include "com_common.h"
#include "com_cpuinfo.h"
#include "com_error.h"
#include "com_resolutions.h"
#include "enc_typedefs.h"
#include "enc_gop.h"
#include "enc_config.h"
#include "enc_cabac_engine.h"

#ifdef TAA_SAVE_264_MEINFO

#ifdef ENV_DARWIN
#include <stdio.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#endif
 
#ifdef ENV_WIN32
#include <stdio.h>
#include <winsock.h>
#include <stdlib.h> 
#endif

#endif

#define NUM_4x4_BLOCKS_Y  16
#define NUM_4x4_BLOCKS_UV 4
#define MB_WIDTH_Y        16
#define MB_HEIGHT_Y       16
#define MB_WIDTH_UV       8
#define MB_HEIGHT_UV      8
#define MB_STRIDE_UV      16
#define MB_SIZE_Y         (MB_WIDTH_Y * MB_HEIGHT_Y)
#define MB_SIZE_UV        (MB_WIDTH_UV * MB_HEIGHT_UV)

#define NUM_I4x4_MODES 9
#define NUM_I16x16_MODES 4
#define NUM_IMODES_UV 3

struct luma_s
{
  TAA_H264_ALIGN(64) uint8_t input[256];
  TAA_H264_ALIGN(64) uint8_t recon[256];
  TAA_H264_ALIGN(64) uint8_t do_4x4_enc[16];
};


struct chroma_s
{
  TAA_H264_ALIGN(64) uint8_t input[128];
  TAA_H264_ALIGN(64) uint8_t recon[128];
};


// Input to CAVLC which is the output of all calculations in enc_mb.c
struct coeffs4_s
{
  //int cbp;
  int num_nonzero;
  int8_t sign[16];   // 0...15 or 1...15
  uint8_t run[16];
  int16_t level[16]; // 0...15 or 1...15
};

struct coeffs2_s
{
  //int cbp;
  int num_nonzero;
  int8_t sign[4];
  uint8_t run[4];
  int16_t level[4];
};

struct mbcoeffs_s
{
  coeffs4_t luma_ac[NUM_4x4_BLOCKS_Y];
  coeffs4_t chroma_ac[NUM_4x4_BLOCKS_UV * 2];
  coeffs4_t luma_dc;
  coeffs2_t chroma_dc[2];
  uint16_t chroma_cbp;
  uint16_t luma_cbp;
};

struct mbinfo_s
{
  int mby;
  int mbx;
  int mbpos;
  int mbxmax;

  mv_t mv1;
  mv_t mv2;
  mv_t pred1;
  mv_t pred2;
  mv_t mvskip;
  mv_t max_mv_top_left;
  mv_t max_mv_bottom_right;

  mbcoeffs_t mbcoeffs;

  int mquant;
  int lambda;
  int thr;
  mbtype_t mbtype;
  unsigned approx_i4x4_cost;

  luma_t luma;
  chroma_t chroma;

  TAA_H264_ALIGN(64) uint8_t recon_top_y  [MB_WIDTH_Y+4];               /* reconstructed row above MB: [ 16 top | 4 top right ]*/
  TAA_H264_ALIGN(64) uint8_t recon_left_y [MB_HEIGHT_Y];                /* reconstructed col left of MB */
  TAA_H264_ALIGN(64) uint8_t recon_top_u  [MB_WIDTH_UV];
  TAA_H264_ALIGN(64) uint8_t recon_left_u [MB_HEIGHT_UV];
  TAA_H264_ALIGN(64) uint8_t recon_top_v  [MB_WIDTH_UV];
  TAA_H264_ALIGN(64) uint8_t recon_left_v [MB_HEIGHT_UV];
  TAA_H264_ALIGN(64) uint8_t recon_topleft_y;
  TAA_H264_ALIGN(64) uint8_t pred_i16x16_y[NUM_I16x16_MODES * MB_SIZE_Y];        /* stores intra predicted y */

  int avail_flags;          
  imode4_t pred_intra_modes [NUM_4x4_BLOCKS_Y];
  imode8_t intra_mode_chroma;
  imode16_t best_i16x16_mode;
  imode8_t best_i8x8_mode_chroma;

  bool encode_luma;
  bool encode_chroma;
  
  //following added for cabac , init on enc_update_mb_assist_info
  int cbp;                         //8x8 cbp , format (cbp_chroma<<4)|cpb_luma, for cabac
  int i16offset;                   //I_16x16  mode offset
  int skip_flag;                   //PSKIP or not
  int is_v_block;
  int subblock_y;
  int subblock_x;
  int64_t  cbp_bits[3]; 
  int is_intra_block;

  int prev_qp;
  int prev_dqp;
  mbinfo_store_t *prevMb;           //previous mb
  
  mbinfo_store_t *mb_left;          //mb_A
  mbinfo_store_t *mb_up;            //mb_B
  mbinfo_store_t *mb_up_right;      //mb_C
  mbinfo_store_t *mb_up_left;       //mb_D

  int mvd[4][4][2];                //[blky][blkx][y/x]
};


/**********************************************************************
* C O N T E X T S   F O R   T M L   S Y N T A X   E L E M E N T S
**********************************************************************
*/

#define NUM_MB_TYPE_CTX  11
#define NUM_MV_RES_CTX   10
#define NUM_REF_NO_CTX   6
#define NUM_DELTA_QP_CTX 4
#define NUM_MB_AFF_CTX   4
#define NUM_BLOCK_TYPES 10

#define NUM_TRANSFORM_SIZE_CTX 3

#define NUM_IPR_CTX    2
#define NUM_CIPR_CTX   4
#define NUM_CBP_CTX    4
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX    5
#define NUM_ABS_CTX    5

//! definition of H.264 syntax elements
typedef enum
{
	SE_HEADER,
	SE_PTYPE,
	SE_MBTYPE,
	SE_REFFRAME,
	SE_INTRAPREDMODE,
	SE_MVD,
	SE_CBP,
	SE_LUM_DC_INTRA,
	SE_CHR_DC_INTRA,
	SE_LUM_AC_INTRA,
	SE_CHR_AC_INTRA,
	SE_LUM_DC_INTER,
	SE_CHR_DC_INTER,
	SE_LUM_AC_INTER,
	SE_CHR_AC_INTER,
	SE_DELTA_QUANT,
	SE_BFRAME,
	SE_EOS,
	SE_MAX_ELEMENTS = 20 //!< number of maximum syntax elements
} SE_type;


struct motionInfoContext_s
{
	BiContextType mb_type_contexts[3][NUM_MB_TYPE_CTX];
	BiContextType mv_res_contexts[2][NUM_MV_RES_CTX];
	//BiContextType ref_no_contexts[2][NUM_REF_NO_CTX];  //only one reference not need this now
};


struct textureInfoContexts_s
{
	//BiContextType  transform_size_contexts[NUM_TRANSFORM_SIZE_CTX];                                 //not support transform 8x8 , not need this ctx now
	BiContextType  ipr_contexts[NUM_IPR_CTX];
	BiContextType  cipr_contexts[NUM_CIPR_CTX];
	BiContextType  cbp_contexts[3][NUM_CBP_CTX];
	BiContextType  bcbp_contexts[NUM_BLOCK_TYPES][NUM_BCBP_CTX];
	BiContextType  delta_qp_contexts[NUM_DELTA_QP_CTX];
	BiContextType  one_contexts[NUM_BLOCK_TYPES][NUM_ONE_CTX];
	BiContextType  abs_contexts[NUM_BLOCK_TYPES][NUM_ABS_CTX];
	BiContextType  map_contexts[1][NUM_BLOCK_TYPES][NUM_MAP_CTX];
	BiContextType  last_contexts[1][NUM_BLOCK_TYPES][NUM_LAST_CTX];
};

typedef enum
{
	CAVLC,
	CABAC
} SymbolMode;


typedef enum
{
	P_SLICE = 0,
	B_SLICE = 1,
	I_SLICE = 2,
	NUM_SLICE_TYPES = 3
} SliceType;

//just for cabac encoder now 
struct slice_enc_s
{
	int                  first_mb_in_slice;
	short                symbol_mode;
	short                slice_type;
	int                  qp;         //last_coded_qp

	int                  slice_size;
	int                  model_number;

	// Some Cabac related parameters (could be put in a different structure so we can dynamically allocate them when needed), using for ceff cabac
	int  coeff[64];
	int  coeff_ctr;
	int  pos;
	EncodingEnvironment ee_cabac;
	motionInfoContext_t mot_ctx;
	textureInfoContexts_t text_ctx;
};


struct frameinfo_s
{
  int width;
  int height;
  int frame_num;
  int pic_bin_count;          //for cabac

  meinfo_t * meinfo;
  frameipbuf_t * refframe;
  frameipbuf_t * refframes[MAX_NUM_REF_FRAMES];
  uint8_t refframe_num;
  uint8_t temporal_id;
  bool is_long_term;
  uint8_t long_term_idx;
  // Reference meta information for the current frame.
  ref_meta_t * ref_meta;
  // Reference meta information for the frames that _generates_ a long term frame.
  // This could be a short the frame which generates a lt frame via mmco.
  // The array is sorted with the lt index that is being generated.
  ref_meta_t * long_term_meta_list[MAX_NUM_REF_FRAMES];
  rplr_t rplr[2];
  framebuf_t recon;
  int approx_i4x4_penalty;

  /* TODO: These should only be stored for current and previous row. */
  uint8_t * num_coeffs_y;
  uint8_t * num_coeffs_uv;
  uint8_t * intra_modes;

  mbtype_t * mbtypes;
  fastpos_t motion_vectors_curr_16x16;
  fastpos_t motion_vectors_prev_16x16;
  mv_t * motion_vectors_curr;
  mv_t * motion_vectors_prev;
  mbinfo_store_t * mbinfo_stored;
  source_res_t resolution;

  mb_deblock_t * mb_deblock_info;
  unsigned mb_deblock_info_size;
};


struct sequence_s
{
  level_t level;
  uint8_t num_ref_frames;
  uint8_t sps_id;
  uint8_t pps_id;
  uint8_t idr_pic_id;
  uint8_t pps_qp;
  uint8_t crop_right;
  uint8_t crop_bottom;
  uint16_t sar_width;
  uint16_t sar_height;  
  unsigned num_frames_since_intra;
  uint8_t  entropy_coding_mode_flag;
};

struct encoder_s
{
  unsigned output_width;
  unsigned output_height;

  unsigned coding_options;
  unsigned using_high_profile;

  sequence_t sequence;
  slice_enc_t    slice;
  gop_t gop;
  uint8_t last_coded_qp;
  unsigned max_nalu_size;

  decoded_picture_buffer_t dpb;
  ref_meta_t refframes_meta[MAX_NUM_DPB_PICS];

  frameinfo_t frameinfo;
  control_t * control;
  bitwriter_t * bitwriter;
  resizer_t * resizer;
  cpuinfo_t cpuinfo;
  error_handler_t error_handler;
  taa_h264_enc_callbacks callbacks;

  int frames_encoded;
#ifdef TAA_SAVE_264_MEINFO
  unsigned frame_idx;
  unsigned char debug_flag;
  char debug_ip_addr[32];
  unsigned debug_port;
  uint8_t key_word[64];
  FILE *fp_meta_info;
  uint8_t meta_detail_flag;
  int sockfd;
  struct sockaddr_in serv_addr;
  uint8_t *data_need_send;
  unsigned data_position;
#endif

#ifdef TAA_DEBUG_READ_YUV_FILE
  unsigned used_yuvfile_flag;
  FILE *fp;
  unsigned cur_pos;
  unsigned total_frame;
  unsigned yuv_w;
  unsigned yuv_h;
#endif
};



#endif
