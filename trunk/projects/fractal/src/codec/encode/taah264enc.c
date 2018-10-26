#include "taah264enc.h"
#include "enc.h"

#include <stdlib.h>
#include <stdio.h>

static void taa_h264_enc_set_callbacks (
  encoder_t *              enc_priv,
  taa_h264_enc_callbacks * callbacks)
{
  enc_priv->callbacks = *callbacks;

  enc_priv->error_handler.func = callbacks->report_error;
  enc_priv->error_handler.context = callbacks->context;
}


taa_h264_enc_handle * taa_h264_enc_create (
  taa_h264_enc_create_params * params)
{
  encoder_t * enc_priv;
  taa_h264_enc_handle * enc;

  enc = TAA_H264_MALLOC (sizeof (taa_h264_enc_handle));
  enc_priv = TAA_H264_MALLOC (sizeof (encoder_t));

  if (enc_priv == NULL || enc == NULL)
  {
    TAA_H264_FREE (enc);
    TAA_H264_FREE (enc_priv);
    return NULL;
  }

  enc->priv = enc_priv;
  enc->callbacks = params->callbacks;
  taa_h264_enc_set_callbacks (enc_priv, &enc->callbacks);

  if (!taa_h264_enc_init (enc_priv, params->width, params->height, params->par_n,
                          params->par_d, params->num_ref_frames,
                          params->temporal_levels, params->profile_level
#ifdef TAA_SAVE_264_MEINFO
                          ,params->debug_flag
                          ,params->debug_keyword
                          ,params->debug_detail_flag
                          ,params->debug_ip_addr
                          ,params->debug_port
#endif
#ifdef TAA_DEBUG_READ_YUV_FILE
                          ,params->used_yuvfile_flag
                          ,params->fp
                          ,params->total_frame
                          ,params->yuv_w
                          ,params->yuv_h
#endif
                          ))
  {
    TAA_H264_FREE (enc);
    TAA_H264_FREE (enc_priv);
    return NULL;
  }

  return enc;
}


void taa_h264_enc_destroy (
  taa_h264_enc_handle * encoder)
{

  taa_h264_enc_free ((encoder_t *) encoder->priv);
  TAA_H264_FREE (encoder->priv);
  TAA_H264_FREE (encoder);
}


int taa_h264_enc_execute (
  taa_h264_enc_handle *      enc,
  taa_h264_enc_exec_params * params)
{
  encoder_t * priv = (encoder_t *) enc->priv;

  taa_h264_enc_set_callbacks (priv, &enc->callbacks);

  return taa_h264_process_frame (priv,
                                 params->raw_buffer,
                                 params->input_width,
                                 params->input_height,
                                 params->intra_period,
                                 (uint8_t) params->gop_size,
                                 params->force_idr,
                                 (uint8_t) params->min_qp,
                                 (uint8_t) params->max_qp,
                                 (uint8_t) params->max_qp_intra,
                                 (uint8_t) params->qp,
                                 (uint16_t) params->bitrate,
                                 (uint8_t) params->framerate,
                                 params->max_mbps,
                                 params->max_static_mbps,
                                 params->enable_frame_skip,
                                 params->active_temporal_levels,
                                 params->flux,
                                 params->max_nalu_size,
                                 params->coding_options,
	                             params->use_high_profile,
                                 params->output_buffer);
}


void taa_h264_enc_default_create_params (
  taa_h264_enc_create_params * params)
{
  params->width = 352;
  params->height = 288;
  params->par_n = 0;
  params->par_d = 0;
  params->num_ref_frames = 1;
  params->temporal_levels = 1;
  params->profile_level = LEVEL_4_0;
  params->super_p_period = 0;
  params->long_term_period = 0;
  const taa_h264_enc_callbacks c = {0, };
  params->callbacks = c;
}

void taa_h264_enc_default_exec_params (
  taa_h264_enc_exec_params * params)
{
  params->raw_buffer = NULL;
  params->input_width = 0;
  params->input_height = 0;
  params->intra_period = 0;
  params->gop_size = 1;
  params->active_temporal_levels = -1;
  params->force_idr = false;
  params->coding_options = 0;
  params->min_qp = 23;
  params->max_qp = 43;
  params->max_qp_intra = TAA_H264_QP_NOT_SPECIFIED;
  params->qp = TAA_H264_QP_NOT_SPECIFIED;
  params->bitrate = 768;
  params->framerate = 30;
  params->max_mbps = 0;
  params->max_static_mbps = 0;
  params->enable_frame_skip = false;
  params->max_nalu_size = 1344;
  params->use_high_profile = 0;
  params->output_buffer = NULL;
  params->flux = NULL;
}
