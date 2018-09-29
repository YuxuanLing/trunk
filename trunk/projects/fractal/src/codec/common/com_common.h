#ifndef _COMMON_H_
#define _COMMON_H_

#include "taah264stdtypes.h"
#include "com_compatibility.h"

/**
 * Configuration common for encoder and decoder
 * **************************************************************************/
#define REF_FRAME_CHECKSUM 0


/**
 * Defined constants
 * **************************************************************************/
#define MAX_FRAME_WIDTH   4096
#define MAX_FRAME_HEIGHT  4096
#define MAX_FRAME_AREA    4096*2304
#define NUM_4x4_BLOCKS_Y  16
#define NUM_4x4_BLOCKS_UV 4
#define MB_WIDTH_Y        16
#define MB_HEIGHT_Y       16
#define MB_WIDTH_UV       8
#define MB_HEIGHT_UV      8
#define MB_STRIDE_UV      16
#define MB_SIZE_Y         (MB_WIDTH_Y * MB_HEIGHT_Y)
#define MB_SIZE_UV        (MB_WIDTH_UV * MB_HEIGHT_UV)

#define PAD_VERT_Y 32
#define PAD_HORZ_Y 32
#define PAD_VERT_UV 16
#define PAD_HORZ_UV 16


#define MBTYPE_ISLICE_OFFSET 5


/**
 * Enums
 * **************************************************************************/

typedef enum
{
  BASELINE_PROFILE  = 66,
  SCALABLE_BASELINE_PROFILE = 83
} profile_t;

typedef enum
{
  LEVEL_1_0 = 10,
  LEVEL_1_1 = 11,
  LEVEL_1_2 = 12,
  LEVEL_1_3 = 13,
  LEVEL_2_0 = 20,
  LEVEL_2_1 = 21,
  LEVEL_2_2 = 22,
  LEVEL_3_0 = 30,
  LEVEL_3_1 = 31,
  LEVEL_3_2 = 32,
  LEVEL_4_0 = 40,
  LEVEL_4_1 = 41,
  LEVEL_4_2 = 42,
  LEVEL_5_0 = 50,
  LEVEL_5_1 = 51
} level_t;

typedef enum
{
  NALU_PRIORITY_DISPOSABLE  = 0,
  NALU_PRIORITY_LOW         = 1,
  NALU_PRIORITY_NORMAL      = 2,
  NALU_PRIORITY_HIGH        = 3
} nalref_t;

typedef enum
{
  NALU_TYPE_SLICE           = 1,
  NALU_TYPE_DPA             = 2,
  NALU_TYPE_DPB             = 3,
  NALU_TYPE_DPC             = 4,
  NALU_TYPE_IDR             = 5,
  NALU_TYPE_SEI             = 6,
  NALU_TYPE_SPS             = 7,
  NALU_TYPE_PPS             = 8,
  NALU_TYPE_AUD             = 9,
  NALU_TYPE_EOSEQ           = 10,
  NALU_TYPE_EOSTREAM        = 11,
  NALU_TYPE_FILL            = 12,
  NALU_TYPE_SPS_EXTENSION   = 13,
  NALU_TYPE_PREFIX          = 14,
  NALU_TYPE_SUBSET_SPS      = 15,
  NALU_TYPE_RESERVED_16     = 16,
  NALU_TYPE_RESERVED_17     = 17,
  NALU_TYPE_RESERVED_18     = 18,
  NALU_TYPE_AUX_CODED_SLICE = 19,
  NALU_TYPE_SLICE_SCALABLE  = 20,
  NALU_TYPE_RESERVED_21     = 21,
  NALU_TYPE_RESERVED_22     = 22,
  NALU_TYPE_RESERVED_23     = 23,
#if REF_FRAME_CHECKSUM
  // we're hijacking a unspecified nalu type for debugging purposes
  NALU_TYPE_CHECKSUM        = NALU_TYPE_RESERVED_16
#endif
} nalutype_t;

typedef enum
{
  SLICE_TYPE_P  = 0,
  SLICE_TYPE_B  = 1,
  SLICE_TYPE_I  = 2,
  SLICE_TYPE_SP = 3,
  SLICE_TYPE_SI = 4
} slicetype_t;

/* Values of enums of MB type correspond to tables 7-8 and 7-10 for P
 * slices */
