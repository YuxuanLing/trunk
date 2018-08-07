#ifndef _TAA_H264_DEC_H_
#define _TAA_H264_DEC_H_

#include "taah264stdtypes.h"

typedef struct taa_h264_decoded_picture_t
{
  const uint8_t * y;
  const uint8_t * u;
  const uint8_t * v;
  unsigned width;
  unsigned height;
  unsigned stride;
  unsigned stride_chroma;
  int is_in_sync;

  unsigned crop_left;
  unsigned crop_right;
  unsigned crop_top;
  unsigned crop_bottom;
  uint16_t par_n;
  uint16_t par_d;

  int is_idr;
  int is_long_term;
  unsigned frame_num;
  unsigned picid;
} taa_h264_decoded_frame_t;

/* Callback used when a frame is finished decoded (complete or not) */
typedef void (*taa_h264_report_frame_complete_func_t)(
  void *                           context,
  const taa_h264_decoded_frame_t * dp);

/* Callback used for passing data received in a flux sei message */
typedef void (*taa_h264_received_sei_user_data_func_t)(
  void *          context,
  unsigned char * data,
  unsigned        len);

/* Callback used for ack/nack of reference frames in flux */
typedef void (*taa_h264_reference_ack_t)(
  void *   context,
  unsigned frame_num,
  int      ack,
  int      idr);

typedef struct taa_h264_dec_callbacks
{
  void * context;
  taa_h264_report_error_func report_error;
  taa_h264_report_frame_complete_func_t report_frame_complete;
  taa_h264_received_sei_user_data_func_t received_sei_user_data;
  taa_h264_reference_ack_t reference_ack;
} taa_h264_dec_callbacks;

typedef struct taa_h264_dec_handle
{
  void * priv;
  taa_h264_dec_callbacks callbacks;
} taa_h264_dec_handle;


typedef struct taa_h264_dec_create_params
{
  unsigned width;
  unsigned height;
  unsigned num_ref_frames;
  taa_h264_dec_callbacks callbacks;
} taa_h264_dec_create_params;


typedef struct taa_h264_dec_exec_params
{
  unsigned char * nalu_buffer;
  unsigned nalu_size;
} taa_h264_dec_exec_params;


taa_h264_dec_handle * taa_h264_dec_create (
  taa_h264_dec_create_params * params);

void taa_h264_dec_destroy (
  taa_h264_dec_handle * dec);

int taa_h264_dec_execute (
  taa_h264_dec_handle *      dec,
  taa_h264_dec_exec_params * params,
  int *                      happy);

#endif
