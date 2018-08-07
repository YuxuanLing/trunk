#ifndef _ENC_TYPEDEFS_H_
#define _ENC_TYPEDEFS_H_

#include "com_typedefs.h"
#include "taah264enc.h"

#define MAX_PREVIOUS_FRMAE_INFO_NUM 8     /* This means the array previous_frame_length has 8 elements. */
                                         /* This value must be equal 1<<k, where k is a integer.       */
struct ratecontrol_s
{
  uint8_t min_qp;
  uint8_t max_qp;
  uint8_t framerate;
  uint16_t bitrate;

  uint8_t * prev_frame_mb_qp;             /* Quant parameter used for each MB in previous frame */
  uint32_t * prev_frame_mb_bits;          /* Number of bits used for each MB in previous frame */
  uint32_t bits_sliding_frame;            /* Number of bits used for sliding frame */
  uint32_t bits_quant_sliding_frame;      /* bits * quant for last sliding frame */
  uint32_t bits_this_frame;               /* Number of bits used for current frame */
  uint32_t bits_quant_this_frame;         /* bits * quant for current frame */
  int32_t target_bits;                    /* Target number of bits for this frame */
  int32_t overprod_bits;                  /* Overproduced bits for last sliding frame */
  int32_t quant_average;                  /* bitQuantsum/bitsLastSlidingFrame */
  int64_t corr_factor;                    /* Over production ratio last frame * 2^16 */

  /* Special ratecontrol to handle legacy decoders */
  uint32_t num_mbs;                       /* Number of MBs in a frame (frame size) */
  uint32_t max_mbps;                      /* Max number of MBs per sec */
  uint32_t max_static_mbps;               /* Max number of skip MBs per sec*/

  // For optimize the frame skipping algorithm
  int32_t previous_frame_length[MAX_PREVIOUS_FRMAE_INFO_NUM];
  int32_t previous_frame_pos;             /* This variable is used to point the oldest frame in previsous_frame_length, which will be */
                                          /* replaced by current frame. */
  int32_t pre_frame_length_sum;
  int32_t length_compensate_ratio;        /* This variable is used to compensate for current frame langth counting. */
                                          /* If this value is 1, that measn 1/10, and so on. So the max value of it won't exceed 10. */

};

typedef struct luma_s               luma_t;
typedef struct chroma_s             chroma_t;
typedef struct coeffs4_s            coeffs4_t;
typedef struct coeffs2_s            coeffs2_t;
typedef struct mbcoeffs_s           mbcoeffs_t;
typedef struct mbinfo_s             mbinfo_t;
typedef struct frameinfo_s          frameinfo_t;
typedef struct sequence_s           sequence_t;
typedef struct encoder_s            encoder_t;
typedef struct gop_s                gop_t;
typedef struct meinfo_s             meinfo_t;
typedef struct ref_control_s        ref_control_t;
typedef struct bitwriter_s          bitwriter_t;
typedef struct ref_meta_enc_s       ref_meta_t;
typedef struct rpm_s                rpm_t;
typedef struct control_s            control_t;
typedef struct ratecontrol_s        ratecontrol_t;
typedef struct resizer_s            resizer_t;
typedef struct taa_h264_flux_params flux_t;
typedef taa_h264_output_nalu_func   nal_cb_t;




#endif
