#ifndef _TAA_H264_ENC_H_
#define _TAA_H264_ENC_H_

#include "taah264stdtypes.h"
#include <stdio.h>

// When you want to do this, you can define this macro "TAA_SAVE_264_MEINFO", or please don't define it.
//#define TAA_SAVE_264_MEINFO   // Used to write h264 meta information into a file or socket tunle.
//#define TAA_DEBUG_READ_YUV_FILE  //Used to read yuv data from file instead of camera.

/* Return flags */
#define TAA_H264_OK             (1 << 0)
#define TAA_H264_FRAME_SKIPPED  (1 << 1)

/* Coding options to turn on/off algorithms */
#define TAA_H264_LQ_DISABLE_HIGH_INTRA_4x4_MODES (1 << 0)
#define TAA_H264_HQ_DECREASED_THRESHOLD          (1 << 1)
#define TAA_H264_HQ_DISABLE_EARLY_SKIP_MV_SEARCH (1 << 2)
#define TAA_H264_XQ_DISABLE_EARLY_SKIP           (1 << 3)


#define TAA_H264_QP_NOT_SPECIFIED 52

typedef bool (*taa_h264_output_nalu_func)(unsigned char * buf,
                                          size_t          buf_size,
                                          unsigned char * total_buf,
                                          size_t          total_buf_size,
                                          unsigned int    lt_idx,
                                          unsigned int    frame_num,
                                          bool            last_nalu_in_frame,
                                          void *          context);

typedef struct taa_h264_enc_callbacks
{
  void * context;
  taa_h264_report_error_func report_error;
  taa_h264_output_nalu_func output_nalu;
} taa_h264_enc_callbacks;


typedef struct taa_h264_enc_handle
{
  void * priv;
  taa_h264_enc_callbacks callbacks;
} taa_h264_enc_handle;


typedef struct taa_h264_enc_create_params
{
  unsigned width;
  unsigned height;
  unsigned short par_n;      /* nominator   in fraction describing pixel aspect ratio */
  unsigned short par_d;      /* denominator in fraction describing pixel aspect ratio */
  unsigned num_ref_frames;
  unsigned temporal_levels;  /* number of temporal levels (aka temporal layers) */
  unsigned profile_level;    /* the signaled h.264 level (level 3.0 = 30) */
  unsigned super_p_period;   /* specifies the periodicity of super p frames (0 = no periodic super p) */
  unsigned long_term_period; /* specifies the periodicity of LT frames (0 = adaptive LT period) */
  taa_h264_enc_callbacks callbacks;
  unsigned char debug_flag;
  char debug_keyword[64];    /* specifies the keyword string from which you can general any uniqual string for debuging*/
  unsigned char debug_detail_flag; /* If this flag is true, some detial flag will be written in h264 meta file.*/
  char debug_ip_addr[32];
  unsigned int debug_port;

#ifdef TAA_DEBUG_READ_YUV_FILE
  unsigned used_yuvfile_flag;
  char debug_yuv_filename[64];
  FILE *fp;
  unsigned total_frame;
  unsigned yuv_w;
  unsigned yuv_h;
#endif

} taa_h264_enc_create_params;


typedef struct taa_h264_flux_params
{
  bool recover;            /* whether there is an error that should be recovered ("super p request")*/
  bool long_term;          /* whether this frame should be stored as long term */
  bool ref_repetition;     /* whether there should be sent a ref_pic_marking_repetition SEI for current frame */
  int  * ack_status;       /* array of equal length to long term buffer, holding ack status of long term indices */
  unsigned char * message; /* flux message to send to receiver */
  unsigned message_len;    /* length of the message to send (in bytes) */
} taa_h264_flux_params;


typedef struct taa_h264_enc_exec_params
{
  unsigned char * raw_buffer;
  unsigned input_width;  /* Width of input buffer. 0 means same as configured output. */
  unsigned input_height; /* Height of input buffer. 0 means same as configured output. */
  bool force_idr;              /* IDR according to H.241 standard. Not implemented */
  unsigned intra_period;       /* Rate of intra frames, 0 = disable */
  unsigned coding_options;     /* Flags to enable/disable coding algorithms */

  unsigned min_qp;    /* min qp when adjusting qp with rate control */
  unsigned max_qp;    /* max qp when adjusting qp with rate control */
  unsigned max_qp_intra;  /* max qp when adjusting qp with rate control on intra frames.
                           * If value is TAA_H264_QP_NOT_SPECIFIED max_qp is used also for intra frames. */
  unsigned qp;        /* When qp != TAA_H264_QP_NOT_SPECIFIED it
                       * overrides the target_bitrate */

  bool enable_frame_skip;     /* whether skipping frames for rate control is allowed */
  unsigned bitrate;           /* target bitrate for rate control */
  unsigned framerate;         /* framerate is used for rate control calculation only */
  unsigned max_mbps;          /* max number of MBs per sec, 0 = disable */
  unsigned max_static_mbps;   /* max number of static MBs per sec, 0 = disable */

  unsigned gop_size;          /* "group of pictures" size */
  int active_temporal_levels; /* number of active temporal levels (higher levels will be skipped). negative number disables check. */

  unsigned max_nalu_size;
  unsigned use_high_profile;          /* = 1 means using cabac */
  unsigned char * output_buffer;  /* Must be at least one MB bigger than max_nalu_size. */
  

  taa_h264_flux_params * flux; /* information needed for packet loss scheme */
} taa_h264_enc_exec_params;


taa_h264_enc_handle * taa_h264_enc_create (
  taa_h264_enc_create_params * params);

void taa_h264_enc_destroy (
  taa_h264_enc_handle * encoder);

int taa_h264_enc_execute (
  taa_h264_enc_handle *      encoder,
  taa_h264_enc_exec_params * params);

void taa_h264_enc_default_create_params (
  taa_h264_enc_create_params * params);

void taa_h264_enc_default_exec_params (
  taa_h264_enc_exec_params * params);


enum
{
  FLUX_NACKED = -1,
  FLUX_UNKNOWN_ACK_STATE,
  FLUX_ACKED
};

#endif
