#ifndef _COM_DPB_H_
#define _COM_DPB_H_

#include "com_common.h"
#include "com_error.h"
#include "com_typedefs.h"

#define MAX_NUM_REF_FRAMES 16
#define MAX_NUM_DPB_PICS (MAX_NUM_REF_FRAMES + 1)
#define MAX_NUM_RPLR_CMDS MAX_NUM_REF_FRAMES
/* can't see MAX_NUM_MMCO_CMDS defined in the standard.
 * theoretically infinite? */
#define MAX_NUM_MMCO_CMDS 16
#define TAA_H264_DPB_NO_MATCH 255

/* Structure for storing interpolated and padded frames. */
struct frameipbuf_s
{
  framebuf_t full;
  framebuf_t half_diag;
  framebuf_t half_horz;
  framebuf_t half_vert;
};


struct decoded_picture_s
{
  frameipbuf_t ipbuf;
  bool is_used;
  bool is_longterm;
  /* picid is picnum for ST, long term frame index for LT */
  int picid;
  void * meta;
  int original_frame_num;
# if REF_FRAME_CHECKSUM
  uint32_t checksum;
# endif
};

struct decoded_picture_buffer_s
{
  decoded_picture_t pictures[MAX_NUM_DPB_PICS];   // index is in range [ 0, max_ref_frames+1 >
  decoded_picture_t * ref_list[MAX_NUM_DPB_PICS]; // index is in range [ 0, total_ref_frames >
  uint8_t max_ref_frames;
  uint8_t total_ref_frames;
  uint8_t max_lt_frames;
  uint8_t num_lt_frames;
  uint8_t curr_idx;
  uint8_t output_idx;
  uint8_t conceal_idx;
  uint8_t * mem_base;
  error_handler_t * error_handler;
  int max_width;
  int max_height;
  int last_seen_frame_num;
  int max_frame_num;
};


struct rplr_s
{
  int idc;
  int val;
};


struct mmco_s
{
  int op;
  int val1;
  int val2;
};


bool taa_h264_dpb_init (
  decoded_picture_buffer_t * dpb,
  int                        num_ref_frames,
  int                        width,
  int                        height,
  int                        max_frame_num,
  bool                       dpb_has_interpolated,
  error_handler_t *          error_handler);

void taa_h264_dpb_reset (
  decoded_picture_buffer_t * dpb);

void taa_h264_dpb_update_picnums (
  decoded_picture_buffer_t * dpb,
  int                        frame_num);

bool taa_h264_dpb_update (
  decoded_picture_buffer_t * dpb,
  int *                      frame_num,
  bool                       is_ref,
  bool                       is_idr,
  bool                       is_idr_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco);

int taa_h264_dpb_set_ref_list (
  decoded_picture_buffer_t * dpb,
  bool                       reordering_flag,
  const rplr_t *             rplr,
  int                        frame_num,
  frameipbuf_t * *           ref_buffers,
  void * *                   ref_metas);

void taa_h264_unpad_framebuf (
  framebuf_t * buf);

void taa_h264_dpb_get_current_framebuf (
  decoded_picture_buffer_t * dpb,
  framebuf_t *               current);

# if REF_FRAME_CHECKSUM
void taa_h264_dpb_set_checksum (
  decoded_picture_buffer_t * dpb);
# endif

/* Functions that should not be used by others, except enc_dpb.c and dec_dpb.c */
bool taa_h264_dpb_set_short_term_unused (
  decoded_picture_buffer_t * dpb,
  int                        picnum);

bool taa_h264_dpb_set_long_term_unused (
  decoded_picture_buffer_t * dpb,
  int                        lt_index);

void taa_h264_dpb_set_long_term (
  decoded_picture_buffer_t * dpb,
  decoded_picture_t *        pic,
  int                        lt_index);

decoded_picture_t * taa_h264_dpb_find_oldest_short_term (
  decoded_picture_buffer_t * dpb);

decoded_picture_t * taa_h264_dpb_find_picture (
  decoded_picture_buffer_t * dpb,
  int                        picid,
  bool                       longterm);


#endif