typedef enum
{
  P_SKIP    = -1,
  P_16x16   =  0,
  P_16x8    =  1,
  P_8x16    =  2,
  P_8x8     =  3,
  P_8x8ref0 =  4,
  I_4x4     =  5,
  I_16x16   =  6,
  I_PCM     = 30,
  MIN_MBTYPE = P_SKIP,
  MAX_MBTYPE = I_PCM
} mbtype_t;

/* 4x4 intra prediction modes */
typedef enum {
  VERT_PRED            = 0,
  HOR_PRED             = 1,
  DC_PRED              = 2,
  DIAG_DOWN_LEFT_PRED  = 3,
  DIAG_DOWN_RIGHT_PRED = 4,
  VERT_RIGHT_PRED      = 5,
  HOR_DOWN_PRED        = 6,
  VERT_LEFT_PRED       = 7,
  HOR_UP_PRED          = 8,
  MAX_PRED             = HOR_UP_PRED
} imode4_t;

/* 16x16 intra prediction modes */
typedef enum {
  VERT_PRED_16   =  0,
  HOR_PRED_16    =  1,
  DC_PRED_16     =  2,
  PLANE_PRED_16  =  3
} imode16_t;

/* 8x8 chroma intra prediction modes */
typedef enum {
  DC_PRED_8     =  0,
  HOR_PRED_8    =  1,
  VERT_PRED_8   =  2,
  PLANE_PRED_8  =  3,
  MAX_PRED_8    = PLANE_PRED_8
} imode8_t;


typedef enum
{
  LUMA_4x4,
  LUMA_16x16_DC,
  LUMA_16x16_AC,
  CHROMA_DC,
  CHROMA_AC,
} blocktype_t;




/**
 * Structs
 * **************************************************************************/

typedef struct
{
  int16_t * data;
  int16_t * x;
  int16_t * y;
} fastpos_t;

/* TODO: Must be replaced with mv_t when more than one reference frame is
 * accepted. */
typedef struct
{
  int x;
  int y;
} mvpos;

typedef struct
{
  union
  {
    struct
    {
      int16_t y;
      int16_t x;
    };
    uint32_t vec;
  };

  int ref_num;           /* Should be -1 for blocks coded as intra */
} mv_t;

typedef struct mbinfo_store_s
{
  uint8_t skip; // smart mask 1=skip, 3=skip+skip strength
  /* the flags variable really doesn't have to be an int (not that many flags) */
  int flags;
  uint16_t cbp_blk;  //for deblocking
  uint8_t qp;
  /* uint8_t disable_deblock; */
  uint8_t filter_offset_a;
  uint8_t filter_offset_b;
  mbtype_t mbtype;
  uint8_t prev_qp;

  /*following addded for cabac*/
  int cbp;          //8x8 cbp, format cbp_chroma << 4 | cbp_luma, for cabac use
  imode8_t best_i8x8_mode_chroma;

  int mbx;
  int mby;
  int mbpos;
  int64_t cbp_bits[3];
  int mvd[4][4][2];             //[blky][blkx][y/x]


#ifdef TAA_SAVE_264_MEINFO
  int consumed_bits_num;   //This variable imply bits which are consumed by current marcoblock.
#endif
} mbinfo_store_t;

typedef struct
{
  /* bit 31-16 horizontal edges, 15-0 vertical edges */
  /* order is in raster block order */
  uint32_t do_filter;

  /* each variable is in raster block order */
  /* strength1[i] + strength2[i] * 2 ==  CLIP (0, 2, BoundaryStrength-1) */
  /* the clipping is because the deblocking itself handles strength 3/4 on mb boundaries */
  uint16_t vert_strength1;
  uint16_t vert_strength2;
  uint16_t horz_strength1;
  uint16_t horz_strength2;

  int8_t filter_offset_a;
  int8_t filter_offset_b;

  uint8_t curr_qp;
  uint8_t left_qp;
  uint8_t top_qp;
  uint8_t curr_qpc;
  uint8_t left_qpc;
  uint8_t top_qpc;
} mb_deblock_t;

typedef struct framebuf_t_
{
  uint8_t * data;                       /* pointer to the allocated data */
  uint8_t * y;                          /* pointer to y component */
  uint8_t * u;                          /* pointer to u component */
  uint8_t * v;                          /* pointer to v component */
  int rows;
  int cols;
  int stride;
  int crows;
  int ccols;
  int cstride;
} framebuf_t;


#endif
