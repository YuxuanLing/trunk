#include "taah264dec.h"
#include "dec.h"

#include <stdlib.h>

static void taa_h264_dec_set_callbacks (
  decoder_t *              dec_priv,
  taa_h264_dec_callbacks * callbacks)
{
  dec_priv->callbacks = *callbacks;

  dec_priv->error_handler.func = callbacks->report_error;
  dec_priv->error_handler.context = callbacks->context;
}


taa_h264_dec_handle * taa_h264_dec_create (
  taa_h264_dec_create_params * params)
{
  decoder_t * dec_priv;
  taa_h264_dec_handle * dec;

  dec_priv = TAA_H264_MALLOC (sizeof (decoder_t));
  dec = TAA_H264_MALLOC (sizeof (taa_h264_dec_handle));

  if (dec_priv == NULL || dec == NULL)
  {
    TAA_H264_FREE (dec_priv);
    TAA_H264_FREE (dec);
    return NULL;
  }

  dec->priv = dec_priv;
  dec->callbacks = params->callbacks;
  taa_h264_dec_set_callbacks (dec_priv, &dec->callbacks);
  if (!taa_h264_dec_init (dec_priv, params->width, params->height, params->num_ref_frames))
  {
    TAA_H264_FREE (dec_priv);
    TAA_H264_FREE (dec);
    return NULL;
  }

  return dec;
}


void taa_h264_dec_destroy (
  taa_h264_dec_handle * dec)
{
  taa_h264_dec_free ((decoder_t *) dec->priv);
  TAA_H264_FREE (dec->priv);
  TAA_H264_FREE (dec);
}


int taa_h264_dec_execute (
  taa_h264_dec_handle *      dec,
  taa_h264_dec_exec_params * params,
  int *                      happy)
{
  decoder_t * priv = (decoder_t *) dec->priv;

  /* In case the callback functions/context have been changed during runtime. */
  taa_h264_dec_set_callbacks (priv, &dec->callbacks);

  bool nalu_ok = taa_h264_process_nalu (
    priv, params->nalu_buffer, params->nalu_size);
  *happy = priv->happy;

  return nalu_ok;
}
