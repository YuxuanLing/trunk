#include <stdlib.h>
#include <string.h>
#include "dec.h"
#include "dec_readbits.h"
#include "dec_getvlc.h"
#include "dec_transform.h"
#include "dec_quant.h"
#include "dec_headers.h"
#include "dec_sei.h"
#include "dec_intrapred.h"
#include "com_prediction.h"
#include "com_mbutils.h"
#include "com_intmath.h"
#include "com_deblock.h"
#include "com_error.h"
#include "com_interpolation.h"
#include "com_interpolation_ref.h"


static void taa_h264_init_mb (
  frameinfo_t * frameinfo,
  mbinfo_t *    mb,
  int           mby,
  int           mbx,
  int           first_mb_in_slice)
{
  int mbxmax = frameinfo->width / MB_WIDTH_Y;
  int mbymax = frameinfo->height / MB_HEIGHT_Y;
  int xpos = mbx * MB_WIDTH_Y;
  int ypos = mby * MB_HEIGHT_Y;

  mb->mby = mby;
  mb->mbx = mbx;
  mb->mbpos = mby * mbxmax + mbx;
  mb->mbxmax = mbxmax;

  /* We don't care about max mv's according to H.264 Level. This is for not
   * using a block outside the padded picture as reference. */
  mb->max_mv_top_left.x = (int16_t)((-xpos - PAD_HORZ_Y + 2) * 4);
  mb->max_mv_top_left.y = (int16_t)((-ypos - PAD_VERT_Y + 2) * 4);
  mb->max_mv_bottom_right.x = (int16_t)((frameinfo->width + PAD_HORZ_Y - xpos - MB_WIDTH_Y - 3) * 4);
  mb->max_mv_bottom_right.y = (int16_t)((frameinfo->height + PAD_VERT_Y - ypos - MB_HEIGHT_Y - 3) * 4);

  taa_h264_set_mb_availability (
    mbx,
    mby,
    mbxmax,
    mbymax,
    first_mb_in_slice,
    &frameinfo->mbinfo_stored[mb->mbpos],
    frameinfo->constrained_intra_pred,
    &mb->avail_flags);
  mb->avail_flags = MB_SET_DECODED (mb->avail_flags);

  mb->chroma_qp_index_offset = frameinfo->chroma_qp_index_offset;
}


static void taa_h264_init_frame (
  frameinfo_t *              frameinfo,
  const sliceinfo_t *        sliceinfo,
  const pic_parset_t *       pps,
  const seq_parset_t *       sps,
  decoded_picture_buffer_t * dpb,
  const bool                 force_out_of_sync,
  const bool                 maybe_ghost_frame)
{

#ifdef TAA_H264_TRACE_ENABLED
  static unsigned frame_counter = -1;
  frame_counter++;
  TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "frame_counter %04u\n", frame_counter);
#endif

  frameinfo->cancel = false;
  frameinfo->frame_num = sliceinfo->frame_num;
  frameinfo->max_frame_num = 1 << sps->log2_max_frame_num;
  frameinfo->num_active_ref_frames = sliceinfo->num_ref_idx_l0_active;
  frameinfo->is_ref = (sliceinfo->nalref_idc != NALU_PRIORITY_DISPOSABLE);
  frameinfo->is_idr = (sliceinfo->nalutype == NALU_TYPE_IDR);
  frameinfo->is_idr_long_term = sliceinfo->long_term_ref_flag;
  frameinfo->constrained_intra_pred = pps->constrained_intra_pred_flag;
  frameinfo->all_mbs_decoded = false;
  frameinfo->num_mbs_decoded = 0;
  frameinfo->num_mbs_intra = 0;
  frameinfo->ref_in_sync = !force_out_of_sync;
  frameinfo->maybe_ghost_frame = maybe_ghost_frame;

  frameinfo->adaptive_marking_flag = sliceinfo->adaptive_marking_flag;
  frameinfo->mmco = sliceinfo->mmco;

  frameinfo->crop_left = sps->crop_left * 2;
  frameinfo->crop_right = sps->crop_right * 2;
  frameinfo->crop_top = sps->crop_top * 2;
  frameinfo->crop_bottom = sps->crop_bottom * 2;
  frameinfo->sar_width = sps->sar_width;
  frameinfo->sar_height = sps->sar_height;

  frameinfo->chroma_qp_index_offset = pps->chroma_qp_index_offset;

  if (sliceinfo->islice == false)
  {
    /* TODO: DPB - Rewrite decoder to properly use frameipbuf_t */
    frameipbuf_t * ref_buffers_tmp[MAX_NUM_REF_FRAMES];
    int res = taa_h264_dpb_set_ref_list (
      dpb,
      sliceinfo->reordering_flag,
      sliceinfo->rplr,
      frameinfo->frame_num,
      ref_buffers_tmp, NULL);
    if (res < 0)
      frameinfo->cancel = true;

    /* Workaround */
    for (int i = 0; i < frameinfo->num_active_ref_frames; i++)
      frameinfo->ref_buffers[i] = &ref_buffers_tmp[i]->full;
  }

  taa_h264_dpb_get_current_framebuf (dpb, &frameinfo->curr);
  taa_h264_unpad_framebuf (&frameinfo->curr);

  /* Initialize misc arrays */
  const int num_mbs = frameinfo->width * frameinfo->height / MB_SIZE_Y;
  memset (frameinfo->num_coeffs_y, 0, num_mbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t));
  memset (frameinfo->num_coeffs_uv, 0, num_mbs * NUM_4x4_BLOCKS_UV * 2 * sizeof (uint8_t));
  memset (frameinfo->intra_modes, DC_PRED, num_mbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t));
  memset (frameinfo->mbinfo_stored, 0, num_mbs * sizeof (mbinfo_store_t));
}

