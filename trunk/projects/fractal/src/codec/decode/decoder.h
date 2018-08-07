#ifndef _DECODER_H_
#define _DECODER_H_

#include "taah264dec.h"
#include "dec_typedefs.h"
#include "com_common.h"
#include "com_cpuinfo.h"
#include "com_resolutions.h"
#include "dec_dpb.h"

/* Configuration */
#define OUTPUT_CORRUPT_IDR_SEQUENCE 0

#define NUM_MVS_MB  16   /* Number of MVs stored per MB */


struct mbinfo_s
{
  int mby;
  int mbx;
  int mbpos;
  int mbxmax;

  mv_t max_mv_top_left;
  mv_t max_mv_bottom_right;

  int mquant;
  unsigned cbp;
  int cbp_4x4;
  int avail_flags;
  mbtype_t mbtype;
  bool all_P_8x8;
  imode8_t intra_mode_chroma;
  imode16_t intra_16x16_mode;

  uint8_t chroma_qp_index_offset;
};

struct poc_s
{
  int type;
  int lsb;
  int delta_bottom;
  int delta_0;
  int delta_1;
};

#define IDR_PIC_ID_NOT_AVAILABLE (-1)
struct sliceinfod_s
{
  bool reordering_flag;
  rplr_t rplr[MAX_NUM_RPLR_CMDS];

  bool long_term_ref_flag;
  bool adaptive_marking_flag;
  mmco_t mmco[MAX_NUM_MMCO_CMDS];

  poc_t poc;
  nalref_t nalref_idc;
  nalutype_t nalutype;
  uint8_t pic_parset_id;
  int16_t idr_pic_id;
  bool islice;
  int first_mb;
  int frame_num;
  int qp;
  uint8_t num_ref_idx_l0_active;
  uint8_t disable_deblock;
  uint8_t filter_offset_a;
  uint8_t filter_offset_b;
};



struct bitreader_s
{
  uint32_t bitbuf[2];
  uint8_t * bufptr;
  uint8_t * bufend;
  int32_t bitsleft;
  int bitrest;
};



struct frameinfod_s
{
  int width;
  int height;
  source_res_t resolution;
  int frame_num;
  int max_frame_num;
  uint8_t num_active_ref_frames;
  bool is_ref;
  bool is_idr;
  bool constrained_intra_pred;
  bool all_mbs_decoded;
  unsigned num_mbs_decoded;
  unsigned num_mbs_intra;
  bool ref_in_sync;
  bool cancel;
  bool maybe_ghost_frame;

  bool is_idr_long_term;
  bool adaptive_marking_flag;
  const mmco_t * mmco;

  framebuf_t * ref_buffers[MAX_NUM_REF_FRAMES];
  framebuf_t curr;                      /* Current decoded picture, pointing
                                         * into the above frame_buffers */

  uint8_t * intra_modes;               /* Intra modes for all 4x4 blocks */
  mv_t * motion_vectors;                /* MVs for all 4x4 blocks */
  uint8_t * num_coeffs_y;              /* number of non zero coeffs for the 4x4 blocks */
  uint8_t * num_coeffs_uv;
  mbinfo_store_t * mbinfo_stored;

  mb_deblock_t * mb_deblock_info;
  unsigned mb_deblock_info_size;

  /* Some SPS info needs to be stored on a per frame basis since we might need
   * to output the previous frame after having activated a new SPS (in the case
   * of packet loss). */
  unsigned crop_left;
  unsigned crop_right;
  unsigned crop_top;
  unsigned crop_bottom;
  uint16_t sar_width;
  uint16_t sar_height;

  uint8_t chroma_qp_index_offset;
};


struct seq_parset_s
{
  int width_mbs;
  int height_mbs;
  int log2_max_frame_num;
  int log2_max_pic_order_cnt_lsb;
  bool gaps_in_frame_num_allowed;
  uint8_t num_ref_frames;
  uint8_t poc_type;
  bool poc_delta_always_zero;
  bool is_valid;

  unsigned crop_left;
  unsigned crop_right;
  unsigned crop_top;
  unsigned crop_bottom;
  uint16_t sar_width;
  uint16_t sar_height;
};

struct pic_parset_s
{
  uint8_t seq_parset_id;
  uint8_t init_qp;
  uint8_t chroma_qp_index_offset;
  uint8_t num_ref_idx_l0_active;
  bool deblocking_control_present;
  bool pic_order_present;
  bool constrained_intra_pred_flag;
  bool is_valid;
};

#define MAX_NUM_PIC_PARSETS 256
#define MAX_NUM_SEQ_PARSETS 32

struct sequenced_s
{
  pic_parset_t pic_parsets[MAX_NUM_PIC_PARSETS];
  seq_parset_t seq_parsets[MAX_NUM_SEQ_PARSETS];
  seq_parset_t active_sps;

  sliceinfo_t curr_slice;
  sliceinfo_t prev_slice;

  bool have_complete_iframe;
  bool have_gap;
  bool first_slice;
};

struct decoder_s
{
  sequence_t sequence;
  decoded_picture_buffer_t dpb;
  ref_meta_t refframes_meta[MAX_NUM_DPB_PICS];
  frameinfo_t frameinfo;
  error_handler_t error_handler;
  taa_h264_dec_callbacks callbacks;
  cpuinfo_t cpuinfo;
  bool happy;
  bool dont_decode;
};

#endif