static bool taa_h264_frameinfo_create (
  frameinfo_t *     frameinfo,
  const int         max_width,
  const int         max_height,
  const int         max_num_ref_frames,
  error_handler_t * eh,
  const bool        update_frameinfo)
{
  error_codes_t err = TAA_H264_NO_ERROR;
  const int max_mbs = max_width * max_height / MB_SIZE_Y;

  if (update_frameinfo)
  {
    frameinfo->width = max_width;
    frameinfo->height = max_height;
    frameinfo->resolution = RES_UNKNOWN;
    frameinfo->mb_deblock_info = NULL;
    frameinfo->mb_deblock_info_size = 0;
  }
  frameinfo->frame_num = -1;
  frameinfo->all_mbs_decoded = true;

  /* Allocate memory for misc arrays */
  frameinfo->num_coeffs_y = NULL;
  frameinfo->num_coeffs_uv = NULL;
  frameinfo->intra_modes = NULL;
  frameinfo->motion_vectors = NULL;
  frameinfo->mbinfo_stored = NULL;

  if (!(frameinfo->num_coeffs_y = TAA_H264_MALLOC (max_mbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_Y;
    goto alloc_failed;
  }

  if (!(frameinfo->num_coeffs_uv = TAA_H264_MALLOC (max_mbs * NUM_4x4_BLOCKS_UV * 2 * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_UV;
    goto alloc_failed;
  }

  if (!(frameinfo->intra_modes = TAA_H264_MALLOC (max_mbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_INTRA_MODES;
    goto alloc_failed;
  }

  if (!(frameinfo->motion_vectors = TAA_H264_MALLOC (max_mbs * NUM_MVS_MB * sizeof(mv_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS;
    goto alloc_failed;
  }

  if (!(frameinfo->mbinfo_stored = TAA_H264_MALLOC (max_mbs * sizeof(mbinfo_store_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MBINFO_STORED;
    goto alloc_failed;
  }

  if (frameinfo->mb_deblock_info_size < (frameinfo->width / MB_WIDTH_Y) * (frameinfo->height / MB_HEIGHT_Y))
  {
    TAA_H264_FREE (frameinfo->mb_deblock_info);

    frameinfo->mb_deblock_info_size = (frameinfo->width / MB_WIDTH_Y) * (frameinfo->height / MB_HEIGHT_Y);
    if (!(frameinfo->mb_deblock_info = TAA_H264_MALLOC (frameinfo->mb_deblock_info_size * sizeof(mb_deblock_t))))
    {
      err = TAA_H264_ALLOCATION_ERROR_MB_DEBLOCK_INFO;
      goto alloc_failed;
    }
  }

  return true;

alloc_failed:
  TAA_H264_CRITICAL (eh, err);

  TAA_H264_FREE (frameinfo->num_coeffs_y);
  TAA_H264_FREE (frameinfo->num_coeffs_uv);
  TAA_H264_FREE (frameinfo->intra_modes);
  TAA_H264_FREE (frameinfo->motion_vectors);
  TAA_H264_FREE (frameinfo->mbinfo_stored);
  TAA_H264_FREE (frameinfo->mb_deblock_info);

  frameinfo->mb_deblock_info_size = 0;

  return false;
}

bool taa_h264_dec_init (
  decoder_t * decoder,
  int         max_width,
  int         max_height,
  int         max_num_ref_frames)
{
  error_codes_t err = TAA_H264_NO_ERROR;
  sequence_t * sequence = &decoder->sequence;

  taa_h264_get_cpuinfo (&decoder->cpuinfo);

  decoder->happy = true;
  decoder->dont_decode = true;

  sequence->first_slice = true;
  sequence->curr_slice.frame_num = -1;
  sequence->curr_slice.pic_parset_id = 0;
  sequence->curr_slice.nalref_idc = NALU_PRIORITY_DISPOSABLE;
  sequence->curr_slice.idr_pic_id = 0;
  sequence->prev_slice = sequence->curr_slice;
  sequence->have_complete_iframe = false;
  sequence->have_gap = false;
  sequence->active_sps.is_valid = false;

  for (int i = 0; i < MAX_NUM_SEQ_PARSETS; i++)
    sequence->seq_parsets[i].is_valid = false;
  for (int i = 0; i < MAX_NUM_PIC_PARSETS; i++)
    sequence->pic_parsets[i].is_valid = false;

  if (max_width>0 && max_height>0 && max_num_ref_frames>0)
  {
    int padded_width = max_width  + PAD_HORZ_Y  * 2;
    int padded_height = max_height + PAD_VERT_Y * 2;
    int buf_size_one = (padded_width * padded_height * 3 / 2) * sizeof (uint8_t);
    int num_dpb_pics = max_num_ref_frames + 1; // + 1 for the current decoded frame.
    int mem_size = buf_size_one * num_dpb_pics;

    decoder->dpb.max_width  = max_width;
    decoder->dpb.max_height = max_height;
    decoder->dpb.max_ref_frames = (uint8_t)max_num_ref_frames;
    decoder->dpb.mem_base = TAA_H264_MM_MALLOC (mem_size, 64);
    if (!decoder->dpb.mem_base)
    {
      err = TAA_H264_ALLOCATION_ERROR_DPB_MEM_BASE;
      goto alloc_failed;
    }

    if (!taa_h264_frameinfo_create (&decoder->frameinfo,
                                     max_width,
                                     max_height,
                                     max_num_ref_frames,
                                    &decoder->error_handler,
                                    true))
    {
      err = TAA_H264_ALLOCATION_ERROR_FRAMEINFO_CREATE;
      goto alloc_failed;
    }
  }
  else
  {
    decoder->dpb.max_width      = 0;
    decoder->dpb.max_height     = 0;
    decoder->dpb.max_ref_frames = 0;
    decoder->dpb.mem_base       = 0;
    decoder->frameinfo.num_coeffs_y   = NULL;
    decoder->frameinfo.num_coeffs_uv  = NULL;
    decoder->frameinfo.intra_modes    = NULL;
    decoder->frameinfo.motion_vectors = NULL;
    decoder->frameinfo.mbinfo_stored  = NULL;
    decoder->frameinfo.mb_deblock_info = NULL;
    decoder->frameinfo.mb_deblock_info_size = 0;
  }

  return true;

alloc_failed:
  TAA_H264_CRITICAL (&decoder->error_handler, err);
  TAA_H264_MM_FREE (decoder->dpb.mem_base);
  // frameinfo cleans up after itself
  return false;
}

void taa_h264_dec_free (
  decoder_t * decoder)
{
  frameinfo_t * frameinfo = &decoder->frameinfo;

  TAA_H264_MM_FREE (decoder->dpb.mem_base);
  TAA_H264_FREE (frameinfo->intra_modes);
  TAA_H264_FREE (frameinfo->num_coeffs_y);
  TAA_H264_FREE (frameinfo->num_coeffs_uv);
  TAA_H264_FREE (frameinfo->motion_vectors);
  TAA_H264_FREE (frameinfo->mbinfo_stored);
  TAA_H264_FREE (frameinfo->mb_deblock_info);

  decoder->dpb.mem_base     = NULL;
  frameinfo->num_coeffs_y   = NULL;
  frameinfo->num_coeffs_uv  = NULL;
  frameinfo->intra_modes    = NULL;
  frameinfo->motion_vectors = NULL;
  frameinfo->mbinfo_stored  = NULL;
  frameinfo->mb_deblock_info = NULL;

  frameinfo->mb_deblock_info_size = 0;
}


static void taa_h264_recon_skipped(
  mbinfo_t *   mb,
  framebuf_t * curr,
  framebuf_t * pad_prev,
  mv_t         pred)
{
  const int ypos = mb->mby * MB_HEIGHT_Y;
  const int xpos = mb->mbx * MB_WIDTH_Y;
  const int cypos = ypos / 2;
  const int cxpos = xpos / 2;
  const int stride = curr->stride;
  const int cstride = curr->cstride;
  uint8_t * restrict curr_y = &curr->y [ypos * stride + xpos];
  uint8_t * restrict curr_u = &curr->u [cypos * cstride + cxpos];
  uint8_t * restrict curr_v = &curr->v [cypos * cstride + cxpos];

#if 1
  TAA_H264_ALIGN(64) uint8_t pred_y[MB_SIZE_Y];
  TAA_H264_ALIGN(64) uint8_t pred_uv[MB_SIZE_UV*2];
  {
    const int yvec = 4 * ypos + pred.y;
    const int xvec = 4 * xpos + pred.x;
    const int xint = xvec >> 2;
    const int yint = yvec >> 2;
    const int xint_ref = xint + PAD_HORZ_Y;
    const int yint_ref = yint + PAD_VERT_Y;
    const int zfrac = 4 * (yvec & 3) + (xvec & 3);

    taa_h264_save_best_16x16(
      stride,
      zfrac,
      &pad_prev->y[yint_ref * stride + xint_ref],
      pred_y);
  }

  {
    const int yint = cypos + (pred.y >> 3);
    const int xint = cxpos + (pred.x >> 3);
    const int yint_ref = yint + PAD_VERT_UV;
    const int xint_ref = xint + PAD_HORZ_UV;
    const int yfrac = pred.y & 7;
    const int xfrac = pred.x & 7;

    taa_h264_save_best_chroma_8x8(
      xfrac,
      yfrac,
      cstride,
      &pad_prev->u[yint_ref*cstride + xint_ref],
      &pad_prev->v[yint_ref*cstride + xint_ref],
      pred_uv);
  }
  _mm_store_pd((double *)(curr_y +  0*stride), _mm_load_pd((double *)(pred_y +  0*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  1*stride), _mm_load_pd((double *)(pred_y +  1*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  2*stride), _mm_load_pd((double *)(pred_y +  2*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  3*stride), _mm_load_pd((double *)(pred_y +  3*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  4*stride), _mm_load_pd((double *)(pred_y +  4*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  5*stride), _mm_load_pd((double *)(pred_y +  5*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  6*stride), _mm_load_pd((double *)(pred_y +  6*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  7*stride), _mm_load_pd((double *)(pred_y +  7*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  8*stride), _mm_load_pd((double *)(pred_y +  8*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y +  9*stride), _mm_load_pd((double *)(pred_y +  9*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 10*stride), _mm_load_pd((double *)(pred_y + 10*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 11*stride), _mm_load_pd((double *)(pred_y + 11*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 12*stride), _mm_load_pd((double *)(pred_y + 12*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 13*stride), _mm_load_pd((double *)(pred_y + 13*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 14*stride), _mm_load_pd((double *)(pred_y + 14*MB_WIDTH_Y)));
  _mm_store_pd((double *)(curr_y + 15*stride), _mm_load_pd((double *)(pred_y + 15*MB_WIDTH_Y)));

  const __m128d xmm0 = _mm_load_pd((double *)(pred_uv +  0*MB_STRIDE_UV));
  const __m128d xmm1 = _mm_load_pd((double *)(pred_uv +  1*MB_STRIDE_UV));
  const __m128d xmm2 = _mm_load_pd((double *)(pred_uv +  2*MB_STRIDE_UV));
  const __m128d xmm3 = _mm_load_pd((double *)(pred_uv +  3*MB_STRIDE_UV));
  const __m128d xmm4 = _mm_load_pd((double *)(pred_uv +  4*MB_STRIDE_UV));
  const __m128d xmm5 = _mm_load_pd((double *)(pred_uv +  5*MB_STRIDE_UV));
  const __m128d xmm6 = _mm_load_pd((double *)(pred_uv +  6*MB_STRIDE_UV));
  const __m128d xmm7 = _mm_load_pd((double *)(pred_uv +  7*MB_STRIDE_UV));

  _mm_storel_pd((double *)&curr_u[0*cstride], xmm0);
  _mm_storeh_pd((double *)&curr_v[0*cstride], xmm0);
  _mm_storel_pd((double *)&curr_u[1*cstride], xmm1);
  _mm_storeh_pd((double *)&curr_v[1*cstride], xmm1);
  _mm_storel_pd((double *)&curr_u[2*cstride], xmm2);
  _mm_storeh_pd((double *)&curr_v[2*cstride], xmm2);
  _mm_storel_pd((double *)&curr_u[3*cstride], xmm3);
  _mm_storeh_pd((double *)&curr_v[3*cstride], xmm3);
  _mm_storel_pd((double *)&curr_u[4*cstride], xmm4);
  _mm_storeh_pd((double *)&curr_v[4*cstride], xmm4);
  _mm_storel_pd((double *)&curr_u[5*cstride], xmm5);
  _mm_storeh_pd((double *)&curr_v[5*cstride], xmm5);
  _mm_storel_pd((double *)&curr_u[6*cstride], xmm6);
  _mm_storeh_pd((double *)&curr_v[6*cstride], xmm6);
  _mm_storel_pd((double *)&curr_u[7*cstride], xmm7);
  _mm_storeh_pd((double *)&curr_v[7*cstride], xmm7);
#else
  TAA_H264_ALIGN(64) uint8_t pred_y[MB_SIZE_Y];
  TAA_H264_ALIGN(64) uint8_t pred_u[MB_SIZE_UV];
  TAA_H264_ALIGN(64) uint8_t pred_v[MB_SIZE_UV];
  taa_h264_get_block_luma_ref (pad_prev->y, pad_prev->stride,
                               ypos * 4 + pred.y, xpos * 4 + pred.x,
                               pred_y, MB_WIDTH_Y, MB_HEIGHT_Y, MB_WIDTH_Y);
  taa_h264_get_block_chroma_ref (pad_prev->u, pad_prev->cstride,
                                 pred_u, MB_HEIGHT_UV, MB_WIDTH_UV, MB_WIDTH_UV,
                                 cypos, cxpos, pred.y, pred.x);
  taa_h264_get_block_chroma_ref (pad_prev->v, pad_prev->cstride,
                                 pred_v, MB_HEIGHT_UV, MB_WIDTH_UV, MB_WIDTH_UV,
                                 cypos, cxpos, pred.y, pred.x);


  for (int i = 0; i < MB_HEIGHT_Y; i++)
  {
    for (int j = 0; j < MB_WIDTH_Y; j++)
    {
      curr_y [i * stride + j] = pred_y[i * MB_WIDTH_Y + j];
    }
  }
  for (int i = 0; i < MB_HEIGHT_UV; i++)
  {
    for (int j = 0; j < MB_WIDTH_UV; j++)
    {
      curr_u[i * cstride + j] = pred_u[i * MB_WIDTH_UV + j];
      curr_v[i * cstride + j] = pred_v[i * MB_WIDTH_UV + j];
    }
  }
#endif
}

/**
 * Finds the motion vector to use for skipped MB. Returns true if the
 * MV to use is a zero vector. */
static
bool taa_h264_find_skip_motion_vector (
  const mv_t * mvsbuf,                                      /* MV buffer pointing at current MB */
  int          mvs_stride,
  const mv_t * pred,                                        /* Predicted MV for current MB */
  int          avail,                                       /* MB availability flags */
  mv_t *       mv_out)                                      /* Returned MV to use for SKIP compenstation */
{
  bool avail_a = MB_LEFT (avail);
  bool avail_b = MB_UP (avail);
  int offset_a = avail_a ? -13 : 0;
  int offset_b = avail_b ? -mvs_stride + 12 : 0;
  const mv_t * mv_a = &mvsbuf[offset_a];
  const mv_t * mv_b = &mvsbuf[offset_b];

  return taa_h264_compute_skip_motion_vector (
           pred, mv_a, mv_b, avail_a, avail_b, mv_out);
}


static void taa_h264_recon_skipped_mbs (
  frameinfo_t *       frameinfo,
  const sliceinfo_t * slice,
  int                 start_pos,
  int                 end_pos,
  int                 qp,
  mv_t *              motion_vectors,
  int                 mv_stride)
{
  int mv_offset = 0;
  mv_t * motion_vectors_ptr = &motion_vectors[start_pos * NUM_MVS_MB];

  mbinfo_t mb;
  const int mbxmax = frameinfo->width / MB_WIDTH_Y;

  for (int spos = start_pos; spos < end_pos; spos++)
  {
    /* TODO: init mb can take mbpos */
    int mby = spos / mbxmax;
    int mbx = spos % mbxmax;

    taa_h264_init_mb (frameinfo, &mb, mby, mbx, slice->first_mb);
    bool left = MB_LEFT (mb.avail_flags);
    bool up = MB_UP (mb.avail_flags);
    bool upright = MB_UP_RIGHT (mb.avail_flags);
    bool upleft = MB_UP_LEFT (mb.avail_flags);

    mv_t pmv = taa_h264_predict_motion_vector (
      &motion_vectors_ptr[mv_offset],
      mv_stride,
      0, 16, 16, 0,        /* P_16x16 ref 0 */
      left, up, upright, upleft);
    mv_t mv;
    taa_h264_find_skip_motion_vector (
      &motion_vectors_ptr[mv_offset],
      mv_stride,
      &pmv,
      mb.avail_flags,
      &mv);

    mv_t cmv;
    cmv.x = (int16_t) TAA_H264_CLIP (mv.x, mb.max_mv_top_left.x, mb.max_mv_bottom_right.x);
    cmv.y = (int16_t) TAA_H264_CLIP (mv.y, mb.max_mv_top_left.y, mb.max_mv_bottom_right.y);

    taa_h264_recon_skipped(&mb, &frameinfo->curr, frameinfo->ref_buffers[0], cmv);

    for (int i = 0; i < NUM_MVS_MB; i++)
      motion_vectors_ptr[mv_offset + i] = mv;

    const uint16_t cbp = 0;

    taa_h264_store_mbinfo (mbxmax, mbx, mby, P_SKIP, mb.avail_flags,
                           cbp, qp,
                           slice->disable_deblock,
                           slice->filter_offset_a,
                           slice->filter_offset_b,
                           &motion_vectors_ptr[mv_offset],
#ifdef TAA_SAVE_264_MEINFO
                           0,
#endif
                           &frameinfo->mbinfo_stored[spos]);

    mv_offset += NUM_MVS_MB;
  }
}


static bool taa_h264_decode_i4x4_luma (
  mbinfo_t *         mb,
  uint8_t * restrict curr_ptr,                                           /* pointer to recon for this mb */
  int                curr_stride,
  uint8_t *          num_coeffs_ptr,                                       /* pointer to array for this mb */
  int                num_coeffs_stride,
  uint8_t *          intra_modes_ptr,                                      /* pointer to array for this mb */
  bitreader_t *      reader,
  error_handler_t *  eh)
{
  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_top = MB_UP (mb->avail_flags);
  bool mb_top_left = MB_UP_LEFT (mb->avail_flags);
  bool mb_top_right = MB_UP_RIGHT (mb->avail_flags);
  bool mb_left_intra = MB_LEFT_INTRA (mb->avail_flags);
  bool mb_top_intra = MB_UP_INTRA (mb->avail_flags);
  bool mb_top_left_intra = MB_UP_LEFT_INTRA (mb->avail_flags);
  bool mb_top_right_intra = MB_UP_RIGHT_INTRA (mb->avail_flags);

  int left_flags;
  int top_flags;
  int top_right_flags;
  int top_left_flags;
  int left_intra_flags;
  int top_intra_flags;
  int top_right_intra_flags;
  int top_left_intra_flags;

  taa_h264_set_4x4_blocks_neighbor_flags (
    mb_left, mb_top, mb_top_right, mb_top_left,
    &left_flags, &top_flags, &top_right_flags, &top_left_flags);

  taa_h264_set_4x4_blocks_neighbor_flags (
    mb_left_intra, mb_top_intra, mb_top_right_intra, mb_top_left_intra,
    &left_intra_flags, &top_intra_flags, &top_right_intra_flags, &top_left_intra_flags);

  TAA_H264_ALIGN(64) int16_t diff[4 * 4];

  int count = NUM_4x4_BLOCKS_Y - 1;
  int cbp_4x4 = 0x0;

  for (int k = 0; k < 16; k += 8)
  {
    for (int l = 0; l < 16; l += 8)
    {
      int i8x8 = k / 4 + l / 8;
      for (int m = 0; m < 8; m += 4)
      {
        for (int n = 0; n < 8; n += 4)
        {
          int raster_idx = k + l / 4 + m + n / 4;

          bool left_avail = NEIGHBOR_BLOCK_AVAILABLE (left_flags, raster_idx);
          bool up_avail = NEIGHBOR_BLOCK_AVAILABLE (top_flags, raster_idx);

          if (mb->cbp & (1 << i8x8))
          {
            TAA_H264_ALIGN(64) int16_t qcoeff[16];
            TAA_H264_ALIGN(64) int16_t dqcoeff[16];
            bool first_col = raster_idx % 4 == 0;
            bool first_row = raster_idx / 4 == 0;

            int total_coeffs_pred = taa_h264_predict_nonzero_coeffs (
              &num_coeffs_ptr[raster_idx], num_coeffs_stride,
              first_col, first_row, left_avail, up_avail, false);

            int total_coeffs = taa_h264_get_coeffs_luma_cavlc (
              reader, qcoeff, total_coeffs_pred);
            num_coeffs_ptr[raster_idx] = (uint8_t) total_coeffs;

            cbp_4x4 |= (total_coeffs != 0 ? 1 : 0) << count;

            const int chroma = 0;
            const int ac = 0;
            taa_h264_dequantize_4x4 (qcoeff, dqcoeff, mb->mquant, chroma, ac, 0);
            taa_h264_inverse_transform_4x4_dec (dqcoeff, diff);
          }
          else
          {
            for (int i = 0; i < 16; i++)
              diff[i] = 0;
          }

          uint8_t * curr_y = &curr_ptr [(k + m) * curr_stride + l + n];
          uint8_t left[4];
          uint8_t * top = NULL;

          if (left_avail)
          {
            left[0] = curr_y [-1];
            left[1] = curr_y [-1 + 1 * curr_stride];
            left[2] = curr_y [-1 + 2 * curr_stride];
            left[3] = curr_y [-1 + 3 * curr_stride];
          }

          if (up_avail)
            top = &curr_y [-curr_stride];

          left_avail = NEIGHBOR_BLOCK_AVAILABLE (left_intra_flags, raster_idx);
          up_avail = NEIGHBOR_BLOCK_AVAILABLE (top_intra_flags, raster_idx);
          bool upright_avail = NEIGHBOR_BLOCK_AVAILABLE (top_right_intra_flags, raster_idx);
          bool upleft_avail = NEIGHBOR_BLOCK_AVAILABLE (top_left_intra_flags, raster_idx);

          /* Prediction and reconstruction */
          imode4_t imode = (imode4_t) intra_modes_ptr [raster_idx];
          if (!taa_h264_intra_4x4_pred_and_recon_luma (
                imode, curr_y, curr_stride, left, top,
                left_avail, up_avail, upleft_avail, upright_avail, diff))
          {
            TAA_H264_ERROR (eh, TAA_H264_INVALID_INTRA_MODE);
            return false;
          }

          count--;
        }
      }
    }
  }
  mb->cbp_4x4 = cbp_4x4;
  return true;
}


static bool taa_h264_decode_i16x16_luma (
  mbinfo_t *         mb,
  uint8_t * restrict curr_ptr,                                             /* pointer to recon for this mb */
  int                curr_stride,
  uint8_t *          num_coeffs_ptr,                                         /* pointer to array for this mb */
  int                num_coeffs_stride,
  bitreader_t *      reader,
  error_handler_t *  eh)
{
  TAA_H264_ALIGN(64) int16_t residual [MB_SIZE_Y];
  TAA_H264_ALIGN(64) int16_t dcbuf [4 * 4];

  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_top = MB_UP (mb->avail_flags);
  bool mb_left_intra = MB_LEFT_INTRA (mb->avail_flags);
  bool mb_top_intra = MB_UP_INTRA (mb->avail_flags);
  bool mb_topleft_intra = MB_UP_LEFT (mb->avail_flags);

  /* DC coeffs are always present */
  unsigned total_coeffs_pred = taa_h264_predict_nonzero_coeffs (
    &num_coeffs_ptr[0],
    num_coeffs_stride,
    true,
    true,
    mb_left,
    mb_top,
    false);
  taa_h264_get_coeffs_luma_dc_cavlc (reader, dcbuf, total_coeffs_pred);
  taa_h264_inverse_hadamard_and_dequant_4x4 (dcbuf, mb->mquant);

  /* AC coeffs */
  const int CBP_LUMA_AC_MASK = 0x0F;
  if (mb->cbp & CBP_LUMA_AC_MASK)
  {
    mb->cbp_4x4 = 0xffff;
    for (int i8 = 0; i8 < 4; i8++)
    {
      int raster_idx = (i8 / 2) * 8 + (i8 % 2) * 2;
      for (int i4 = 0; i4 < 4; i4++)
      {
        TAA_H264_ALIGN(64) int16_t qcoeff [4 * 4];
        TAA_H264_ALIGN(64) int16_t dqcoeff [4 * 4];

        bool first_col = raster_idx % 4 == 0;
        bool first_row = raster_idx / 4 == 0;
        bool left_avail = mb_left || !first_col;
        bool up_avail = mb_top || !first_row;

        total_coeffs_pred = taa_h264_predict_nonzero_coeffs (
          &num_coeffs_ptr[raster_idx],
          num_coeffs_stride,
          first_col,
          first_row,
          left_avail,
          up_avail,
          false);

        int total_coeffs = taa_h264_get_coeffs_luma_ac_cavlc (
          reader, qcoeff, total_coeffs_pred);
        num_coeffs_ptr[raster_idx] = (uint8_t) total_coeffs;

        dqcoeff[0] = (int16_t) dcbuf[raster_idx];
        const int chroma = 0;
        const int ac = 1;
        taa_h264_dequantize_4x4 (qcoeff, dqcoeff, mb->mquant, chroma, ac, 0);
        taa_h264_inverse_transform_4x4_dec (dqcoeff, &residual[raster_idx * 16]);

        raster_idx += (i4 == 1) ? 3 : 1;
      }
    }
  }
  else
  {
    TAA_H264_ALIGN(64) short dqcoeff [16] = { 0, };

    for (int k = 0; k < NUM_4x4_BLOCKS_Y; k++)
    {
      dqcoeff[0] = (int16_t) dcbuf[k];
      taa_h264_inverse_transform_4x4_dec (dqcoeff, &residual[k * 16]);
    }

    mb->cbp_4x4 = 0x0;
  }

  /* Intra pred and reconstruct */
  if (!taa_h264_intra_16x16_pred_and_recon_luma (
        mb->intra_16x16_mode, curr_ptr, curr_stride,
        mb_left_intra, mb_top_intra, mb_topleft_intra, residual))
  {
    TAA_H264_ERROR (eh, TAA_H264_INVALID_INTRA_MODE);
    return false;
  }
  return true;
}


static bool taa_h264_decode_pcm_luma_chroma (
  uint8_t *     curr_y_ptr,
  uint8_t *     curr_u_ptr,
  uint8_t *     curr_v_ptr,
  int           curr_stride,
  int           curr_cstride,
  uint8_t *     num_coeffs_y_ptr,
  uint8_t *     num_coeffs_uv_ptr,
  bitreader_t * reader)
{
  for (int i = 0; i < MB_HEIGHT_Y; i++)
  {
    for (int j = 0; j < MB_WIDTH_Y; j++)
    {
      curr_y_ptr [i * curr_stride + j] =
        taa_h264_read_byte_aligned (reader);
    }
  }

  for (int i = 0; i < MB_HEIGHT_UV; i++)
  {
    for (int j = 0; j < MB_WIDTH_UV; j++)
    {
      curr_u_ptr [i * curr_cstride + j] =
        taa_h264_read_byte_aligned (reader);
    }
  }

  for (int i = 0; i < MB_HEIGHT_UV; i++)
  {
    for (int j = 0; j < MB_WIDTH_UV; j++)
    {
      curr_v_ptr [i * curr_cstride + j] =
        taa_h264_read_byte_aligned (reader);
    }
  }

  for (int i = 0; i < NUM_4x4_BLOCKS_Y; i++)
    num_coeffs_y_ptr [i] = 16;

  for (int i = 0; i < 2 * NUM_4x4_BLOCKS_UV; i++)
    num_coeffs_uv_ptr [i] = 16;

  return true;
}


static bool taa_h264_decode_inter_luma (
  mbinfo_t *         mb,
  uint8_t * restrict curr_ptr,               /* pointer to recon for this mb */
  int                curr_stride,
  uint8_t *          num_coeffs_ptr,         /* pointer to array for this mb */
  int                num_coeffs_stride,
  mv_t *             motion_vectors_ptr,     /* pointer to array for this mb */
  framebuf_t * *     ref_buffers,
  bitreader_t *      reader)
{
  TAA_H264_ALIGN(64) uint8_t recon_y[256];
  TAA_H264_ALIGN(64) int16_t diff [256];

  const int CBP_LUMA_8x8_MASK = 0xF;

  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_up  = MB_UP (mb->avail_flags);

  int ypos = mb->mby * MB_HEIGHT_Y;
  int xpos = mb->mbx * MB_WIDTH_Y;

  int cbp_4x4 = 0x0;

  /* Find and store the signaled residual for each block (if any). */
  if (mb->cbp & CBP_LUMA_8x8_MASK)
  {
    memset(diff, 0, 256 * sizeof(int16_t));

    for (int k = 0; k < 2; k++)
    {
      for (int l = 0; l < 2; l++)
      {
        int i8x8 = 2*k + l;
        if (mb->cbp & (1 << i8x8))
        {
          int count = 15 - 4*i8x8;
          for (int m = 0; m < 2; m++)
          {
            for (int n = 0; n < 2; n++)
            {
              const int raster_idx = 8*k + 2*l + 4*m + n;
              const bool first_col = raster_idx % 4 == 0;
              const bool first_row = raster_idx / 4 == 0;
              const bool left_avail = mb_left || !first_col;
              const bool up_avail = mb_up || !first_row;
              TAA_H264_ALIGN(64) int16_t qcoeff[16];

              const int total_coeffs_pred = taa_h264_predict_nonzero_coeffs (
                &num_coeffs_ptr[raster_idx], num_coeffs_stride,
                first_col, first_row, left_avail, up_avail, false);

              const int total_coeffs = taa_h264_get_coeffs_luma_cavlc (
                reader, qcoeff, total_coeffs_pred);
              num_coeffs_ptr[raster_idx] = (uint8_t) total_coeffs;

              cbp_4x4 |= (total_coeffs != 0 ? 1 : 0) << count;

              const int chroma = 0;
              const int ac = 0;

              TAA_H264_ALIGN(64) int16_t buff[16];
              TAA_H264_ALIGN(64) int16_t dqcoeff[16];
              taa_h264_dequantize_4x4 (qcoeff, dqcoeff, mb->mquant, chroma, ac, 0);
              taa_h264_inverse_transform_4x4_dec (dqcoeff, buff);

              __m128d xmm0 = _mm_load_pd((double *)&buff[0]);
              __m128d xmm1 = _mm_load_pd((double *)&buff[8]);
              _mm_storel_pd((double *)&diff[(8*k+4*m+0)*16 + (8*l+4*n)], xmm0);
              _mm_storeh_pd((double *)&diff[(8*k+4*m+1)*16 + (8*l+4*n)], xmm0);
              _mm_storel_pd((double *)&diff[(8*k+4*m+2)*16 + (8*l+4*n)], xmm1);
              _mm_storeh_pd((double *)&diff[(8*k+4*m+3)*16 + (8*l+4*n)], xmm1);

              count--;
            }
          }
        }
      }
    }
  }
  /* Find the predicted (interpolated) blocks */
  if (mb->mbtype == P_16x16)
  {
    const mv_t * mv = &motion_vectors_ptr[0];
    const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
    const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
    const int yvec = 4 * ypos + clipped_mvy;
    const int xvec = 4 * xpos + clipped_mvx;
    const int xint = xvec >> 2;
    const int yint = yvec >> 2;
    const int xint_ref = xint + PAD_HORZ_Y;
    const int yint_ref = yint + PAD_VERT_Y;
    const int zfrac = 4 * (yvec & 3) + (xvec & 3);
    const int stride = ref_buffers[mv->ref_num]->stride;

    taa_h264_save_best_16x16(
      stride,
      zfrac,
      &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref],
      recon_y);
  }
  else if (mb->mbtype == P_16x8)
  {
    {
      const mv_t * mv = &motion_vectors_ptr[0];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_16x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref],
        recon_y);
    }
    {
      const mv_t * mv = &motion_vectors_ptr[8];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_16x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[(yint_ref + 8) * stride + xint_ref],
        &recon_y[8 * MB_WIDTH_Y]);
    }
  }
  else if (mb->mbtype == P_8x16)
  {
    {
      const mv_t * mv = &motion_vectors_ptr[0];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x16(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref],
        recon_y);
    }
    {
      const mv_t * mv = &motion_vectors_ptr[2];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x16(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref + 8],
        &recon_y[8]);
    }
  }
  else if (mb->all_P_8x8)
  {
    {
      const mv_t * mv = &motion_vectors_ptr[0];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref],
        recon_y);
    }
    {
      const mv_t * mv = &motion_vectors_ptr[2];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[yint_ref * stride + xint_ref + 8],
        &recon_y[8]);
    }
    {
      const mv_t * mv = &motion_vectors_ptr[8];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[(yint_ref + 8) * stride + xint_ref],
        &recon_y[8 * MB_WIDTH_Y]);
    }
    {
      const mv_t * mv = &motion_vectors_ptr[10];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yvec = 4 * ypos + clipped_mvy;
      const int xvec = 4 * xpos + clipped_mvx;
      const int xint = xvec >> 2;
      const int yint = yvec >> 2;
      const int xint_ref = xint + PAD_HORZ_Y;
      const int yint_ref = yint + PAD_VERT_Y;
      const int zfrac = 4 * (yvec & 3) + (xvec & 3);
      const int stride = ref_buffers[mv->ref_num]->stride;

      taa_h264_save_best_8x8(
        stride,
        zfrac,
        &ref_buffers[mv->ref_num]->y[(yint_ref + 8) * stride + xint_ref + 8],
        &recon_y[8 * MB_WIDTH_Y + 8]);
    }
  }
  else
  {
    for (int k = 0; k < 16; k += 8)
    {
      for (int l = 0; l < 16; l += 8)
      {
        for (int m = 0; m < 8; m += 4)
        {
          for (int n = 0; n < 8; n += 4)
          {
            int raster_idx = k + l / 4 + m + n / 4;
            uint8_t pblock[4 * 4];
            mv_t * mv = &motion_vectors_ptr[raster_idx];
            int clipped_mvx = TAA_H264_CLIP (mv->x,
                                             mb->max_mv_top_left.x,
                                             mb->max_mv_bottom_right.x);
            int clipped_mvy = TAA_H264_CLIP (mv->y,
                                             mb->max_mv_top_left.y,
                                             mb->max_mv_bottom_right.y);

            int yvec = 4 * (ypos + k + m) + clipped_mvy;
            int xvec = 4 * (xpos + l + n) + clipped_mvx;

            taa_h264_get_block_luma_ref (ref_buffers[mv->ref_num]->y,
                                         ref_buffers[mv->ref_num]->stride,
                                         yvec,
                                         xvec,
                                         pblock,
                                         4, 4, 4);

            for (int i = 0; i < 4; i++)
            {
              for (int j = 0; j < 4; j++)
              {
                recon_y [(k + m + i) * 16 + l + n + j] = pblock[i * 4 + j];
              }
            }
          }
        }
      }
    }
  }
  mb->cbp_4x4 = cbp_4x4;

  if (mb->cbp & CBP_LUMA_8x8_MASK)
  {
    for (int i = 0; i < 256; i++)
      diff[i] = (int16_t) (diff[i] + recon_y[i]);
    for (int i = 0; i < 256; i++)
      recon_y[i] = (uint8_t)(min(max(diff[i], 0), 255));
  }

  _mm_store_pd((double *)(curr_ptr +  0*curr_stride), *(__m128d *)(recon_y +  0*16));
  _mm_store_pd((double *)(curr_ptr +  1*curr_stride), *(__m128d *)(recon_y +  1*16));
  _mm_store_pd((double *)(curr_ptr +  2*curr_stride), *(__m128d *)(recon_y +  2*16));
  _mm_store_pd((double *)(curr_ptr +  3*curr_stride), *(__m128d *)(recon_y +  3*16));
  _mm_store_pd((double *)(curr_ptr +  4*curr_stride), *(__m128d *)(recon_y +  4*16));
  _mm_store_pd((double *)(curr_ptr +  5*curr_stride), *(__m128d *)(recon_y +  5*16));
  _mm_store_pd((double *)(curr_ptr +  6*curr_stride), *(__m128d *)(recon_y +  6*16));
  _mm_store_pd((double *)(curr_ptr +  7*curr_stride), *(__m128d *)(recon_y +  7*16));
  _mm_store_pd((double *)(curr_ptr +  8*curr_stride), *(__m128d *)(recon_y +  8*16));
  _mm_store_pd((double *)(curr_ptr +  9*curr_stride), *(__m128d *)(recon_y +  9*16));
  _mm_store_pd((double *)(curr_ptr + 10*curr_stride), *(__m128d *)(recon_y + 10*16));
  _mm_store_pd((double *)(curr_ptr + 11*curr_stride), *(__m128d *)(recon_y + 11*16));
  _mm_store_pd((double *)(curr_ptr + 12*curr_stride), *(__m128d *)(recon_y + 12*16));
  _mm_store_pd((double *)(curr_ptr + 13*curr_stride), *(__m128d *)(recon_y + 13*16));
  _mm_store_pd((double *)(curr_ptr + 14*curr_stride), *(__m128d *)(recon_y + 14*16));
  _mm_store_pd((double *)(curr_ptr + 15*curr_stride), *(__m128d *)(recon_y + 15*16));

  return true;
}


static bool taa_h264_decode_chroma (
  mbinfo_t *        mb,
  uint8_t *         curr_u_ptr,
  uint8_t *         curr_v_ptr,
  int               curr_cstride,
  uint8_t *         num_coeffs_ptr,
  int               num_coeffs_stride,
  mv_t *            motion_vectors_ptr,
  framebuf_t * *    ref_buffers,
  bitreader_t *     reader,
  error_handler_t * eh)
{
  TAA_H264_ALIGN(64) int16_t qcoeff_uv[MB_SIZE_UV * 2];
  TAA_H264_ALIGN(64) int16_t dqcoeff_uv[MB_SIZE_UV * 2] = {0, };

  const int CBP_CHROMA_DC_MASK = (0x3 << 4);
  const int CBP_CHROMA_AC_MASK = (0x2 << 4);

  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_top  = MB_UP (mb->avail_flags);
  bool mb_left_intra = MB_LEFT_INTRA (mb->avail_flags);
  bool mb_top_intra = MB_UP_INTRA (mb->avail_flags);
  bool mb_topleft_intra = MB_UP_LEFT_INTRA (mb->avail_flags);

  if (mb->cbp & CBP_CHROMA_DC_MASK)
  {
    for (int uv = 0; uv < 2; uv++)
    {
      int16_t qcoeff[4];
      taa_h264_get_coeffs_chroma_dc_cavlc (reader, qcoeff);
      taa_h264_dequantize_chroma_dc (qcoeff, &dqcoeff_uv[MB_SIZE_UV * uv], mb->mquant, mb->chroma_qp_index_offset);
    }
  }

  if (mb->cbp & CBP_CHROMA_AC_MASK)
  {
    int q_offset = 0;
    int nc_offset = 0;

    for (int uv = 0; uv < 2; uv++)
    {
      for (int i = 0; i < NUM_4x4_BLOCKS_UV; i++)
      {
        bool first_col = i % 2 == 0;
        bool first_row = i / 2 == 0;
        bool left_avail = mb_left || !first_col;
        bool up_avail = mb_top || !first_row;

        int total_coeffs_pred = taa_h264_predict_nonzero_coeffs (
          &num_coeffs_ptr[nc_offset], num_coeffs_stride,
          first_col, first_row, left_avail, up_avail, true);

        int total_coeffs = taa_h264_get_coeffs_chroma_ac_cavlc (
          reader, &qcoeff_uv[q_offset], total_coeffs_pred);
        num_coeffs_ptr[nc_offset] = (uint8_t) total_coeffs;

        const int chroma = 1;
        const int ac = 1;
        taa_h264_dequantize_4x4 (&qcoeff_uv[q_offset],
                                 &dqcoeff_uv[q_offset],
                                 mb->mquant, chroma, ac, mb->chroma_qp_index_offset);
        q_offset += 16;
        nc_offset += 1;
      }
    }
  }

  /* U and V reconstruction */
  if (MB_TYPE_IS_INTRA (mb->mbtype))
  {
    for (int uv = 0; uv < 2; uv++)
    {
      TAA_H264_ALIGN(64) int16_t diff[MB_SIZE_UV];
      if (mb->cbp & (CBP_CHROMA_DC_MASK | CBP_CHROMA_AC_MASK))
      {
        taa_h264_inverse_hadamard_2x2_dec (&dqcoeff_uv [64*uv]);
        for (int i = 0; i < 4; i++)
          taa_h264_inverse_transform_4x4_dec (&dqcoeff_uv[64*uv+16*i], &diff[16*i]);
      }
      else
      {
        memset(diff, 0, MB_SIZE_UV*sizeof(int16_t));
      }
      uint8_t * curr_uv = (uv == 0) ? curr_u_ptr : curr_v_ptr;
      if (!taa_h264_intra_pred_and_recon_chroma_8x8 (
            mb->intra_mode_chroma, curr_uv, curr_cstride,
            mb_left_intra, mb_top_intra, mb_topleft_intra, diff))
      {
        TAA_H264_ERROR (eh, TAA_H264_INVALID_INTRA_MODE);
        return false;
      }

    }
  }
  else
  {
    TAA_H264_ALIGN(64) uint8_t recon_uv[MB_SIZE_UV * 2];
    TAA_H264_ALIGN(64) int16_t diff_uv[MB_SIZE_UV * 2];

    const int cypos = mb->mby * MB_HEIGHT_UV;
    const int cxpos = mb->mbx * MB_WIDTH_UV;

    /* U and V inverse transform  */
    if (mb->cbp & (CBP_CHROMA_DC_MASK | CBP_CHROMA_AC_MASK))
    {
      int offset = 0;
      for (int uv = 0; uv < 2; uv++)
      {
        TAA_H264_ALIGN(64) int16_t diff[16];

        taa_h264_inverse_hadamard_2x2_dec (&dqcoeff_uv [offset]);

        for (int i = 0; i < 2; i++)
        {
          taa_h264_inverse_transform_4x4_dec (&dqcoeff_uv[offset], diff);
          __m128d xmm0 = _mm_load_pd((double *)(&diff[0]));
          __m128d xmm1 = _mm_load_pd((double *)(&diff[8]));
          _mm_storel_pd((double *)&diff_uv[ 0+8*uv+4*i], xmm0);
          _mm_storeh_pd((double *)&diff_uv[16+8*uv+4*i], xmm0);
          _mm_storel_pd((double *)&diff_uv[32+8*uv+4*i], xmm1);
          _mm_storeh_pd((double *)&diff_uv[48+8*uv+4*i], xmm1);
          offset += 16;
        }
        for (int i = 0; i < 2; i++)
        {
          taa_h264_inverse_transform_4x4_dec (&dqcoeff_uv[offset], diff);
          __m128d xmm0 = _mm_load_pd((double *)(&diff[0]));
          __m128d xmm1 = _mm_load_pd((double *)(&diff[8]));
          _mm_storel_pd((double *)&diff_uv[ 0+8*uv+4*i+64], xmm0);
          _mm_storeh_pd((double *)&diff_uv[16+8*uv+4*i+64], xmm0);
          _mm_storel_pd((double *)&diff_uv[32+8*uv+4*i+64], xmm1);
          _mm_storeh_pd((double *)&diff_uv[48+8*uv+4*i+64], xmm1);
          offset += 16;
        }
      }
    }

    if (mb->mbtype == P_16x16)
    {
      const mv_t * mv = &motion_vectors_ptr[0];
      const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
      const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
      const int yint = cypos + (clipped_mvy >> 3);
      const int xint = cxpos + (clipped_mvx >> 3);
      const int yint_ref = yint + PAD_VERT_UV;
      const int xint_ref = xint + PAD_HORZ_UV;
      const int yfrac = clipped_mvy & 7;
      const int xfrac = clipped_mvx & 7;

      taa_h264_save_best_chroma_8x8(
        xfrac,
        yfrac,
        curr_cstride,
        &ref_buffers[mv->ref_num]->u[yint_ref*curr_cstride + xint_ref],
        &ref_buffers[mv->ref_num]->v[yint_ref*curr_cstride + xint_ref],
        recon_uv);
    }
    else if (mb->mbtype == P_16x8)
    {
      {
        const mv_t * mv = &motion_vectors_ptr[0];
        const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
        const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
        const int yint = cypos + (clipped_mvy >> 3);
        const int xint = cxpos + (clipped_mvx >> 3);
        const int yint_ref = yint + PAD_VERT_UV;
        const int xint_ref = xint + PAD_HORZ_UV;
        const int yfrac = clipped_mvy & 7;
        const int xfrac = clipped_mvx & 7;

        taa_h264_save_best_chroma_8x4(
          xfrac,
          yfrac,
          curr_cstride,
          &ref_buffers[mv->ref_num]->u[yint_ref*curr_cstride + xint_ref],
          &ref_buffers[mv->ref_num]->v[yint_ref*curr_cstride + xint_ref],
          recon_uv);
      }
      {
        const mv_t * mv = &motion_vectors_ptr[8];
        const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
        const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
        const int yint = cypos + (clipped_mvy >> 3);
        const int xint = cxpos + (clipped_mvx >> 3);
        const int yint_ref = yint + PAD_VERT_UV + 4;
        const int xint_ref = xint + PAD_HORZ_UV + 0;
        const int yfrac = clipped_mvy & 7;
        const int xfrac = clipped_mvx & 7;

        taa_h264_save_best_chroma_8x4(
          xfrac,
          yfrac,
          curr_cstride,
          &ref_buffers[mv->ref_num]->u[yint_ref*curr_cstride + xint_ref],
          &ref_buffers[mv->ref_num]->v[yint_ref*curr_cstride + xint_ref],
          &recon_uv[4 * MB_STRIDE_UV]);
      }
    }
    else if (mb->mbtype == P_8x16)
    {
      {
        const mv_t * mv = &motion_vectors_ptr[0];
        const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
        const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
        const int yint = cypos + (clipped_mvy >> 3);
        const int xint = cxpos + (clipped_mvx >> 3);
        const int yint_ref = yint + PAD_VERT_UV;
        const int xint_ref = xint + PAD_HORZ_UV;
        const int yfrac = clipped_mvy & 7;
        const int xfrac = clipped_mvx & 7;

        taa_h264_save_best_chroma_4x8(
          xfrac,
          yfrac,
          curr_cstride,
          &ref_buffers[mv->ref_num]->u[yint_ref*curr_cstride + xint_ref],
          &ref_buffers[mv->ref_num]->v[yint_ref*curr_cstride + xint_ref],
          recon_uv);
      }
      {
        const mv_t * mv = &motion_vectors_ptr[2];
        const int clipped_mvx = TAA_H264_CLIP (mv->x, mb->max_mv_top_left.x, mb->max_mv_bottom_right.x);
        const int clipped_mvy = TAA_H264_CLIP (mv->y, mb->max_mv_top_left.y, mb->max_mv_bottom_right.y);
        const int yint = cypos + (clipped_mvy >> 3);
        const int xint = cxpos + (clipped_mvx >> 3);
        const int yint_ref = yint + PAD_VERT_UV + 0;
        const int xint_ref = xint + PAD_HORZ_UV + 4;
        const int yfrac = clipped_mvy & 7;
        const int xfrac = clipped_mvx & 7;

        taa_h264_save_best_chroma_4x8(
          xfrac,
          yfrac,
          curr_cstride,
          &ref_buffers[mv->ref_num]->u[yint_ref*curr_cstride + xint_ref],
          &ref_buffers[mv->ref_num]->v[yint_ref*curr_cstride + xint_ref],
          &recon_uv[4]);
      }
    }
    else
    {
      for (int uv = 0; uv < 2; uv++)
      {
        const int block_size = 4 * 4;
        int offset = MB_SIZE_UV * uv;

        for (int m = 0; m < 8; m += 4)
        {
          for (int n = 0; n < 8; n += 4)
          {
            uint8_t pblock[4 * 4] = {0, };

            for (int m2 = 0; m2 < 4; m2 += 2)
            {
              for (int n2 = 0; n2 < 4; n2 += 2)
              {
                int luma_raster_idx = (m + m2) * 2 + (n + n2) / 2;
                mv_t * mv = &motion_vectors_ptr[luma_raster_idx];
                uint8_t * refbuf_uv = uv == 0 ?
                                      ref_buffers[mv->ref_num]->u : ref_buffers[mv->ref_num]->v;
                int clipped_mvx = TAA_H264_CLIP (mv->x,
                                                 mb->max_mv_top_left.x,
                                                 mb->max_mv_bottom_right.x);
                int clipped_mvy = TAA_H264_CLIP (mv->y,
                                                 mb->max_mv_top_left.y,
                                                 mb->max_mv_bottom_right.y);

                taa_h264_get_block_chroma_ref (refbuf_uv,
                                               ref_buffers[mv->ref_num]->cstride,
                                               &pblock[m2 * 4 + n2],
                                               2, 2, 4,
                                               cypos + m + m2,
                                               cxpos + n + n2,
                                               (int16_t) clipped_mvy,
                                               (int16_t) clipped_mvx);
              }
            }

            for (int i = 0; i < 4; i++)
            {
              for (int j = 0; j < 4; j++)
              {
                recon_uv [(m + i) * 16 + n + j + 8 * uv] = pblock[i * 4 + j];
              }
            }
            offset += block_size;
          }
        }
      }
    }
    if (mb->cbp & (CBP_CHROMA_DC_MASK | CBP_CHROMA_AC_MASK)) // diff is not zero
    {
      for (int i = 0; i < 128; i++)
        diff_uv[i] = (int16_t) (diff_uv[i] + recon_uv[i]);
      for (int i = 0; i < 128; i++)
        recon_uv[i] = (uint8_t)(min(max(diff_uv[i], 0), 255));
    }

    _mm_storel_pd((double *)&curr_u_ptr[0*curr_cstride], *(__m128d *) &recon_uv[  0]);
    _mm_storeh_pd((double *)&curr_v_ptr[0*curr_cstride], *(__m128d *) &recon_uv[  0]);
    _mm_storel_pd((double *)&curr_u_ptr[1*curr_cstride], *(__m128d *) &recon_uv[ 16]);
    _mm_storeh_pd((double *)&curr_v_ptr[1*curr_cstride], *(__m128d *) &recon_uv[ 16]);
    _mm_storel_pd((double *)&curr_u_ptr[2*curr_cstride], *(__m128d *) &recon_uv[ 32]);
    _mm_storeh_pd((double *)&curr_v_ptr[2*curr_cstride], *(__m128d *) &recon_uv[ 32]);
    _mm_storel_pd((double *)&curr_u_ptr[3*curr_cstride], *(__m128d *) &recon_uv[ 48]);
    _mm_storeh_pd((double *)&curr_v_ptr[3*curr_cstride], *(__m128d *) &recon_uv[ 48]);
    _mm_storel_pd((double *)&curr_u_ptr[4*curr_cstride], *(__m128d *) &recon_uv[ 64]);
    _mm_storeh_pd((double *)&curr_v_ptr[4*curr_cstride], *(__m128d *) &recon_uv[ 64]);
    _mm_storel_pd((double *)&curr_u_ptr[5*curr_cstride], *(__m128d *) &recon_uv[ 80]);
    _mm_storeh_pd((double *)&curr_v_ptr[5*curr_cstride], *(__m128d *) &recon_uv[ 80]);
    _mm_storel_pd((double *)&curr_u_ptr[6*curr_cstride], *(__m128d *) &recon_uv[ 96]);
    _mm_storeh_pd((double *)&curr_v_ptr[6*curr_cstride], *(__m128d *) &recon_uv[ 96]);
    _mm_storel_pd((double *)&curr_u_ptr[7*curr_cstride], *(__m128d *) &recon_uv[112]);
    _mm_storeh_pd((double *)&curr_v_ptr[7*curr_cstride], *(__m128d *) &recon_uv[112]);
  }

  return true;
}

/* Conceal a missing macroblock. SRC is preferably previous displayed picture.
 * If NULL then an alternative mechanism is used. */
static void taa_h264_conceal_errors_mb (
  framebuf_t *       dst,
  const framebuf_t * src,
  int                mbx,
  int                mby)
{
  uint8_t * dy = &dst->y[mby * MB_HEIGHT_Y * dst->stride + mbx * MB_WIDTH_Y];
  uint8_t * du = &dst->u[mby * MB_HEIGHT_UV * dst->cstride + mbx * MB_WIDTH_UV];
  uint8_t * dv = &dst->v[mby * MB_HEIGHT_UV * dst->cstride + mbx * MB_WIDTH_UV];

  if (src)
  {
    /* Copy from previous frame */
    const uint8_t * sy = &src->y[mby * MB_HEIGHT_Y * src->stride + mbx * MB_WIDTH_Y];
    const uint8_t * su = &src->u[mby * MB_HEIGHT_UV * src->cstride + mbx * MB_WIDTH_UV];
    const uint8_t * sv = &src->v[mby * MB_HEIGHT_UV * src->cstride + mbx * MB_WIDTH_UV];

    for (int i = 0; i < MB_HEIGHT_Y; i++)
    {
      memcpy (&dy[i * dst->stride], &sy[i * dst->stride], MB_WIDTH_Y * sizeof(uint8_t));
    }
    for (int i = 0; i < MB_HEIGHT_UV; i++)
    {
      memcpy (&du[i * dst->cstride], &su[i * dst->cstride], MB_WIDTH_UV * sizeof(uint8_t));
      memcpy (&dv[i * dst->cstride], &sv[i * dst->cstride], MB_WIDTH_UV * sizeof(uint8_t));
    }
  }
  else
  {
    /* Conceal with a fixed color. */
    const uint8_t y_val = 16;
    const uint8_t u_val = 128;
    const uint8_t v_val = 128;

    for (int i = 0; i < MB_HEIGHT_Y; i++)
    {
      memset (&dy[i * dst->stride], y_val, MB_WIDTH_Y * sizeof(uint8_t));
    }
    for (int i = 0; i < MB_HEIGHT_UV; i++)
    {
      memset (&du[i * dst->cstride], u_val, MB_WIDTH_UV * sizeof(uint8_t));
      memset (&dv[i * dst->cstride], v_val, MB_WIDTH_UV * sizeof(uint8_t));
    }
  }
}


static void taa_h264_conceal_errors (
  framebuf_t *           curr,
  const framebuf_t *     src,
  const mbinfo_store_t * mbinfo_stored,
  int                    mbxmax,
  int                    mbymax)
{
  int mbpos = 0;
  for (int mby = 0; mby < mbymax; mby++)
  {
    for (int mbx = 0; mbx < mbxmax; mbx++)
    {
      const mbinfo_store_t * mbinfo = &mbinfo_stored[mbpos++];
      if (!MB_IS_DECODED(mbinfo->flags))
      {
        taa_h264_conceal_errors_mb (curr, src, mbx, mby);
      }
    }
  }
}


/* Terminates frame and returns whether this frame is in sync */
static bool taa_h264_terminate_frame (
  frameinfo_t *              frameinfo,
  decoded_picture_buffer_t * dpb,
  cpuinfo_t *                cpuinfo,
  callbacks_t *              callbacks)
{
  if (frameinfo->maybe_ghost_frame)
  {
    /* We don't trust that this frame does not look crappy because it might
     * have used a significantly older reference frame than it was supposed 
     * to. Conceal the whole frame even though it has been decoded without 
     * error. This will will most likely look better than using the wrong long 
     * term reference frame. */
    framebuf_t conceal_src;
    if (taa_h264_dpb_get_conceal_src (dpb, &conceal_src))
    {
      taa_h264_unpad_framebuf (&conceal_src);
      const int mbymax = frameinfo->height / 16;
      const int mbxmax = frameinfo->width / 16;
      for (int mby = 0; mby < mbymax; mby++)
      {
        for (int mbx = 0; mbx < mbxmax; mbx++)
          taa_h264_conceal_errors_mb (&frameinfo->curr, &conceal_src, mbx, mby);
      }
    }
  }
#ifdef NDEBUG
  if (cpuinfo->have_ssse3)
  {
    taa_h264_deblock_frame_ssse3 (&frameinfo->curr,
                                  frameinfo->mbinfo_stored,
                                  frameinfo->mb_deblock_info,
                                  frameinfo->motion_vectors,
                                  frameinfo->height / MB_HEIGHT_Y,
                                  frameinfo->width / MB_WIDTH_Y,
                                  frameinfo->resolution);
  }
  else
#endif
  {
    taa_h264_deblock_frame (&frameinfo->curr,
                            frameinfo->mbinfo_stored,
                            frameinfo->mb_deblock_info,
                            frameinfo->motion_vectors,
                            frameinfo->height / MB_HEIGHT_Y,
                            frameinfo->width / MB_WIDTH_Y,
                            frameinfo->resolution);
  }

#if REF_FRAME_CHECKSUM
  taa_h264_dpb_set_checksum (dpb);
#endif

  bool in_sync = taa_h264_dec_dpb_update_metainfo (dpb,
                                                   frameinfo->frame_num,
                                                   frameinfo->all_mbs_decoded,
                                                   frameinfo->ref_in_sync);

  taa_h264_dpb_update_picnums (dpb, frameinfo->frame_num);

  taa_h264_dec_dpb_rpm_ack (dpb,
                            frameinfo->frame_num,
                            frameinfo->is_ref,
                            frameinfo->is_idr,
                            frameinfo->is_idr_long_term,
                            frameinfo->adaptive_marking_flag,
                            frameinfo->mmco,
                            callbacks);

  in_sync &= taa_h264_dpb_update (dpb,
                                  &frameinfo->frame_num,
                                  frameinfo->is_ref,
                                  frameinfo->is_idr,
                                  frameinfo->is_idr_long_term,
                                  frameinfo->adaptive_marking_flag,
                                  frameinfo->mmco);

  TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES,
    "Terminate frame %d, mbs decoded %d, ref in sync %d, this in sync %d\n",
    frameinfo->frame_num, frameinfo->num_mbs_decoded, frameinfo->ref_in_sync, in_sync);

  return in_sync;
}

static void taa_h264_report_frame_complete (
  frameinfo_t *       frameinfo,
  decoded_picture_t * curr_pic,
  bool                happy,
  callbacks_t *       cb)
{
  framebuf_t * buf = &frameinfo->curr;

  if (cb->report_frame_complete)
  {
    taa_h264_decoded_frame_t dp = {
      .y = buf->y,
      .u = buf->u,
      .v = buf->v,
      .width = buf->cols,
      .height = buf->rows,
      .stride = buf->stride,
      .stride_chroma = buf->cstride,
      .is_in_sync = happy && frameinfo->ref_in_sync,
      .crop_left = frameinfo->crop_left,
      .crop_right = frameinfo->crop_right,
      .crop_top = frameinfo->crop_top,
      .crop_bottom = frameinfo->crop_bottom,
      .par_n = frameinfo->sar_width,
      .par_d = frameinfo->sar_height,
      .is_long_term = curr_pic->is_longterm,
      .is_idr = frameinfo->is_idr,
      .frame_num = frameinfo->frame_num,
      .picid = curr_pic->picid,
    };

    cb->report_frame_complete (cb->context, &dp);
  }
}

static void terminate_incomplete_frame (
  decoder_t * decoder)
{
  frameinfo_t * frameinfo = &decoder->frameinfo;

  // We have packet loss. Conceal it and terminate the frame.
  TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "Incomplete frame detected\n");

  /* We only output the corrupt frame if we previously have received a full
  * I frame (so that we're not in a totally hopeless state quality wise)
  * or if we have configured the decoder to output such frames anyway.
  * Typically this translates to "should we output the first IDR frame
  * even if parts of it are missing". */
  bool output_this_frame =
    decoder->sequence.have_complete_iframe || OUTPUT_CORRUPT_IDR_SEQUENCE;

  framebuf_t src;
  if (taa_h264_dpb_get_conceal_src (&decoder->dpb, &src))
  {
    /* We have previously displayed buffers available. Conceal using
    * last displayed frame. */
    TAA_H264_TRACE (
      TAA_H264_TRACE_KEY_VALUES, "Conceal with previous frame\n");

    taa_h264_unpad_framebuf (&src);
    taa_h264_conceal_errors (&frameinfo->curr,
      &src,
      frameinfo->mbinfo_stored,
      frameinfo->width / MB_WIDTH_Y,
      frameinfo->height / MB_HEIGHT_Y);
  }
  else
  {
    TAA_H264_TRACE (
      TAA_H264_TRACE_KEY_VALUES, "No previous frame available for concealment\n");
    taa_h264_conceal_errors (&frameinfo->curr,
      NULL,
      frameinfo->mbinfo_stored,
      frameinfo->width / MB_WIDTH_Y,
      frameinfo->height / MB_HEIGHT_Y);
  }

  decoder->happy = taa_h264_terminate_frame (frameinfo,
    &decoder->dpb,
    &decoder->cpuinfo,
    &decoder->callbacks);

  if (output_this_frame)
  {
    decoded_picture_t * output_frame = &decoder->dpb.pictures[decoder->dpb.output_idx];
    taa_h264_report_frame_complete (frameinfo, output_frame, decoder->happy,
      &decoder->callbacks);
  }
}


/* Check if there is a gap in frame_num and handles it accordingly.
 * FRAMEINFO still should hold previous frame's values, while SLICEINFO is the
 * first slice of the current frame.
 * Returns TRUE if there is a gap that should be propagated to next frame.
 * Returns force_out_of_sync=TRUE the gap forces current frame to be out of sync.
 * Returns maybe_ghost_frame=TRUE if gap may cause the "ghost" artifact. */
static bool check_and_handle_frame_num_gap (
  const frameinfo_t *        frameinfo,
  const sliceinfo_t *        sliceinfo,
  decoded_picture_buffer_t * dpb,
  bool                       gaps_in_frame_num_allowed,
  bool                       have_gap,
  bool *                     force_out_of_sync,
  bool *                     maybe_ghost_frame,
  error_handler_t *          eh)
{
  int prev_fn = frameinfo->frame_num;
  int curr_fn = sliceinfo->frame_num;
  int max_fn = frameinfo->max_frame_num;

  // If previous frame was disposable we don't expected increased frame_num
  bool prev_frame_was_disposable = !frameinfo->is_ref;
  int expect_fn = prev_frame_was_disposable ? prev_fn : prev_fn + 1;
  if (expect_fn >= max_fn)
    expect_fn -= max_fn;

  if ((curr_fn != expect_fn) && (sliceinfo->nalutype != NALU_TYPE_IDR))
  {
    if (gaps_in_frame_num_allowed)
    {
      taa_h264_dec_dpb_fill_gap_in_frame_num (dpb, prev_fn, curr_fn);
    }
    else
    {
      have_gap = true;
      /* Report error but continue decoding the best we can. The problem is
      * that we don't know what frames should be in the reference buffer.
      * The missing frames could have had MMCO commands. */ 
      TAA_H264_WARNING (eh, TAA_H264_GAP_IN_FRAME_NUM_NOT_ALLOWED);
      TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "frame_num gap: prev_frame_num %d, "
        "curr_frame_num %d\n", prev_fn, curr_fn);
    }
  }

  if (have_gap)
  {
    /* Be conservative. When we see a gap in frame_num assume that we lost
    * something important. The downside is that a super p-frame needs at
    * least one of the packets of the previous frame to go through (so there
    * isn't a gap), thus less super p-frames are ACKED. We assume ref pic
    * marking repetition will take care of missing MMCOs, thus we only
    * invalidate the short term frames.*/
    *force_out_of_sync = true;
    taa_h264_dec_dpb_set_all_short_term_out_of_sync (dpb);

    /* If there is a possibility that we use a much older LT reference than we
    * are supposed to, we might have a "ghost frame". These are really ugly
    * artifacts and should be treated differently. Typically happens if this
    * is a short term frame that refers to a long term frame (e.g. a LT frame
    * is lost and the next frame refers it but instead uses the old LT frame
    * with same LT-index). */
    *maybe_ghost_frame = (!sliceinfo->adaptive_marking_flag) && sliceinfo->reordering_flag;

    /* Losing frame_num == 0 is potentially dangerous since it is likely 
    * to be an IDR. We hope to detect this by repetition RPMs but it 
    * might be the case that there is another IDR from previous sequence 
    * at the same position in the reference buffer. Thus normal handling 
    * of repetition RPM will not work since frame_num 0 will be ACKed. 
    * Thus, we set frame_num 0 to be an invalid frame, causing any 
    * repeating RPM about frame_num 0 to generate a NACK/FUR. */
    if (prev_fn >= curr_fn && curr_fn > 0)
      taa_h264_dec_dpb_set_idr_as_unusable_reference (dpb);

    /* Errors caused by gaps must be propagated to the next reference frame if
    * current frame is a disposable frame. If not the following might happen:
    * 1. Disposable frame
    * 2. Lost LT frame
    * 3. Disposable frame detects a gap and is set out of sync (OOS may be lost)
    * 4. P frame refers to lost LT frame which instead uses previous LT frame
    *    with same LT-index and is thus marked OK. */
    if (sliceinfo->nalref_idc != 0)
      have_gap = false;
  }

  return have_gap;
}


static void taa_h264_decode_slice (
  decoder_t *         decoder,
  const sliceinfo_t * sliceinfo,
  bool                new_frame,
  bitreader_t *       reader)
{
  frameinfo_t * frameinfo = &decoder->frameinfo;

  if (new_frame)
  {
    if (!frameinfo->all_mbs_decoded && !decoder->sequence.first_slice)
    {
      terminate_incomplete_frame (decoder);
    }

    const pic_parset_t * pps = &decoder->sequence.pic_parsets[sliceinfo->pic_parset_id];
    const seq_parset_t * sps = &decoder->sequence.seq_parsets[pps->seq_parset_id];
    bool force_out_of_sync = false;
    bool maybe_ghost_frame = false;

    TAA_H264_DEBUG_ASSERT (pps->is_valid);

    decoder->sequence.have_gap = check_and_handle_frame_num_gap (
      frameinfo,
      sliceinfo,
      &decoder->dpb,
      sps->gaps_in_frame_num_allowed,
      decoder->sequence.have_gap,
      &force_out_of_sync,
      &maybe_ghost_frame,
      &decoder->error_handler);

    if (sliceinfo->nalutype == NALU_TYPE_IDR)
    {
      /* Reset the DPB. Reset is also done when IDR frame is terminated, but
       * doing it here is needed in some very rare an special packet loss cases.
       * If we have a good IDR frame already in the DPB, don't receive the
       * complete frame of the current IDR, and then get a SEI rpm repetition
       * referring to this IDR before the next frame, the old IDR would exist in
       * the DPB as a good frame and we would send an ACK instead of NACK. */
      taa_h264_dpb_reset (&decoder->dpb);
    }

    decoder->sequence.first_slice = false;
    taa_h264_init_frame (frameinfo, sliceinfo, pps, sps, &decoder->dpb, force_out_of_sync, maybe_ghost_frame);
  }

  if (frameinfo->cancel)
  {
    TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES,
                    "Decoding of this slice has been canceled\n");
    return;
  }

  int prev_qp = sliceinfo->qp;
  framebuf_t * curr = &frameinfo->curr;
  unsigned mba;
  unsigned pos = 0;
  unsigned ypos, xpos, cxpos, cypos;
  unsigned mby, mbx;
  const unsigned mbxmax = frameinfo->width / 16;
  const unsigned mbymax = frameinfo->height / 16;
  const unsigned num_mbs = mbxmax * mbymax;
  const unsigned mv_stride = mbxmax * NUM_MVS_MB;
  unsigned rest_skipped = 0;

  mba = sliceinfo->first_mb;

  /* Checking for pos < num_mbs in case of a corrupt stream and invalid skip
   * runs. Only test for more data is needed for correct streams. */
  while (pos < num_mbs && taa_h264_reader_has_more_data (reader))
  {
    unsigned mbadiff = 1;
    if (sliceinfo->islice == false)
    {
      unsigned mbrun = TAA_H264_READ_UE (reader, "mb_skip_run");
      mbadiff = mbrun + 1;
    }

    mba += mbadiff;

    /* Find the MB position in the frame */
    pos = mba - 1;

    if (pos < num_mbs && taa_h264_reader_has_more_data (reader))
    {
      //TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "pos = %d\n", pos);

      mby = pos / mbxmax;
      mbx = pos % mbxmax;
      ypos = mby * 16;
      xpos = mbx * 16;
      cypos = ypos / 2;
      cxpos = xpos / 2;

      mbinfo_t mb_;
      mbinfo_t * mb = &mb_;
      mv_t * motion_vectors_ptr = &frameinfo->motion_vectors[pos * NUM_MVS_MB];

      taa_h264_init_mb (frameinfo, mb, mby, mbx, sliceinfo->first_mb);

      /* Fill in data for any skipped macroblocks */
      if (mbadiff > 1)
      {
        frameinfo->ref_in_sync &= taa_h264_dec_dpb_ref_in_sync (&decoder->dpb, 0);

        if (taa_h264_dec_dpb_ref_exist (&decoder->dpb, 0))
        {
          int start_pos = mba - mbadiff;
          taa_h264_recon_skipped_mbs (frameinfo, sliceinfo, start_pos, pos, prev_qp,
                                      frameinfo->motion_vectors, mv_stride);
        }
        else
        {
          TAA_H264_ERROR (&decoder->error_handler,
                          TAA_H264_INVALID_REFERENCE_FRAME);
          mb->avail_flags = MB_UNSET_DECODED (mb->avail_flags);
          return;
        }
      }

      int res = taa_h264_read_mb_header (frameinfo, mb, sliceinfo->islice, reader, (unsigned *)&prev_qp);
      if (res != TAA_H264_NO_ERROR)
      {
        TAA_H264_ERROR (&decoder->error_handler, res);
        mb->avail_flags = MB_UNSET_DECODED (mb->avail_flags);
        return;
      }

      /* Store whether we're using a good reference frame or not. */
      if (MB_TYPE_IS_INTRA (mb->mbtype))
      {
        frameinfo->ref_in_sync &= true;
        frameinfo->num_mbs_intra++;
      }
      else
      {
        /* Check the status of reference frame(s) */
        for (int i = 0; i < NUM_MVS_MB; i++)
        {
          bool ref_exist = taa_h264_dec_dpb_ref_exist (
            &decoder->dpb, motion_vectors_ptr[i].ref_num);

          frameinfo->ref_in_sync &= taa_h264_dec_dpb_ref_in_sync (
            &decoder->dpb, motion_vectors_ptr[i].ref_num);

          if (!ref_exist)
          {
            frameinfo->ref_in_sync = false;
            TAA_H264_ERROR (&decoder->error_handler, TAA_H264_INVALID_REFERENCE_FRAME);
            mb->avail_flags = MB_UNSET_DECODED (mb->avail_flags);
            return;
          }
        }
      }

      // TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "mbtype = %d\n", mb->mbtype);

      /* Y component */
      bool mb_ok = true;
      if (mb->mbtype == I_4x4)
      {
        int offset = pos * NUM_4x4_BLOCKS_Y;
        mb_ok &= taa_h264_decode_i4x4_luma (
          mb,
          &curr->y [ypos * curr->stride + xpos],
          curr->stride,
          &frameinfo->num_coeffs_y [offset],
          mbxmax * NUM_4x4_BLOCKS_Y,
          &frameinfo->intra_modes [offset],
          reader,
          &decoder->error_handler);
      }
      else if (mb->mbtype == I_16x16)
      {
        int offset = pos * NUM_4x4_BLOCKS_Y;
        mb_ok &= taa_h264_decode_i16x16_luma (
          mb,
          &curr->y [ypos * curr->stride + xpos],
          curr->stride,
          &frameinfo->num_coeffs_y [offset],
          mbxmax * NUM_4x4_BLOCKS_Y,
          reader,
          &decoder->error_handler);
      }
      else if (mb->mbtype == I_PCM)
      {
        int offset = pos * NUM_4x4_BLOCKS_Y;
        mb_ok &= taa_h264_decode_pcm_luma_chroma (
          &curr->y [ypos * curr->stride + xpos],
          &curr->u [cypos * curr->cstride + cxpos],
          &curr->v [cypos * curr->cstride + cxpos],
          curr->stride,
          curr->cstride,
          &frameinfo->num_coeffs_y [offset],
          &frameinfo->num_coeffs_uv [pos * NUM_4x4_BLOCKS_UV * 2],
          reader);
      }
      else
      {
        int offset = pos * NUM_4x4_BLOCKS_Y;
        mb_ok &= taa_h264_decode_inter_luma (
          mb,
          &curr->y [ypos * curr->stride + xpos],
          curr->stride,
          &frameinfo->num_coeffs_y [offset],
          mbxmax * NUM_4x4_BLOCKS_Y,
          &frameinfo->motion_vectors [offset],
          frameinfo->ref_buffers,
          reader);
      }

      if (mb->mbtype != I_PCM)
      {
        mb_ok &= taa_h264_decode_chroma (
          mb,
          &curr->u [cypos * curr->cstride + cxpos],
          &curr->v [cypos * curr->cstride + cxpos],
          curr->cstride,
          &frameinfo->num_coeffs_uv [pos * NUM_4x4_BLOCKS_UV * 2],
          mbxmax * NUM_4x4_BLOCKS_UV * 2,
          motion_vectors_ptr,
          frameinfo->ref_buffers,
          reader,
          &decoder->error_handler);
      }
      // Abort decoding of slice if one MB is not decodable. May be ugly
      // artificats if we continue to decode an invalid stream
      if (!mb_ok)
      {
        mb->avail_flags = MB_UNSET_DECODED (mb->avail_flags);
        return;
      }

      if (pos == mbymax * mbxmax - 1)
      {
        /* Finish the frame */
        mba++;
      }

      static uint8_t const TAA_H264_ALIGN (16) zigzag_8x8[16] =
      {
        0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
      };

      uint16_t cpb_blk = 0;
      for (int i = 0; i < NUM_4x4_BLOCKS_Y; i++)
      {
        cpb_blk = (uint16_t) (cpb_blk | ((mb->cbp_4x4 >> (15 - i)) & 0x1) <<  zigzag_8x8[i]);
      }
      taa_h264_store_mbinfo (mbxmax, mbx, mby, mb->mbtype, mb->avail_flags,
                             cpb_blk, mb->mquant,
                             sliceinfo->disable_deblock,
                             sliceinfo->filter_offset_a,
                             sliceinfo->filter_offset_b,
                             motion_vectors_ptr,
#ifdef TAA_SAVE_264_MEINFO
                             0,
#endif
                             &frameinfo->mbinfo_stored[mb->mbpos]);

      frameinfo->num_mbs_decoded += mbadiff;
    }
    else if (pos <= num_mbs)
    {
      rest_skipped = mbadiff - 1;
      frameinfo->num_mbs_decoded += rest_skipped;
    }
    else
    {
      /* We have been told to skip to a position that is bigger than the frame
       * size. Assume the stream is corrupt and treat the invalid skipped mbs
       * as lost mbs. */
    }
  }
  taa_h264_advance_rbsp_trailing_bits (reader);
  /* Fill in data for any remaining skipped macroblocks */
  if (rest_skipped)
  {
    frameinfo->ref_in_sync &= taa_h264_dec_dpb_ref_in_sync (&decoder->dpb, 0);
    if (taa_h264_dec_dpb_ref_exist (&decoder->dpb, 0))
    {
      taa_h264_recon_skipped_mbs (frameinfo, sliceinfo,
                                  pos - rest_skipped, pos, prev_qp,
                                  frameinfo->motion_vectors, mv_stride);
    }
    else
    {
      TAA_H264_ERROR (&decoder->error_handler,
                      TAA_H264_INVALID_REFERENCE_FRAME);
      return;
    }
  }

  TAA_H264_DEBUG_ASSERT (frameinfo->num_mbs_decoded <= num_mbs);
  if (frameinfo->num_mbs_decoded == num_mbs)
  {
    frameinfo->all_mbs_decoded = true;

    if (frameinfo->is_idr || (frameinfo->num_mbs_intra == num_mbs))
    {
      decoder->sequence.have_complete_iframe = true;
    }

    decoder->happy = taa_h264_terminate_frame (
      frameinfo, &decoder->dpb, &decoder->cpuinfo, &decoder->callbacks);

    if (decoder->sequence.have_complete_iframe || OUTPUT_CORRUPT_IDR_SEQUENCE)
    {
      decoded_picture_t * curr_pic = &decoder->dpb.pictures[decoder->dpb.output_idx];
      taa_h264_report_frame_complete (frameinfo, curr_pic, decoder->happy,
                                      &decoder->callbacks);
    }
  }
}


static void taa_h264_set_resolution (
  frameinfo_t * frameinfo,
  int           width,
  int           height)
{
  source_res_t res = RES_UNKNOWN;
  if     (width ==  128 && height ==  96) res = RES_SQCIF;
  else if(width ==  176 && height == 144) res =  RES_QCIF;
  else if(width ==  352 && height == 240) res =   RES_SIF;
  else if(width ==  352 && height == 288) res =   RES_CIF;
  else if(width ==  512 && height == 288) res =  RES_WCIF;
  else if(width ==  576 && height == 448) res = RES_25CIF;
  else if(width ==  640 && height == 480) res =   RES_VGA;
  else if(width ==  704 && height == 480) res =  RES_4SIF;
  else if(width ==  768 && height == 448) res = RES_W3CIF;
  else if(width ==  704 && height == 576) res =  RES_4CIF;
  else if(width ==  800 && height == 600) res =  RES_SVGA;
  else if(width == 1024 && height == 576) res = RES_W4CIF;
  else if(width == 1024 && height == 768) res =   RES_XGA;
  else if(width == 1280 && height == 720) res =  RES_720P;
  else if(width == 1920 && height == 1088) res = RES_1080P;

  frameinfo->width = width;
  frameinfo->height = height;
  frameinfo->resolution = res;
}


/* Checks if current slice belongs to a new frame, according to rules in
 * section 7.4.1.2.4. Returns TRUE if a new frame is detected. */
static bool taa_h264_is_new_frame (
  sliceinfo_t * curr_slice,
  sliceinfo_t * prev_slice,
  bool          frame_complete)
{
  /* This check is not listed in the H.264 recommendation, but is needed for
   * interoperability. E.g. some encoders do not alternate the idr_pic_id for
   * two immediately following IDR frames. This helps detect new frames in such
   * cases, except when packet loss occurs in the first IDR frame. */
  if (frame_complete)
    return true;

  if (curr_slice->frame_num != prev_slice->frame_num)
    return true;

  if (curr_slice->pic_parset_id != prev_slice->pic_parset_id)
    return true;

  if ((curr_slice->nalref_idc != prev_slice->nalref_idc)
      && (curr_slice->nalref_idc == NALU_PRIORITY_DISPOSABLE
          || prev_slice->nalref_idc == NALU_PRIORITY_DISPOSABLE))
    return true;

  /*
   * TODO: Check pic order count
   */
  if (curr_slice->poc.type == 0 && prev_slice->poc.type == 0
      && (curr_slice->poc.lsb != prev_slice->poc.lsb
          || curr_slice->poc.delta_bottom != prev_slice->poc.delta_bottom))
    return true;

  if (curr_slice->poc.type == 1 && prev_slice->poc.type == 1
      && (curr_slice->poc.delta_0 != prev_slice->poc.delta_0
          || curr_slice->poc.delta_1 != prev_slice->poc.delta_1))
    return true;

  if (curr_slice->idr_pic_id != prev_slice->idr_pic_id)
    return true;

  return false;
}

/**
 * Returns true if SPSs are equal. Checks by value.
 */
static bool taa_h264_sps_is_equal (
  const seq_parset_t * sps1,
  const seq_parset_t * sps2)
{
  bool equal = true;
  equal &= sps1->is_valid == sps2->is_valid;
  equal &= sps1->width_mbs == sps2->width_mbs;
  equal &= sps1->height_mbs == sps2->height_mbs;
  equal &= sps1->num_ref_frames == sps2->num_ref_frames;
  return equal;
}

static void taa_h264_teardown_olddpb (
  decoded_picture_buffer_t * dpb, 
  frameinfo_t * frameinfo)
{
  if (dpb->mem_base)
    TAA_H264_MM_FREE (dpb->mem_base);
  if (frameinfo->intra_modes)
    TAA_H264_FREE (frameinfo->intra_modes);
  if (frameinfo->num_coeffs_y)
    TAA_H264_FREE (frameinfo->num_coeffs_y);
  if (frameinfo->num_coeffs_uv)
    TAA_H264_FREE (frameinfo->num_coeffs_uv);
  if (frameinfo->motion_vectors)
    TAA_H264_FREE (frameinfo->motion_vectors);
  if (frameinfo->mbinfo_stored)
    TAA_H264_FREE (frameinfo->mbinfo_stored);

  dpb->mem_base     = NULL;
  frameinfo->num_coeffs_y   = NULL;
  frameinfo->num_coeffs_uv  = NULL;
  frameinfo->intra_modes    = NULL;
  frameinfo->motion_vectors = NULL;
  frameinfo->mbinfo_stored  = NULL;
}

static bool taa_h264_allocate_newdpb (
  const int max_width, 
  const int max_height, 
  const int max_ref_frames,
  decoded_picture_buffer_t * dpb, 
  frameinfo_t * frameinfo, 
  error_handler_t * eh)
{
  int mem_size = taa_h264_dec_dpb_size(max_width, max_height, max_ref_frames);

  dpb->mem_base   = TAA_H264_MM_MALLOC (mem_size, 64);
  if (!dpb->mem_base)
  {
    TAA_H264_ERROR (eh, TAA_H264_ALLOCATION_ERROR_DPB_MEM_BASE);
    return false;
  }

  dpb->max_width  = max_width;
  dpb->max_height = max_height;
  dpb->max_ref_frames = (uint8_t)max_ref_frames;

  if (!taa_h264_frameinfo_create (frameinfo,
                                  max_width,
                                  max_height,
                                  max_ref_frames,
                                  eh,
                                  false))
      return false;

  return true;
}

static bool taa_h264_activate_parameters (
  frameinfo_t *              frameinfo,
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refframes_meta,
  const seq_parset_t *       sps,
  error_handler_t *          eh)
{
  // set the num_ref_frames from active sps
  uint8_t num_ref_frames = sps->num_ref_frames;

  // set the frameinfo width and height from active sps
  taa_h264_set_resolution (
    frameinfo, sps->width_mbs * MB_WIDTH_Y, sps->height_mbs * MB_HEIGHT_Y);

  if(frameinfo->width > MAX_FRAME_WIDTH || frameinfo->height > MAX_FRAME_HEIGHT 
      || frameinfo->width * frameinfo->height > MAX_FRAME_AREA)
  {
	  TAA_H264_ERROR (eh, TAA_H264_UNSUPPORTED_RESOLUTION);
	  return false;
  }

  if(num_ref_frames > MAX_NUM_REF_FRAMES)
  {
	  TAA_H264_ERROR (eh, TAA_H264_INVALID_NUM_REFERENCE_FRAMES);
	  return false;
  }

  // compare previous dpb settings to active sps settings
  if(taa_h264_dec_new_dpb_needed(
    dpb->max_width, dpb->max_height, dpb->max_ref_frames,
    frameinfo->width, frameinfo->height, num_ref_frames))
  {
      taa_h264_teardown_olddpb(dpb, frameinfo);

      // See taa_h264_dec_new_dpb_needed() for conditions warranting re-create
      // of the dpb - there are many.  In general, we are growing the dpb as/when
      // necessary.  Two approaches are possible, and where this  matters the
      // most is when we are receiving switches between landscape and portrait
      // orientation (e.g. when remote device is hand-held).
      //
      // 1. Minimum memory footprint, but more re-allocations.
      //    In this approach, we potentially shrink the buffer on a change from
      //    landscape to portrait or vice-versa.  This is to avoid allocating the
      //    space required to store the largest width and length for both
      //    orientations.
      //    Example:
      //      first res: frameinfo is 672x384 16 refframes - we will allocate 8408064 bytes
      //      next res:  frameinfo is 448x608 16 refframes - we will re-allocate to 8773632 bytes
      //      next res:  frameinfo is 480x640 16 refframes - we will re-allocate to 9765888 bytes
      //      next res:  frameinfo is 736x416 16 refframes - we will re-allocate to 9792000 bytes
      //      next res:  frameinfo is 480x640 16 refframes - we will re-allocate to 9765888 bytes
      //
      // 2. Minimize memory re-allocations, but potentially larger memory footprint.
      //    In this approach we only grow the dpb, period.  This means that we
      //    may end up allocating a buffer that can accomodate the widest width
      //    of landscape and the tallest height of portrait - which would be a
      //    buffer larger than necessary to handle the actual largest landscape
      //    or portrait resolution.  The "benefit" of this approach is to avoid
      //    frequent buffer re-allocations on when landscape/portrait switches
      //    are frequent.
      //    Example:
      //      first res: frameinfo 672x384 16 refframes	- we will allocate 8408064 bytes
      //      next res:  frameinfo 448x608 16 refframes - we will re-allocate to 12612096 bytes
      //      next res:  frameinfo 480x640 16 refframes - we will re-allocate to 13212672 bytes
      //      next res:  frameinfo 736x416 16 refframes - we will re-allocate to 14361600 bytes
      //      next res:  frameinfo 480x640 16 refframes	- no re-allocation is necessary
      //
      // We are currently using approach 1:
      int max_width      = frameinfo->width;
      int max_height     = frameinfo->height;
      //
      // This is approach 2:
      // int max_width      = (dpb->max_width > frameinfo->width) ? dpb->max_width : frameinfo->width;
      // int max_height     = (dpb->max_height > frameinfo->height) ? dpb->max_height : frameinfo->height;
      //
      // Same value for ref frames in either approach.
	  int max_ref_frames = num_ref_frames;

      // allocate or reallocate the buffers in the dpb. the allocated buffers
      // contain extra padding in both the horizontal and vertical directions.
      // the dpb also contains an extra buffer for the current decoded frame.
      if(!taa_h264_allocate_newdpb(max_width, max_height, max_ref_frames, dpb, frameinfo, eh))
          return false;
  }

  // the dpb is initialized. the fourth condition if the active picture size
  // is less than the previous picture size is handled here as well.
  const bool dpb_has_interpolated = false;
  if (!taa_h264_dpb_init (dpb,
                          num_ref_frames,
                          sps->width_mbs * MB_WIDTH_Y,
                          sps->height_mbs * MB_HEIGHT_Y,
                          1 << sps->log2_max_frame_num,
                          dpb_has_interpolated,
                          eh))
    return false;

  taa_h264_dec_dpb_init_metainfo (dpb, refframes_meta);

  return true;
}

/* Returns true if everything is perfect in the decoder, false if the decoder
 * is in error state. */
bool taa_h264_process_nalu (
  decoder_t * decoder,
  uint8_t *   input_buffer,
  unsigned    input_buffer_size)
{
  bitreader_t reader;
  bitreader_t * r = &reader;
  nalutype_t type;
  nalref_t idc;
  sequence_t * seq = &decoder->sequence;
  error_handler_t * eh = &decoder->error_handler;

  // Reset the error detection for each NALU
  eh->error_detected = false;

  taa_h264_load_buffer (r, input_buffer, input_buffer_size);
  taa_h264_read_nalu_header (r, &idc, &type, eh);
  if (!eh->error_detected)
  {
    switch (type)
    {
    case NALU_TYPE_SPS:
    {
      taa_h264_read_sequence_parameter_set (r, seq->seq_parsets);
      break;
    }
    case NALU_TYPE_PPS:
    {
      taa_h264_read_picture_parameter_set (r, seq->pic_parsets);
      break;
    }
    case NALU_TYPE_SLICE:
    case NALU_TYPE_IDR:
    {
      seq->prev_slice = seq->curr_slice;

      taa_h264_read_slice_header (
        r, idc, type,
        seq->seq_parsets,
        seq->pic_parsets,
        &seq->curr_slice,
        eh);
      seq->curr_slice.nalutype = type;
      if (eh->error_detected)
      {
        if (type == NALU_TYPE_IDR)
        {
          /* Reset the DPB. If we abort all IDR slices here, e.g. because of
           * invalid SPS/PPS, there may still be a valid IDR frame in the DPB
           * which will be a cause for problems, e.g we receive a SEI rpm
           * repetition and we ACK that we have a good IDR although we don't.*/
          taa_h264_dpb_reset (&decoder->dpb);
        }

        // Don't bother decoding if an error was discovered in the header
        break;
      }

      /* Check if we need to reconfigure because of SPS change */
      int pps_id = seq->curr_slice.pic_parset_id;
      pic_parset_t * curr_pps = &seq->pic_parsets[pps_id];
      seq_parset_t * curr_sps = &seq->seq_parsets[curr_pps->seq_parset_id];
      /* Need to check against a _copy_ of the active sps in order to detect when
       * we switch to a new SPS which have the same sps_id (and thus overwritten
       * the previous active one). Typically happens when the encoder is restarted
       * with a new resolution. */
      bool new_frame;
      bool equal_sps = taa_h264_sps_is_equal (&seq->active_sps, curr_sps);
      if (!equal_sps || seq->first_slice)
      {
        seq->active_sps = *curr_sps;
		if (!taa_h264_activate_parameters (&decoder->frameinfo,
                                           &decoder->dpb,
                                           decoder->refframes_meta,
                                           curr_sps,
                                           eh))
        {
          decoder->dont_decode = true;		  
          break;
        }
        else
        {
          decoder->dont_decode = false;
        }

        /* Set variables so we don't use any previous frames (which may or may
         * not be of the same resolution). The implication is that we don't try
         * to conceal and output the previous frame if the previous frame was
         * not complete, e.g. due to packet loss. Also, don't use the previous
         * frame for concealment of current frame. This is not optimal, but 
         * doing concealment when changing resolution is tricky.*/
        seq->first_slice = true;
        seq->have_complete_iframe = false;
        seq->have_gap = false;
        new_frame = true;
      }
      else
      {
        new_frame = taa_h264_is_new_frame (
          &seq->curr_slice,
          &seq->prev_slice,
          decoder->frameinfo.all_mbs_decoded);
      }

      if (decoder->dont_decode)
        break;

      taa_h264_decode_slice (decoder, &seq->curr_slice, new_frame, r);
      break;
    }
    case NALU_TYPE_AUD:
      /* Access unit delimiter marks a picture boundary. We don't
       * really need to do something with this since we detect
       * picture boundaries when decoding the first VCL-NALU of the
       * frame. Skip this NALU until it's proven useful. */
      break;
    case NALU_TYPE_SEI:
      taa_h264_read_sei (r, decoder);
      break;
#if REF_FRAME_CHECKSUM
    case NALU_TYPE_CHECKSUM:
    {
      uint32_t checksum;
      int long_term_frame_idx;
      int original_frame_num;
      taa_h264_read_checksum (r, &checksum, &long_term_frame_idx, &original_frame_num);
      taa_h264_dpb_assert_checksum (&decoder->dpb, checksum, long_term_frame_idx, original_frame_num);
      break;
    }
#endif
    default:
    {
      TAA_H264_WARNING (eh, TAA_H264_UNSUPPORTED_NALU_TYPE);
      /* Ignore it but return that everything is ok. */
      break;
    }
    }
  }

  // TODO: Should we really be unhappy for every error we detect, e.g. gaps in
  // frame num?
  decoder->happy = decoder->happy && !eh->error_detected;
  return !eh->error_detected;
}
