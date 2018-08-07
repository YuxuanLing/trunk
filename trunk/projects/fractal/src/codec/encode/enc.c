#include "enc.h"
#include "enc_config.h"
#include "enc_writebits.h"
#include "enc_putvlc.h"
#include "enc_headers.h"
#include "enc_mb.h"
#include "com_mbutils.h"
#include "enc_interpolate.h"
#include "enc_motionsearch.h"
#include "com_deblock.h"
#include "com_error.h"
#include "com_cpuinfo.h"
#include "com_common.h"
#include "encoder.h"
#include "enc_init_mb.h"
#include "enc_sei.h"
#include "enc_control.h"
#include "enc_load_mb.h"
#ifndef AVG_HPEL
# include "enc_interpolate.c"
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <emmintrin.h>


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
#pragma comment(lib, "ws2_32.lib")

typedef struct {
  long tv_sec;
  long tv_usec;
}timeval;

int gettimeofday(struct timeval *tp, struct  timeval *tzp)
{
  /* data */
  // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
  static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

  SYSTEMTIME  system_time;
  FILETIME    file_time;
  uint64_t    time;

  GetSystemTime( &system_time );
  SystemTimeToFileTime( &system_time, &file_time );
  time =  ((uint64_t)file_time.dwLowDateTime )      ;
  time += ((uint64_t)file_time.dwHighDateTime) << 32;

  tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
  tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
  return 0;
};

#endif

#define u8		unsigned char 
#define s32     signed	 long 
#define u32     unsigned long	

#ifndef ABS
#define ABS(X)		_abs((X))
static __inline  s32 _abs(s32 x)
{
  return x>0 ? x : -x;
}
#endif

/*****************************************************************************************
 * \brief i_orgstride * i_orgheight.
 * PSNR Calcutor
 *  @param [in] u8* curYUV  : NO Null
 *  @param [in] u8* refYUV  : NO Null
 *  @param [in] u32	i_curwidth  :  > 0 && (i_orgstride & 3 ) == 0
 *  @param [in] u32	i_refstride  :  > 0 && i_refstride >= i_orgstride
 *  @param [in] u32	i_curheight  :  > 0 && (i_orgheight & 3 ) == 0
 *  history  :
 *   -------------------------------------------------------------------------------------
 *     Hanguang Dan    |  Aug 16, 2007 |  build
 *   -------------------------------------------------------------------------------------
******************************************************************************************/
static double calc_PSNR(u8 *curYUV,
                        u8 *refYUV,
                        u32 i_curwidth,
                        u32 i_refstride,
                        u32 i_curheight )
{
    u32 x;
    s32 org0, org1, org2, org3;
    s32 ref0, ref1, ref2, ref3;
    s32 quad_r;
    u32 size = i_curwidth*i_curheight;
    double quad_sum = 0.0;
    double psnr;

    do{
        quad_r = 0;
        x = i_curwidth>>2;
        do{
            org0 = *curYUV++;
            ref0 = *refYUV++;
            org1 = *curYUV++;
            ref1 = *refYUV++;

            org2 = *curYUV++;
            ref2 = *refYUV++;
            org3 = *curYUV++;
            ref3 = *refYUV++;


            org0 = org0-ref0;
            quad_r += org0*org0;

            org1 = org1-ref1;
            quad_r += org1*org1;

            org2 = org2-ref2;;
            quad_r += org2*org2;

            org3 = org3-ref3;
            quad_r += org3*org3;

        }while( --x );
        refYUV += i_refstride-i_curwidth;
        quad_sum += quad_r;
    }while( --i_curheight );


    quad_sum = quad_sum/size;
    if(quad_sum!=0)
    {
        quad_sum = 255*255/quad_sum;
        psnr = (double)(10.0 * (double)log10(quad_sum));
    }
    else
    {
        psnr = 100.00;
    }

    return psnr;
}

#endif


static const int NUM_MVS_MB  = 16;   /* Number of MVs stored per MB */


static void taa_h264_init_frame (
  frameinfo_t *              frameinfo,
  decoded_picture_buffer_t * dpb)
{
  int mbymax = frameinfo->height / 16;
  int mbxmax = frameinfo->width / 16;

  taa_h264_dpb_get_current_framebuf (dpb, &frameinfo->recon);
  taa_h264_unpad_framebuf (&frameinfo->recon);

  frameinfo->ref_meta = taa_h264_enc_dpb_curr_ref_meta (dpb);
  frameinfo->ref_meta->rpm.idr_flag = false;
  frameinfo->ref_meta->rpm.mmco[0].op = 0;
  frameinfo->rplr[0].idc = 3;

  mv_t * tmp = frameinfo->motion_vectors_curr;
  frameinfo->motion_vectors_curr = frameinfo->motion_vectors_prev;
  frameinfo->motion_vectors_prev = tmp;

  int16_t * tmp_x = frameinfo->motion_vectors_curr_16x16.x;
  int16_t * tmp_y = frameinfo->motion_vectors_curr_16x16.y;

  frameinfo->motion_vectors_curr_16x16.x = frameinfo->motion_vectors_prev_16x16.x;
  frameinfo->motion_vectors_curr_16x16.y = frameinfo->motion_vectors_prev_16x16.y;

  frameinfo->motion_vectors_prev_16x16.x = tmp_x;
  frameinfo->motion_vectors_prev_16x16.y = tmp_y;


  memset (frameinfo->num_coeffs_y, 0,
          mbymax * mbxmax * NUM_4x4_BLOCKS_Y * sizeof (uint8_t));
  memset (frameinfo->num_coeffs_uv, 0,
          mbymax * mbxmax * NUM_4x4_BLOCKS_UV * 2 * sizeof (uint8_t));
  memset (frameinfo->intra_modes, DC_PRED,
          mbymax * mbxmax * NUM_4x4_BLOCKS_Y * sizeof (uint8_t));
}


/**
 * Finds which of the available reference frames should be used. If no suitable
 * reference is found FALSE is returned, implying that the frame should be intra
 * coded.
 *
 * Reference frame information is returned in REFCONTROL. REF_IDX gives the
 * reference list index for the frame to use as a reference. LONGTERM_IDX
 * returns where to put the current frame in the long term reference buffer. If
 * current frame should not be marked as long term, LONGTERM_IDX is negative.
 */
static bool taa_h264_decide_reference_frame (
  frameipbuf_t * *           refframes,
  decoded_picture_buffer_t * dpb,
  int                        frame_num,
  uint8_t                    temporal_id,
  uint8_t                    num_layers,
  const flux_t *             flux,
  ref_control_t *            refcontrol)
{
  ref_meta_t * refmetas[MAX_NUM_REF_FRAMES];
  const bool reordering_flag = false;
  rplr_t * dummy = NULL;
  int total_ref_frames;
  bool ref_found = false;

  total_ref_frames = taa_h264_dpb_set_ref_list (
    dpb,
    reordering_flag,
    dummy,
    frame_num,
    refframes,
    (void * *) refmetas);

  refcontrol->ref_idx = -1;
  refcontrol->longterm_idx = -1;

  if (flux && flux->recover)
  {
    // insert a super p frame explicitly
    ref_found = taa_h264_flux_super_p_control (
      refmetas,
      (uint8_t) total_ref_frames,
      dpb->max_ref_frames,
      refcontrol,
      flux);
  }
  else if (flux && flux->long_term)
  {
    // this is a "flux long term frame"
    ref_found = taa_h264_flux_decide_refframes (
      refmetas,
      (uint8_t) total_ref_frames,
      dpb->max_ref_frames,
      refcontrol,
      flux,
      false);
  }
  else
  {
    // default to standard hiearch. prediction
    ref_found = taa_h264_hierarchical_reference_control (
      refmetas,
      (uint8_t) total_ref_frames,
      temporal_id,
      num_layers,
      refcontrol);
  }

  // Should never occur to have ref_idx < 0 && ref_found == true,
  // but to be on the safe side...
  if (refcontrol->ref_idx < 0)
    ref_found = false;

  return ref_found;
}





/* Returns the number of bits written. */
static unsigned taa_h264_write_mb (
  frameinfo_t * frameinfo,
  mbinfo_t *    currmb,
  int           mbrun,
  bool          islice,
  uint8_t *     last_coded_qp,
  bitwriter_t * writer)
{
  unsigned numbits = 0;
#if defined (TAA_H264_TRACE_ENABLED)
  if (MB_TYPE_IS_INTRA (currmb->mbtype) == false)
  {
    int mvdx = currmb->mv1.x - currmb->pred1.x;
    int mvdy = currmb->mv1.y - currmb->pred1.y;

    TAA_H264_TRACE (TAA_H264_TRACE_MV, "pos = %4d, pred (%3d, %3d), mvd (%3d, %3d) mv (%3d, %3d) mode %d\n",
                    currmb->mbpos, currmb->pred1.x, currmb->pred1.y,
                    mvdx, mvdy, currmb->mv1.x, currmb->mv1.y,
                    currmb->mbtype);
    if (currmb->mbtype > P_16x16)
    {
      mvdx = currmb->mv2.x - currmb->pred2.x;
      mvdy = currmb->mv2.y - currmb->pred2.y;

      TAA_H264_TRACE (TAA_H264_TRACE_MV, "pos = %4d, pred (%3d, %3d), mvd (%3d, %3d) mv (%3d, %3d) mode %d\n",
                      currmb->mbpos, currmb->pred2.x, currmb->pred2.y,
                      mvdx, mvdy, currmb->mv2.x, currmb->mv2.y,
                      currmb->mbtype);

    }
  }
#endif

  numbits += taa_h264_write_mb_header (currmb, writer, frameinfo->intra_modes,
                                       mbrun, islice, last_coded_qp);

  int offset = currmb->mbpos * NUM_4x4_BLOCKS_Y;
  numbits += taa_h264_encode_coeffs_luma_cavlc (currmb,
                                                &frameinfo->num_coeffs_y [offset],
                                                writer);

  int mode_chroma = taa_h264_get_cbp_chroma (currmb);
  if (mode_chroma != 0)
  {
    offset = currmb->mbpos * NUM_4x4_BLOCKS_UV * 2;
    numbits += taa_h264_encode_coeffs_chroma_cavlc (currmb,
                                                    &frameinfo->num_coeffs_uv[offset],
                                                    writer,
                                                    mode_chroma);
  }

  return numbits;
}

static void init_intra (
  frameinfo_t *              frameinfo,
  decoded_picture_buffer_t * dpb)
{
  frameinfo->is_long_term = dpb->max_ref_frames > 1;
  frameinfo->long_term_idx = 0;
  frameinfo->frame_num = 0;
  taa_h264_init_frame (frameinfo, dpb);
  frameinfo->temporal_id = 0;
  if (frameinfo->is_long_term)
    frameinfo->long_term_meta_list[frameinfo->long_term_idx] = frameinfo->ref_meta;
}


static void init_inter (
  frameinfo_t *              frameinfo,
  decoded_picture_buffer_t * dpb,
  const ref_control_t *      refcontrol,
  uint8_t                    temporal_id)
{
  taa_h264_init_frame (frameinfo, dpb);

  frameinfo->frame_num++;
  if (frameinfo->frame_num >= (1<<LOG2_MAX_FRAME_NUM))
    frameinfo->frame_num = 0;

  frameinfo->temporal_id = temporal_id;
  frameinfo->is_long_term = (refcontrol->longterm_idx >= 0);
  frameinfo->long_term_idx = (uint8_t)refcontrol->longterm_idx;
  frameinfo->refframe = frameinfo->refframes[refcontrol->ref_idx];
  frameinfo->refframe_num = 0;

  if (refcontrol->ref_idx > 0)
  {
    taa_h264_generate_rplr (
      dpb,
      frameinfo->frame_num,
      (uint8_t) refcontrol->ref_idx,
      frameinfo->rplr);
  }

  if (refcontrol->longterm_idx >= 0)
  {
    taa_h264_generate_mmco_curr_to_long (
      dpb,
      (uint8_t) refcontrol->longterm_idx,
      frameinfo->ref_meta->rpm.mmco);
    // Save the meta information for the frame generating this LTFI
    frameinfo->long_term_meta_list[refcontrol->longterm_idx] = frameinfo->ref_meta;
  }

#ifndef AVG_HPEL
  int const offset_full = frameinfo->refframe->full.stride * PAD_VERT_Y + PAD_HORZ_Y;
  int const offset_diag = frameinfo->refframe->half_diag.stride * PAD_VERT_Y + PAD_HORZ_Y;
  int const offset_horz = frameinfo->refframe->half_horz.stride * PAD_VERT_Y + PAD_HORZ_Y;
  int const offset_vert = frameinfo->refframe->half_vert.stride * PAD_VERT_Y + PAD_HORZ_Y;
  int const padded_width = frameinfo->width + PAD_HORZ_Y * 2;

  taa_h264_hpel_interpolation (
    &frameinfo->refframe->full.y [offset_full - PAD_HORZ_Y],
    &frameinfo->refframe->full.y [offset_full - 3 * padded_width - PAD_HORZ_Y],
    &frameinfo->refframe->half_diag.y [offset_diag - 3 * padded_width - PAD_HORZ_Y],
    &frameinfo->refframe->half_horz.y [offset_horz - PAD_HORZ_Y],
    &frameinfo->refframe->half_vert.y [offset_vert - 3 * padded_width - PAD_HORZ_Y],
    frameinfo->height,
    padded_width,
    frameinfo->resolution);
#endif

  taa_h264_meinfo_init_frame (
    frameinfo->meinfo,
    frameinfo->width,
    frameinfo->height,
    &frameinfo->refframe->full,
    &frameinfo->refframe->half_diag,
    &frameinfo->refframe->half_horz,
    &frameinfo->refframe->half_vert,
    frameinfo->refframe_num);
}


static int taa_h264_process_frame2 (
  encoder_t *        encoder,
  const uint8_t *    input,
  int                input_width,
  int                input_height,
  const bool         intra_frame,
  const flux_t *     flux,
  const uint32_t     max_packet_size,
  uint8_t *          outbuf,
  int *              num_skip_mbs
#ifdef TAA_SAVE_264_MEINFO
  ,int psnr[3]
  ,unsigned *min_qp_in_frame_out
  ,unsigned *max_qp_in_frame_out
  ,unsigned *avg_qp_in_frame_out
#endif
  )
{
  sequence_t * sequence        = &encoder->sequence;
  frameinfo_t * frameinfo      = &encoder->frameinfo;
  bitwriter_t * writer         = encoder->bitwriter;
  unsigned num_bits_in_slice = 0;
  unsigned num_bits_in_slice_prev = 0;
  unsigned num_bits_in_mb = 0;
  unsigned first_mb_in_slice = 0;
  *num_skip_mbs = 0;

  const int mbxmax = frameinfo->width / MB_WIDTH_Y;
  const int mbymax = frameinfo->height / MB_HEIGHT_Y;
  int last_pos;
  bool idr_frame = intra_frame;

#ifdef TAA_SAVE_264_MEINFO
  unsigned max_qp_in_frame = 0;
  unsigned min_qp_in_frame = 0;
  unsigned qp_in_frame_sum = 0;
  unsigned available_qp_in_frame_num = 0;
#endif

#ifdef TAA_DEBUG_SAVE_MEINFO
  int saved_avail_flags[mbxmax * mbymax];
#endif

  if (flux && flux->ack_status && !idr_frame)
  {
    if (flux->ref_repetition)
    {
      // Repetition of MMCO commands.
      ref_meta_t * repeat_list[MAX_RPM_REPEATS];
      int repeat_count = taa_h264_ref_pic_marking_repetition (
        &encoder->dpb, flux->ack_status, frameinfo->long_term_meta_list, repeat_list);
      if (repeat_count > 0)
      {
        taa_h264_write_ref_pic_marking_sei (
          writer, repeat_list, repeat_count, frameinfo->temporal_id);
        taa_h264_send_nonslice (
          writer, encoder->callbacks.output_nalu, encoder->callbacks.context,
          max_packet_size, &encoder->max_nalu_size);
      }
    }
  }
  else if (!idr_frame && 0) // FIXME: Repetition RPM is wrong for with gop-size=8 an temporal-level=4
  {
    ref_meta_t * repeat_list[MAX_RPM_REPEATS];
    int repeat_count = taa_h264_ref_pic_marking_repetition_all_lt_frames (
      &encoder->dpb, frameinfo->long_term_meta_list, repeat_list);
    if (repeat_count > 0)
    {
      taa_h264_write_ref_pic_marking_sei (
        writer, repeat_list, repeat_count, frameinfo->temporal_id);
      taa_h264_send_nonslice (
        writer, encoder->callbacks.output_nalu, encoder->callbacks.context,
        max_packet_size, &encoder->max_nalu_size);
    }
  }

  frameinfo->ref_meta->rpm.frame_num = frameinfo->frame_num;
  frameinfo->ref_meta->rpm.idr_flag = idr_frame;

  if (idr_frame)
  {
    const int width = encoder->frameinfo.width;
    const int height = encoder->frameinfo.height;

    /* In practice we never send a changed SPS and PPS without creating a new
     * encoder. For equal SPS/PPS it is not necessary to update the ID. Also,
     * Polycom's RMX MCU does not support more than two SPS ids. Thus, we always
     * set it 0. */
    sequence->sps_id = 0;
    sequence->pps_id = 0;
    sequence->idr_pic_id = (uint8_t)((sequence->idr_pic_id + 1) % 2);
    sequence->pps_qp = 30;

    taa_h264_write_sequence_parameter_set (writer, sequence, width, height);
    taa_h264_send_nonslice (
      writer, encoder->callbacks.output_nalu, encoder->callbacks.context,
      max_packet_size, &encoder->max_nalu_size);

    taa_h264_write_picture_parameter_set (writer, sequence);
    taa_h264_send_nonslice (
      writer, encoder->callbacks.output_nalu, encoder->callbacks.context,
      max_packet_size, &encoder->max_nalu_size);

    encoder->last_coded_qp = sequence->pps_qp;
    if (frameinfo->is_long_term)
    {
      /* Store whether this IDR frame is long term by specifying "mark current
       * pic as long term". IDR frames don't use MMCOs so this is for internal
       * use only. */
      frameinfo->ref_meta->rpm.mmco[0].op = 6;
      frameinfo->ref_meta->rpm.mmco[1].op = 0;
    }
  }

#ifdef TAA_SAVE_264_MEINFO
    if(encoder->debug_flag)
    {
      encoder->data_position += sprintf(encoder->data_need_send + encoder->data_position,
          "    {\n    SliceStart -->  first_mb_in_slice=%d slice_qp=%d;\n",
          first_mb_in_slice, encoder->last_coded_qp);
    }
#endif

  num_bits_in_slice += taa_h264_write_slice_header (
    writer,
    sequence,
    idr_frame ? NALU_TYPE_IDR : NALU_TYPE_SLICE,
    intra_frame,
    frameinfo->frame_num,
    first_mb_in_slice,
    encoder->last_coded_qp,
    frameinfo->temporal_id,
    &frameinfo->ref_meta->rpm,
    frameinfo->rplr);


  // Configure load/resizing of input
  if (input_width == 0)
    input_width = encoder->output_width;
  if (input_height == 0)
    input_height = encoder->output_height;

  const int fact_x = input_width * RSA / encoder->output_width;
  const int fact_y = input_height * RSA / encoder->output_height;
  const int fact   = input_height * RSA / encoder->output_height;
  const int MBX_IN = fact_x>>(RSA_SHFT-5);
  const int MBY_IN = fact_y>>(RSA_SHFT-5);

  if (fact_x != fact_y)
  {
    TAA_H264_ERROR (&encoder->error_handler, TAA_H264_INVALID_ARGUMENT);
    return 0;
  }

  taa_h264_resizer_init (encoder->resizer, fact);

  last_pos = -1;       /* Reset MBA */
  mbinfo_t currmb;
  const int stride = frameinfo->recon.stride;
  const int cstride = frameinfo->recon.cstride;
  const int height = frameinfo->height;
  const int width = frameinfo->width;
  const int ysize_in = input_width * input_height;
  const int csize_in = ysize_in / 4;

  for (int mby = 0; mby < mbymax; mby++)
  {
    // Magic for loading/resizing MB
    const unsigned int fineV = 2 * (16 * mby) * fact - RSA + fact;
    const int ypos_in = (MBY_IN==64) ? 64*mby : (fineV >> RSA_SHFT);
    const int cypos_in = ypos_in >> 1;

    for (int mbx = 0; mbx < mbxmax; mbx++)
    {
      // Magic for loading/resizing MB
      const unsigned int fineH = 2 * (16 * mbx) * fact - RSA + fact;
      const int xpos_in = (MBX_IN==64) ? 64 * mbx : (fineH >> RSA_SHFT);
      const int cxpos_in = xpos_in >> 1;

      const int pos = mby * mbxmax + mbx;
      const int xpos = mbx * MB_WIDTH_Y;
      const int ypos = mby * MB_HEIGHT_Y;
      const int cxpos = mbx * MB_WIDTH_UV;
      const int cypos = mby * MB_HEIGHT_UV;

      const uint8_t * in_y = input + ypos_in * input_width + xpos_in;
      const uint8_t * in_u = input + ysize_in + cypos_in * input_width/2 + cxpos_in;
      const uint8_t * in_v = input + ysize_in + csize_in + cypos_in * input_width/2 + cxpos_in;

      if ((input_width == encoder->output_width) && (mby < (mbymax-1)))
      {
        const int input_width_chroma = input_width >> 1;

        // Load Luma Y block
        _mm_store_pd((double *)(currmb.luma.input +  0*16), _mm_load_pd((double *)(in_y +  0*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  1*16), _mm_load_pd((double *)(in_y +  1*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  2*16), _mm_load_pd((double *)(in_y +  2*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  3*16), _mm_load_pd((double *)(in_y +  3*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  4*16), _mm_load_pd((double *)(in_y +  4*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  5*16), _mm_load_pd((double *)(in_y +  5*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  6*16), _mm_load_pd((double *)(in_y +  6*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  7*16), _mm_load_pd((double *)(in_y +  7*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  8*16), _mm_load_pd((double *)(in_y +  8*input_width)));
        _mm_store_pd((double *)(currmb.luma.input +  9*16), _mm_load_pd((double *)(in_y +  9*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 10*16), _mm_load_pd((double *)(in_y + 10*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 11*16), _mm_load_pd((double *)(in_y + 11*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 12*16), _mm_load_pd((double *)(in_y + 12*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 13*16), _mm_load_pd((double *)(in_y + 13*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 14*16), _mm_load_pd((double *)(in_y + 14*input_width)));
        _mm_store_pd((double *)(currmb.luma.input + 15*16), _mm_load_pd((double *)(in_y + 15*input_width)));

        // Load Chroma U and V block
        _mm_store_pd((double *)(currmb.chroma.input + 0*16),_mm_set_pd (*((double *)(in_v + 0*input_width_chroma)),*((double *)(in_u + 0*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 1*16),_mm_set_pd (*((double *)(in_v + 1*input_width_chroma)),*((double *)(in_u + 1*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 2*16),_mm_set_pd (*((double *)(in_v + 2*input_width_chroma)),*((double *)(in_u + 2*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 3*16),_mm_set_pd (*((double *)(in_v + 3*input_width_chroma)),*((double *)(in_u + 3*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 4*16),_mm_set_pd (*((double *)(in_v + 4*input_width_chroma)),*((double *)(in_u + 4*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 5*16),_mm_set_pd (*((double *)(in_v + 5*input_width_chroma)),*((double *)(in_u + 5*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 6*16),_mm_set_pd (*((double *)(in_v + 6*input_width_chroma)),*((double *)(in_u + 6*input_width_chroma))));
        _mm_store_pd((double *)(currmb.chroma.input + 7*16),_mm_set_pd (*((double *)(in_v + 7*input_width_chroma)),*((double *)(in_u + 7*input_width_chroma))));
      }
      else
      {
        taa_h264_load_resized_mb (
          encoder->resizer, &encoder->cpuinfo, in_y, in_u, in_v, currmb.luma.input,
          currmb.chroma.input, input_width, input_height, xpos_in, ypos_in, fineH, fineV);
      }
      memset(currmb.luma.do_4x4_enc, 0xff, 16);

      int new_qp = taa_h264_control_mb (
        encoder->control, encoder->last_coded_qp, num_bits_in_mb, (uint16_t) pos);

      mv_t *  motion_vectors_curr = &frameinfo->motion_vectors_curr[pos * NUM_MVS_MB];
      const mv_t * motion_vectors_prev = &frameinfo->motion_vectors_prev[pos * NUM_MVS_MB];
      fastpos_t *  motion_vectors_curr_16x16 = &frameinfo->motion_vectors_curr_16x16;

#ifdef NDEBUG
      if (encoder->cpuinfo.have_ssse3)
      {
        taa_h264_init_mb_ssse3 (
          stride,
          cstride,
          new_qp,
          xpos,
          ypos,
          cxpos,
          cypos,
          mbxmax,
          mbymax,
          height,
          width,
          intra_frame,
          frameinfo,
          &currmb,
          mby,
          mbx,
          first_mb_in_slice,
          motion_vectors_curr,
          motion_vectors_prev,
          motion_vectors_curr_16x16,
          frameinfo->meinfo,
          encoder->coding_options);
      }
      else
#endif
      {
        taa_h264_init_mb (
          stride,
          cstride,
          new_qp,
          xpos,
          ypos,
          cxpos,
          cypos,
          mbxmax,
          mbymax,
          height,
          width,
          intra_frame,
          frameinfo,
          &currmb,
          mby,
          mbx,
          first_mb_in_slice,
          motion_vectors_curr,
          motion_vectors_prev,
          motion_vectors_curr_16x16,
          frameinfo->meinfo,
          encoder->coding_options);
      }

      if (currmb.mbtype == P_SKIP)
      {
        motion_vectors_curr_16x16->x[pos] = currmb.mvskip.x;
        motion_vectors_curr_16x16->y[pos] = currmb.mvskip.y;

        for (int i = 0; i < NUM_MVS_MB; i++)
          motion_vectors_curr[i] = currmb.mvskip;
      }
      else
      {
#ifdef NDEBUG
        if (encoder->cpuinfo.have_ssse3)
        {
          taa_h264_encode_mb_ssse3 (frameinfo, &currmb, intra_frame, encoder->coding_options);
        }
        else
#endif
        {
          taa_h264_encode_mb (frameinfo, &currmb, intra_frame, encoder->coding_options);
        }
      }
      frameinfo->mbtypes[pos] = currmb.mbtype;

      /* Copy the reconstructed macroblock into reconstructed frame */
      uint8_t * recon_write_y = frameinfo->recon.y + ypos * stride + xpos;

      _mm_store_pd((double *)(recon_write_y +  0*stride), _mm_load_pd((double *)(currmb.luma.recon +  0*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  1*stride), _mm_load_pd((double *)(currmb.luma.recon +  1*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  2*stride), _mm_load_pd((double *)(currmb.luma.recon +  2*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  3*stride), _mm_load_pd((double *)(currmb.luma.recon +  3*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  4*stride), _mm_load_pd((double *)(currmb.luma.recon +  4*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  5*stride), _mm_load_pd((double *)(currmb.luma.recon +  5*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  6*stride), _mm_load_pd((double *)(currmb.luma.recon +  6*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  7*stride), _mm_load_pd((double *)(currmb.luma.recon +  7*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  8*stride), _mm_load_pd((double *)(currmb.luma.recon +  8*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y +  9*stride), _mm_load_pd((double *)(currmb.luma.recon +  9*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 10*stride), _mm_load_pd((double *)(currmb.luma.recon + 10*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 11*stride), _mm_load_pd((double *)(currmb.luma.recon + 11*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 12*stride), _mm_load_pd((double *)(currmb.luma.recon + 12*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 13*stride), _mm_load_pd((double *)(currmb.luma.recon + 13*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 14*stride), _mm_load_pd((double *)(currmb.luma.recon + 14*MB_WIDTH_Y)));
      _mm_store_pd((double *)(recon_write_y + 15*stride), _mm_load_pd((double *)(currmb.luma.recon + 15*MB_WIDTH_Y)));

      uint8_t * recon_write_u = frameinfo->recon.u + cypos * cstride + cxpos;
      uint8_t * recon_write_v = frameinfo->recon.v + cypos * cstride + cxpos;

      const __m128d xmm0 = _mm_load_pd((double *)(currmb.chroma.recon +  0*MB_STRIDE_UV));
      const __m128d xmm1 = _mm_load_pd((double *)(currmb.chroma.recon +  1*MB_STRIDE_UV));
      const __m128d xmm2 = _mm_load_pd((double *)(currmb.chroma.recon +  2*MB_STRIDE_UV));
      const __m128d xmm3 = _mm_load_pd((double *)(currmb.chroma.recon +  3*MB_STRIDE_UV));
      const __m128d xmm4 = _mm_load_pd((double *)(currmb.chroma.recon +  4*MB_STRIDE_UV));
      const __m128d xmm5 = _mm_load_pd((double *)(currmb.chroma.recon +  5*MB_STRIDE_UV));
      const __m128d xmm6 = _mm_load_pd((double *)(currmb.chroma.recon +  6*MB_STRIDE_UV));
      const __m128d xmm7 = _mm_load_pd((double *)(currmb.chroma.recon +  7*MB_STRIDE_UV));

      _mm_storel_pd((double *)&recon_write_u[0*cstride], xmm0);
      _mm_storeh_pd((double *)&recon_write_v[0*cstride], xmm0);
      _mm_storel_pd((double *)&recon_write_u[1*cstride], xmm1);
      _mm_storeh_pd((double *)&recon_write_v[1*cstride], xmm1);
      _mm_storel_pd((double *)&recon_write_u[2*cstride], xmm2);
      _mm_storeh_pd((double *)&recon_write_v[2*cstride], xmm2);
      _mm_storel_pd((double *)&recon_write_u[3*cstride], xmm3);
      _mm_storeh_pd((double *)&recon_write_v[3*cstride], xmm3);
      _mm_storel_pd((double *)&recon_write_u[4*cstride], xmm4);
      _mm_storeh_pd((double *)&recon_write_v[4*cstride], xmm4);
      _mm_storel_pd((double *)&recon_write_u[5*cstride], xmm5);
      _mm_storeh_pd((double *)&recon_write_v[5*cstride], xmm5);
      _mm_storel_pd((double *)&recon_write_u[6*cstride], xmm6);
      _mm_storeh_pd((double *)&recon_write_v[6*cstride], xmm6);
      _mm_storel_pd((double *)&recon_write_u[7*cstride], xmm7);
      _mm_storeh_pd((double *)&recon_write_v[7*cstride], xmm7);

      /* Write encoded macroblocks to bitstream */
      int mbrun = 0;
      bool force_no_slice_split = false;
      unsigned num_bits_current_mb;
      if (currmb.mbtype == P_SKIP)
      {
        (*num_skip_mbs)++;
        /* If we are at the last MB, we need to code the run of skips */
        mbrun = pos - last_pos;
        num_bits_current_mb = 0;
        if (pos == mbymax * mbxmax - 1)
        {
          num_bits_current_mb = TAA_H264_WRITE_UE (writer, mbrun, "mb_skip_run");
          /* Force no splitting of the slice if we are at the last mb
           * and it is of skip type. The mb_skip_run must be encoded
           * in the stream anyway, also when splitting. (In some cases
           * we would save two bits, but the extra_bits_compensation
           * makes this unnecessary )*/
          force_no_slice_split = true;
        }
      }
      else
      {
        mbrun = pos - last_pos - 1;
        num_bits_current_mb = taa_h264_write_mb (frameinfo, &currmb, mbrun, intra_frame, &encoder->last_coded_qp, writer);
        //num_bits_in_slice += taa_h264_write_mb (frameinfo, &currmb, mbrun, intra_frame, &encoder->last_coded_qp, writer);
        last_pos = pos;
      }

      num_bits_in_slice += num_bits_current_mb;

      /* At this time, we don't know how many escape bytes will be
       * inserted into bytestream, so the exact number of produced bytes
       * is not known. We compensate for this by reducing max_nalu_size
       * by a number that seems to work well unless we are extremely
       * unlucky. The RBSP trailing bits may also add an additional byte
       * to the current length. */
      const int extra_bits_compensation = 8 * 8;

      if (num_bits_in_slice > encoder->max_nalu_size * 8 - extra_bits_compensation
          && force_no_slice_split == false)
      {
        taa_h264_load_writer_state (writer);

        if (currmb.mbtype == P_SKIP)
        {
          mbrun--;
          (*num_skip_mbs)--;
        }

        if (mbrun > 0)
        {
          /* If last MBs were skipped, we need to write the
           * mb_skip_run value at the end of this slice. */
          TAA_H264_WRITE_UE (writer, mbrun, "mb_skip_run");
        }


        unsigned current_nal_size;
        taa_h264_send_slice (writer,
                             encoder->callbacks.output_nalu,
                             encoder->frameinfo.long_term_idx,
                             encoder->frameinfo.frame_num,
                             false,
                             encoder->callbacks.context,
                             max_packet_size,
                             &encoder->max_nalu_size,
                             &current_nal_size);

#ifdef TAA_SAVE_264_MEINFO
        if(encoder->debug_flag)
        {
          encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,
                "    SliceEnd -->  num_bits_in_slice=%d current_nal_size=%d;\n    }\n",
                num_bits_in_slice, current_nal_size);
        }
#endif

        mbx--;
        last_pos = pos - 1;

        /* Reset arrays written to during this MB. */
        if (MB_TYPE_IS_INTRA (currmb.mbtype))
        {
          for (int q = 0; q < NUM_4x4_BLOCKS_Y; q++)
            frameinfo->intra_modes[currmb.mbpos * NUM_4x4_BLOCKS_Y + q] = DC_PRED;
        } else
        {
          motion_vectors_curr_16x16->x[pos] = 0;
          motion_vectors_curr_16x16->y[pos] = 0;

          for (int q = 0; q < NUM_MVS_MB; q++)
          {
            motion_vectors_curr[q].ref_num = -1;
            motion_vectors_curr[q].x = 0;
            motion_vectors_curr[q].y = 0;
          }
        }

        uint8_t * num_coeffs_y_ptr = &frameinfo->num_coeffs_y[pos * NUM_4x4_BLOCKS_Y];
        uint8_t * num_coeffs_uv_ptr = &frameinfo->num_coeffs_uv[pos * NUM_4x4_BLOCKS_UV * 2];
        for (int k = 0; k < NUM_4x4_BLOCKS_Y; k++)
          num_coeffs_y_ptr[k] = 0;
        for (int k = 0; k < NUM_4x4_BLOCKS_UV * 2; k++)
          num_coeffs_uv_ptr[k] = 0;

        num_bits_in_slice = 0;
        num_bits_in_slice_prev = 0;
        first_mb_in_slice = pos;

#ifdef TAA_SAVE_264_MEINFO
        if(encoder->debug_flag)
        {
          encoder->data_position += sprintf(encoder->data_need_send + encoder->data_position,
                "    {\n    SliceStart -->  first_mb_in_slice=%d slice_qp=%d;\n", first_mb_in_slice, encoder->last_coded_qp);
        }
#endif

        num_bits_in_slice += taa_h264_write_slice_header (
          writer,
          sequence,
          idr_frame ? NALU_TYPE_IDR : NALU_TYPE_SLICE,
          intra_frame,
          frameinfo->frame_num,
          first_mb_in_slice,
          encoder->last_coded_qp,
          frameinfo->temporal_id,
          &frameinfo->ref_meta->rpm,
          frameinfo->rplr);
      }
      else
      {
        num_bits_in_mb = num_bits_in_slice - num_bits_in_slice_prev;
        num_bits_in_slice_prev = num_bits_in_slice;
      }
      taa_h264_save_writer_state (writer);

      /* HACK: QP storage should be redesigned. last_coded_qp at this point
       * holds current quant or previous quant for skip. */
      taa_h264_store_mbinfo (mbxmax, mbx, mby, currmb.mbtype, currmb.avail_flags,
                             currmb.mbcoeffs.luma_cbp, encoder->last_coded_qp, 0, 0, 0,
                             motion_vectors_curr,
#ifdef TAA_SAVE_264_MEINFO
                             num_bits_current_mb,
#endif
                             &frameinfo->mbinfo_stored[currmb.mbpos]);

#ifdef TAA_SAVE_264_MEINFO
      if (mbx == 0 && mby == 0)
      {
        max_qp_in_frame = 0;
        min_qp_in_frame = 100;
        qp_in_frame_sum = 0;
        available_qp_in_frame_num = 0;
      }
      else
      {

        if(new_qp > max_qp_in_frame)
          max_qp_in_frame = new_qp;
        if(new_qp < min_qp_in_frame)
          min_qp_in_frame = new_qp;
        qp_in_frame_sum += new_qp;
        available_qp_in_frame_num++;
      }
#endif
    }
  }

  unsigned current_nal_size;
  taa_h264_send_slice (writer,
                       encoder->callbacks.output_nalu,
                       encoder->frameinfo.long_term_idx,
                       encoder->frameinfo.frame_num,
                       true,
                       encoder->callbacks.context,
                       max_packet_size,
                       &encoder->max_nalu_size,
                       &current_nal_size);

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
  {
    encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,  "    SliceEnd --> num_bits_in_slice=%d current_nal_size=%d;\n    }\n", num_bits_in_slice, current_nal_size);
  }
#endif

#ifdef NDEBUG
  if (encoder->cpuinfo.have_ssse3)
  {
    taa_h264_deblock_frame_ssse3 (&frameinfo->recon,
                                  frameinfo->mbinfo_stored,
                                  frameinfo->mb_deblock_info,
                                  frameinfo->motion_vectors_curr,
                                  mbymax, mbxmax, frameinfo->resolution);
  }
  else
#endif
  {
    taa_h264_deblock_frame (&frameinfo->recon,
                            frameinfo->mbinfo_stored,
                            frameinfo->mb_deblock_info,
                            frameinfo->motion_vectors_curr,
                            mbymax, mbxmax, frameinfo->resolution);
  }

#if REF_FRAME_CHECKSUM
  taa_h264_dpb_set_checksum (&encoder->dpb);
#endif
  taa_h264_enc_dpb_update (&encoder->dpb,
                           &frameinfo->frame_num,
                           frameinfo->temporal_id);

#ifdef TAA_SAVE_264_MEINFO
  // Calculate PSNR which is a measure of image quality.
  //if(encoder->debug_flag)
  if(0)
  {

    double psnr_out =  calc_PSNR(input,
                        frameinfo->recon.y,
                        input_width,
                        frameinfo->recon.stride,
                        input_height);
    psnr[0] = psnr_out * 1000;

    psnr_out =  calc_PSNR(input + ysize_in,
                                 frameinfo->recon.u,
                                 input_width / 2,
                                 frameinfo->recon.cstride,
                                 input_height / 2);
    psnr[1] = psnr_out * 1000;

    psnr_out =  calc_PSNR(input + ysize_in + csize_in,
                          frameinfo->recon.v,
                          input_width / 2,
                          frameinfo->recon.cstride,
                          input_height / 2);
    psnr[2] = psnr_out * 1000;

  }
  else
  {
    psnr[0] = 40 * 1000;
    psnr[1] = 40 * 1000;
    psnr[2] = 40 * 1000;
  }

  if(encoder->debug_flag)
  {
    *max_qp_in_frame_out = max_qp_in_frame;
    *min_qp_in_frame_out = min_qp_in_frame;
    *avg_qp_in_frame_out = qp_in_frame_sum / available_qp_in_frame_num;
  }
#endif

#ifdef TAA_DEBUG_SAVE_MEINFO
  {
    const int luma_size = frameinfo->height * frameinfo->width;
    const int chroma_size = frameinfo->height * frameinfo->width / 4;
    FILE * fd;
    fd = fopen("cam_speed.yuv", "wb+");
    fwrite(input, 1, (luma_size + 2 * chroma_size), fd);
    fclose(fd);
  }
  {
    const int luma_size = frameinfo->ip_prev->full.rows * frameinfo->ip_prev->full.cols;
    const int chroma_size = frameinfo->ip_prev->full.rows * frameinfo->ip_prev->full.cols / 4;
    FILE * fd;
    fd = fopen("ref_speed.yuv", "wb+");
    fwrite(frameinfo->ip_prev->full.data, 1, (luma_size + 2 * chroma_size), fd);
    fclose(fd);
  }
  {
    FILE * fd;
    fd = fopen("test_enc_me_speed_inc.h", "w+");

    const int mv_size = frameinfo->height * frameinfo->width / 16;

    /* Save CURR motion vectors to file */
    fprintf(fd, "const mv_t motion_vectors_curr[%d] = {", mv_size);
    for(int i = 0; i < mv_size; i++)
    {
      if ((i % 16) == 0)
        fprintf(fd, "\n");
      fprintf(fd, "{%d,%d,%d},", frameinfo->motion_vectors_curr[i].x, frameinfo->motion_vectors_curr[i].y, frameinfo->motion_vectors_curr[i].ref_num);
    }
    fprintf(fd, "\n};\n\n");

    /* Save PREV motion vectors to file */
    fprintf(fd, "const mv_t motion_vectors_prev[%d] = {", mv_size);
    for(int i = 0; i < mv_size; i++)
    {
      if ((i % 16) == 0)
        fprintf(fd, "\n");
      fprintf(fd, "{%d,%d,%d},", frameinfo->motion_vectors_prev[i].x, frameinfo->motion_vectors_prev[i].y, frameinfo->motion_vectors_prev[i].ref_num);
    }
    fprintf(fd, "\n};\n\n");

    /* Save mb_types to file for assertion */
    const int num_mbs = frameinfo->height * frameinfo->width / MB_SIZE_Y;
    fprintf(fd, "const mbtype_t mb_types[%d] = {", num_mbs);
    for(int i = 0; i < num_mbs; i++)
    {
      if ((i % 64) == 0)
        fprintf(fd, "\n");
      fprintf(fd, "%d,", frameinfo->mbtypes[i]);
    }
    fprintf(fd, "\n};\n");

    /* Save availbility flags in order to get the same slice boundaries. This is in fact*/
    fprintf(fd, "const int avail_flags[%d] = {", num_mbs);
    for(int i = 0; i < num_mbs; i++)
    {
      if ((i % 16) == 0)
        fprintf(fd, "\n");
      fprintf(fd, "%d,", saved_avail_flags[i]);
    }
    fprintf(fd, "\n};\n");
    fflush(fd);
  }
#endif
  return TAA_H264_OK;
}

#ifdef TAA_SAVE_264_MEINFO
/* Set *hi and *lo to the high and low order bits of the cycle counter.
 * Implementation requires assembly code to use the rdtsc instruction. */
void AccessCounter(unsigned *hi, unsigned *lo){
    __asm__ volatile("rdtsc; movl %%edx,%0; movl %%eax, %1"
    : "=r" (*hi), "=r" (*lo)
    : /* No input */
    : "%edx", "%eax");
}

typedef uint64_t cycle_t;
/* Record the current value of the cycle counter. */
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

inline cycle_t CurrentCycle2(void){
    unsigned hi, lo;  
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((cycle_t)lo) | (((cycle_t)hi)<<32);  
}
#endif

int taa_h264_process_frame (
  encoder_t *        encoder,
  const uint8_t *    input,
  const int          input_width,
  const int          input_height,
  const unsigned     intra_period,
  const uint8_t      gop_size,
  bool               idr_frame,
  const uint8_t      min_qp,
  const uint8_t      max_qp,
  const uint8_t      max_qpi,
  const uint8_t      fixed_qp,
  const uint16_t     bitrate,
  const uint8_t      framerate,
  const uint32_t     max_mbps,
  const uint32_t     max_static_mbps,
  const bool         enable_frame_skip,
  const int          active_temporal_levels,
  const flux_t *     flux,
  const uint32_t     max_packet_size,
  const unsigned     coding_options,
  uint8_t *          outbuf)
{

#ifdef TAA_DEBUG_READ_YUV_FILE
  // Read yuv data from file instead of camear.
  // It's only for test.
  if(encoder->used_yuvfile_flag)
  {
    unsigned int ysize, csize;
    int framesize;
    ysize = encoder->yuv_w * encoder->yuv_h;
    csize = ysize / 4;
    framesize = ysize + 2 * csize;
    uint8_t cur_pos;
    uint8_t total_frame;

    if (encoder->fp != NULL)
    {
        if((input_width == encoder->yuv_w) && (input_height == encoder->yuv_h))
        {
          fread(input, sizeof(unsigned char), framesize, encoder->fp);
        }
    }
    if(encoder->cur_pos >= encoder->total_frame)
    {
      fseek(encoder->fp, 0, SEEK_SET);
      encoder->cur_pos = 0;
    }
    encoder->cur_pos++;
  }
#endif

#ifdef TAA_SAVE_264_MEINFO
  struct timeval start_time;
  struct timeval end_time;
  struct timeval start_time_pre;
  struct timeval end_time_pre;
  cycle_t tStartCyc_s, tStartCyc_e;
#endif

  bitwriter_t * writer   = encoder->bitwriter;
  frameinfo_t * frameinfo = &encoder->frameinfo;

#ifdef TAA_SAVE_264_MEINFO
  start_time_pre = start_time;
  end_time_pre = end_time;
  gettimeofday(&start_time, NULL);
  unsigned long long interval_time = (start_time.tv_sec - start_time_pre.tv_sec) * 1000 + (start_time.tv_usec - start_time_pre.tv_usec) / 1000;

  tStartCyc_s = rdtsc();
#endif

  encoder->coding_options = coding_options;
  encoder->max_nalu_size = max_packet_size;
  taa_h264_reset_writer (writer, outbuf, false);

  if (flux && flux->message)
  {
    /* Send flux message as early as possible. Even send flux messages if rate
     * control says "skip this frame", therefore we do this before the rate
     * control.*/
    taa_h264_write_user_data_sei (writer, flux->message, flux->message_len);
    taa_h264_send_nonslice (writer,
                            encoder->callbacks.output_nalu,
                            encoder->callbacks.context,
                            max_packet_size,
                            &encoder->max_nalu_size);
  }

  bool skip_frame;
  bool intra_frame;
  uint8_t temporal_id;

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
    encoder->data_position = sprintf(encoder->data_need_send + 0,  "{\n    FrameIdx --> id=%d time_second=%d    time_microsecond=%d;\n",encoder->frame_idx++, start_time.tv_sec, start_time.tv_usec);
#endif

  temporal_id = taa_h264_control_frame_type (
    encoder->control, enable_frame_skip, idr_frame, intra_period, gop_size,
    active_temporal_levels, &skip_frame, &intra_frame);

  if (skip_frame)
  {
#ifdef TAA_SAVE_264_MEINFO
    gettimeofday(&end_time, NULL);
    if(encoder->debug_flag)
    {
      unsigned long long escaped_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
      tStartCyc_e = rdtsc();
      unsigned long long cycle_escaped = (unsigned long long)(tStartCyc_e - tStartCyc_s);
      encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,
                                        "    SkipFrame --> escaped_cycle=%llu interval_time=%llu escaped_time=%llu;\n}\n", cycle_escaped, interval_time,escaped_time);

      if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
      #ifdef ENV_WIN32
      if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1, 0) == SOCKET_ERROR) printf("ERROR writing to socket");
      #else
      if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
      #endif
    }
#endif

    return TAA_H264_OK | TAA_H264_FRAME_SKIPPED;
  }

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
    encoder->data_position += sprintf(encoder->data_need_send + encoder->data_position,
          "    StartFrame --> ");
#endif

  ref_control_t ref_control;
  if (!intra_frame)
  {
    /* Force intra if no suitable reference is found */
    idr_frame = !taa_h264_decide_reference_frame (
      frameinfo->refframes,
      &encoder->dpb,
      frameinfo->frame_num + 1, // frame_num for this frame is not updated yet
      temporal_id,
      encoder->control->num_layers,
      flux,
      &ref_control);

    if (idr_frame)
    {
      /* Notify the controller about this frame being forced to intra */
      temporal_id = taa_h264_control_frame_type (
        encoder->control, enable_frame_skip, idr_frame, intra_period, gop_size,
        active_temporal_levels, &skip_frame, &intra_frame);

      TAA_H264_DEBUG_ASSERT (skip_frame == false);
      TAA_H264_DEBUG_ASSERT (intra_frame == true);
    }
#if REF_FRAME_CHECKSUM
    else if (flux && flux->recover && !intra_frame)
    {
      // We have a super-p frame, send checksum of reference frame for debugging
      decoded_picture_t * dp = encoder->dpb.ref_list[ref_control.ref_idx];
      uint32_t checksum = dp->checksum;
      int ltfi = dp->picid;
      TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES,
        "super p: frame-num=%d, age=%d, ltfi=%d, original-frame-num=%d\n",
        frameinfo->frame_num, ((ref_meta_t*)dp->meta)->age, ltfi, dp->original_frame_num);
      taa_h264_write_checksum (encoder->bitwriter, checksum, ltfi, dp->original_frame_num);
      taa_h264_send_nonslice (writer, encoder->callbacks.output_nalu,
        encoder->callbacks.context, max_packet_size, &encoder->max_nalu_size);
    }
#endif

#ifdef TAA_SAVE_264_MEINFO
    if(idr_frame)
    {
      if(encoder->debug_flag)
        encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position, "frameType=%s", "Intra-IDR");
    }
#endif
  }

  uint8_t curr_max_qp = max_qp;
  if (intra_frame && max_qpi != TAA_H264_QP_NOT_SPECIFIED)
    curr_max_qp = max_qpi;

  taa_h264_control_start_frame (
    encoder->control, temporal_id, fixed_qp, framerate, bitrate,
    min_qp, curr_max_qp, max_mbps, max_static_mbps
#ifdef TAA_SAVE_264_MEINFO
    ,encoder->debug_flag
    ,encoder->fp_meta_info
    ,encoder->data_need_send
    ,&(encoder->data_position)
#endif
    );

  if (intra_frame)
  {
#ifdef TAA_SAVE_264_MEINFO
    if(encoder->debug_flag)
      encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,
            "frameType=%s ","Intra");
#endif
    init_intra (frameinfo, &encoder->dpb);
  }
  else
  {
#ifdef TAA_SAVE_264_MEINFO
    if(encoder->debug_flag)
      encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,
            "frameType=%s ","Inter");
#endif
    init_inter (frameinfo, &encoder->dpb, &ref_control, temporal_id);
  }

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
    encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position, ";\n");
#endif

  int num_static_mbs = 0;
#ifdef TAA_SAVE_264_MEINFO
  int psnr[3];
  unsigned max_qp_in_frame, min_qp_in_frame, avg_qp_in_frame;
#endif
  int ret = taa_h264_process_frame2 (encoder,
                                     input,
                                     input_width,
                                     input_height,
                                     intra_frame,
                                     flux,
                                     max_packet_size,
                                     outbuf,
                                     &num_static_mbs
#ifdef TAA_SAVE_264_MEINFO
                                     ,psnr
                                     ,&min_qp_in_frame
                                     ,&max_qp_in_frame
                                     ,&avg_qp_in_frame
#endif
                                     );
#ifdef TAA_SAVE_264_MEINFO
  int skip_frame_debug;
  int skip_mbps_debug;
  int skip_bits_debug;
  int length_compensate_ratio_debug;
#endif

  taa_h264_control_end_frame (encoder->control, num_static_mbs
#ifdef TAA_SAVE_264_MEINFO
                              , &skip_frame_debug, &skip_mbps_debug, &skip_bits_debug, &length_compensate_ratio_debug,
                              encoder->meta_detail_flag
#endif
                              );

#ifdef TAA_SAVE_264_MEINFO
    gettimeofday(&end_time, NULL);
  if(encoder->debug_flag)
  {
    unsigned int escaped_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;
    tStartCyc_e = rdtsc();
    unsigned long long cycle_escaped = (unsigned long long)(tStartCyc_e - tStartCyc_s);
    encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position,"    EndFrame --> escaped_cycle=%llu interval_time=%d escaped_time=%d skip_mbs=%d next_skip_frame=%d skip_mbps=%d skip_bits=%d compensate=%d  min_qp_in_frame=%d max_qp_in_frame=%d avg_qp_in_frame=%d psnr_y=%d psnr_u=%d psnr_v=%d;\n",
            cycle_escaped,
            interval_time,escaped_time,
            num_static_mbs,
            skip_frame_debug,
            skip_mbps_debug,
            skip_bits_debug,
            length_compensate_ratio_debug,
            min_qp_in_frame,
            max_qp_in_frame,
            avg_qp_in_frame,
            psnr[0], psnr[1], psnr[2]);
    encoder->data_position += sprintf(encoder->data_need_send  + encoder->data_position, "}\n");
    if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
    #ifdef ENV_WIN32
    if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1, 0) == SOCKET_ERROR) printf("ERROR writing to socket");
    #else
    if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
    #endif
  }
#endif

  return ret;
}


static bool taa_h264_frameinfo_create (
  frameinfo_t *     frameinfo,
  int               width,
  int               height,
  error_handler_t * eh)
{
  error_codes_t err = TAA_H264_NO_ERROR;
  const int totalmbs = width * height / MB_SIZE_Y;
  frameinfo->width = width;
  frameinfo->height = height;
  frameinfo->approx_i4x4_penalty = 0;

  if     (width ==  128 && height ==  96) frameinfo->resolution = RES_SQCIF;
  else if(width ==  176 && height == 144) frameinfo->resolution =  RES_QCIF;
  else if(width ==  352 && height == 240) frameinfo->resolution =   RES_SIF;
  else if(width ==  352 && height == 288) frameinfo->resolution =   RES_CIF;
  else if(width ==  512 && height == 288) frameinfo->resolution =  RES_WCIF;
  else if(width ==  576 && height == 448) frameinfo->resolution = RES_25CIF;
  else if(width ==  640 && height == 480) frameinfo->resolution =   RES_VGA;
  else if(width ==  704 && height == 480) frameinfo->resolution =  RES_4SIF;
  else if(width ==  768 && height == 448) frameinfo->resolution = RES_W3CIF;
  else if(width ==  704 && height == 576) frameinfo->resolution =  RES_4CIF;
  else if(width ==  800 && height == 600) frameinfo->resolution =  RES_SVGA;
  else if(width == 1024 && height == 576) frameinfo->resolution = RES_W4CIF;
  else if(width == 1024 && height == 768) frameinfo->resolution =   RES_XGA;
  else if(width == 1280 && height == 720) frameinfo->resolution =  RES_720P;
  else if(width == 1920 && height == 1088) frameinfo->resolution = RES_1080P;
  else frameinfo->resolution = RES_UNKNOWN;

  const int size = sizeof frameinfo->long_term_meta_list / sizeof *frameinfo->long_term_meta_list;
  for (int i = 0; i < size; i++)
    frameinfo->long_term_meta_list[i] = NULL;

  // Initialize pointers to NULL so that in case of an error we easily know what
  // is allocated and not
  frameinfo->num_coeffs_y = NULL;
  frameinfo->num_coeffs_uv = NULL;
  frameinfo->intra_modes = NULL;
  frameinfo->mbtypes = NULL;
  frameinfo->motion_vectors_curr = NULL;
  frameinfo->motion_vectors_prev = NULL;
  frameinfo->motion_vectors_curr_16x16.data = NULL;
  frameinfo->motion_vectors_prev_16x16.data = NULL;
  frameinfo->mbinfo_stored = NULL;
  frameinfo->meinfo = NULL;
  frameinfo->mb_deblock_info = NULL;
  frameinfo->mb_deblock_info_size = 0;

  if (!(frameinfo->num_coeffs_y = TAA_H264_MALLOC (totalmbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_Y;
    goto alloc_failed;
  }

  if (!(frameinfo->num_coeffs_uv = TAA_H264_MALLOC (totalmbs * NUM_4x4_BLOCKS_UV * 2 * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_UV;
    goto alloc_failed;
  }

  if (!(frameinfo->intra_modes = TAA_H264_MALLOC (totalmbs * NUM_4x4_BLOCKS_Y * sizeof (uint8_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_INTRA_MODES;
    goto alloc_failed;
  }

  if (!(frameinfo->mbtypes = TAA_H264_MALLOC (totalmbs * sizeof (mbtype_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MBTYPES;
    goto alloc_failed;
  }

  if (!(frameinfo->motion_vectors_curr = TAA_H264_MALLOC (totalmbs * NUM_MVS_MB * sizeof(mv_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR;
    goto alloc_failed;
  }

  if (!(frameinfo->motion_vectors_prev = TAA_H264_MALLOC (totalmbs * NUM_MVS_MB * sizeof(mv_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV;
    goto alloc_failed;
  }

  if (!(frameinfo->motion_vectors_curr_16x16.data = TAA_H264_MALLOC ((2 * totalmbs + 64) * sizeof(int16_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR_16X16;
    goto alloc_failed;
  }

  if (!(frameinfo->motion_vectors_prev_16x16.data = TAA_H264_MALLOC ((2 * totalmbs + 64) * sizeof(int16_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV_16X16;
    goto alloc_failed;
  }

  memset (frameinfo->motion_vectors_curr, 0, totalmbs * NUM_MVS_MB * sizeof(mv_t));
  memset (frameinfo->motion_vectors_prev, 0, totalmbs * NUM_MVS_MB * sizeof(mv_t));
  memset (frameinfo->motion_vectors_curr, 0, (2 * totalmbs + 64) * sizeof(int16_t));
  memset (frameinfo->motion_vectors_curr, 0, (2 * totalmbs + 64) * sizeof(int16_t));

  frameinfo->motion_vectors_curr_16x16.x = frameinfo->motion_vectors_curr_16x16.data + 0 * totalmbs + 8;
  frameinfo->motion_vectors_curr_16x16.y = frameinfo->motion_vectors_curr_16x16.data + 1 * totalmbs + 8 + 8;
  frameinfo->motion_vectors_prev_16x16.x = frameinfo->motion_vectors_prev_16x16.data + 0 * totalmbs + 8;
  frameinfo->motion_vectors_prev_16x16.y = frameinfo->motion_vectors_prev_16x16.data + 1 * totalmbs + 8 + 8;

  if (!(frameinfo->mbinfo_stored = TAA_H264_MALLOC (totalmbs * sizeof(mbinfo_store_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MBINFO_STORED;
    goto alloc_failed;
  }

  if (!(frameinfo->meinfo = taa_h264_meinfo_create ()))
  {
    err = TAA_H264_ALLOCATION_ERROR_MEINFO;
    goto alloc_failed;
  }

  frameinfo->mb_deblock_info_size = (frameinfo->width / MB_WIDTH_Y) * (frameinfo->height / MB_HEIGHT_Y);
  if (!(frameinfo->mb_deblock_info = TAA_H264_MALLOC (frameinfo->mb_deblock_info_size * sizeof(mb_deblock_t))))
  {
    err = TAA_H264_ALLOCATION_ERROR_MB_DEBLOCK_INFO;
    goto alloc_failed;
  }

  return true;

alloc_failed:
  TAA_H264_CRITICAL (eh, err);

  TAA_H264_FREE (frameinfo->num_coeffs_y);
  TAA_H264_FREE (frameinfo->num_coeffs_uv);
  TAA_H264_FREE (frameinfo->intra_modes);
  TAA_H264_FREE (frameinfo->mbtypes);
  TAA_H264_FREE (frameinfo->motion_vectors_curr);
  TAA_H264_FREE (frameinfo->motion_vectors_prev);
  TAA_H264_FREE (frameinfo->motion_vectors_curr_16x16.data);
  TAA_H264_FREE (frameinfo->motion_vectors_prev_16x16.data);
  TAA_H264_FREE (frameinfo->mbinfo_stored);
  TAA_H264_FREE (frameinfo->meinfo);
  TAA_H264_FREE (frameinfo->mb_deblock_info);

  frameinfo->mb_deblock_info_size = 0;

  return false;
}


/**
 * Set the width and height by rounding up to the nearest multiple of 16.
 */
static void taa_h264_set_framesize (
  int * width,
  int * height)
{
  *width = ((*width + 15) / 16) * 16;
  *height = ((*height + 15) / 16) * 16;
}

static level_t valid_level (int level)
{
  level_t ret;
  switch (level)
  {
  case LEVEL_1_0:
  case LEVEL_1_1:
  case LEVEL_1_2:
  case LEVEL_1_3:
  case LEVEL_2_0:
  case LEVEL_2_1:
  case LEVEL_2_2:
  case LEVEL_3_0:
  case LEVEL_3_1:
  case LEVEL_3_2:
  case LEVEL_4_0:
  case LEVEL_4_1:
  case LEVEL_4_2:
  case LEVEL_5_0:
  case LEVEL_5_1:
    ret = (level_t) level;
    break;
  default:
    ret = LEVEL_4_0;
    break;
  }
  return ret;
}




bool taa_h264_enc_init (
  encoder_t * encoder,
  int         width,
  int         height,
  uint16_t    sar_width,
  uint16_t    sar_height,
  int         num_ref_frames,
  int         temporal_levels,
  int         profile_level
#ifdef TAA_SAVE_264_MEINFO
  ,unsigned char debug_flag
  ,uint8_t     *key_word
  ,uint8_t    meta_detail_flag
  ,char debug_ip_addr[32]
  ,unsigned int debug_port
#endif
#ifdef TAA_DEBUG_READ_YUV_FILE
  ,unsigned used_yuvfile_flag
  ,FILE *fp
  ,unsigned total_frame
  ,unsigned yuv_w
  ,unsigned yuv_h
#endif

 )
{
  error_codes_t err = TAA_H264_NO_ERROR;

#ifdef TAA_DEBUG_READ_YUV_FILE
  encoder->used_yuvfile_flag = used_yuvfile_flag;
  encoder->fp = fp;
  encoder->total_frame = total_frame;
  encoder->yuv_w = yuv_w;
  encoder->yuv_h = yuv_h;
#endif

#ifdef TAA_SAVE_264_MEINFO
  encoder->frame_idx = 0;
  encoder->debug_flag = debug_flag;
  sprintf(encoder->debug_ip_addr, "%s", debug_ip_addr);
  encoder->debug_port = debug_port;
  snprintf(encoder->key_word, sizeof(encoder->key_word), "%s", key_word);
  encoder->meta_detail_flag = meta_detail_flag;

  if(encoder->debug_flag)
  {
    uint8_t temp_fp_name[64];
    snprintf(temp_fp_name, sizeof(temp_fp_name), "%s_out.meta", key_word);

    encoder->fp_meta_info = NULL;
    encoder->data_need_send = (uint8_t *)malloc(4096 * 20);

    if(encoder->fp_meta_info == NULL)
      printf("Open file %s failed\n",temp_fp_name);

#ifdef ENV_WIN32
    WSADATA wsaData;
    int iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
      printf("WSAStartup failed with error: %d\n", iResult);
      return false;
    }
#endif

    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return false;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(debug_port);

#ifdef ENV_WIN32
    serv_addr.sin_addr.s_addr = inet_addr(encoder->debug_ip_addr);
#else
    if(inet_pton(AF_INET, encoder->debug_ip_addr, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return false;
    }
#endif

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       close(sockfd);
       encoder->sockfd = -1;
    }
    else
    {
      encoder->sockfd = sockfd;
    }

    encoder->data_position = sprintf(encoder->data_need_send, "KeyWord --> token=%s;\n", encoder->key_word);
    if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
    #ifdef ENV_WIN32
    if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1, 0) == SOCKET_ERROR) printf("ERROR writing to socket");
    #else
    if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
    #endif
    

    encoder->data_position = sprintf(encoder->data_need_send, "Init_parameter --> Width=%d Height=%d num_ref_frames=%d temporal_levels=%d profile_level=%d;\n",
            width, height, num_ref_frames, temporal_levels, profile_level);
    if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
    #ifdef ENV_WIN32
    if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1, 0) == SOCKET_ERROR) printf("ERROR writing to socket");
    #else
    if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
    #endif    
  }
#endif

  // (STJ) uncomment to enable encoder/decoder (joint) logging
  //FILE* file;
  //file = fopen ("encoder.log", "w+");
  //TAA_H264_INIT_TRACE (16, file);

  taa_h264_get_cpuinfo (&encoder->cpuinfo);

  /* Only even output resolutions are supported. We could silently round down
   * to nearest even number, but we would likely get a different aspect ratio
   * on input and output. Support for this is not implemented. */
  if (width % 2 || height % 2)
  {
    TAA_H264_ERROR (&encoder->error_handler, TAA_H264_INVALID_ARGUMENT);
    return false;
  }
  
  if (width > MAX_FRAME_WIDTH || height > MAX_FRAME_HEIGHT  || height * width > MAX_FRAME_AREA)
  {
    TAA_H264_ERROR (&encoder->error_handler, TAA_H264_UNSUPPORTED_RESOLUTION);
    return false;
  }

  encoder->output_width = width;
  encoder->output_height = height;
  taa_h264_set_framesize (&width, &height);

  /* We need to initialize the ratecontrol with a realistic bitrate for this
   * resolution. We don't need a very accurate formula so let's keep it
   * simple with a magic bits-per-pixel value. We also initialize with a very
   * high QP as we are pretty sure that the first frame will overproduce bits
   * since it's an intra. This will cause a low quality intra frame but
   * significantly less burst. */

  /* kshiffer: 0.041 is the smallest bits-per-pixel we use.  This value ensures
   * PSNR of 41dB for 1280x720 (or larger) at 30fps.  For smaller resolutions, 
   * we need a higher bits-per-pixel value.  But okay to use 0.041 here since 
   * this is just for initialization.  The encoder will be updated to the real
   * target bitrate later. */
  const float bits_per_pixel = 0.041;	
  const uint8_t default_framerate = 30;
  // Workaround to trick the compiler to not use fisttp instruction
  const int default_bitrate_int =
    (int) (width * height * default_framerate * bits_per_pixel / 1024.0f);
  uint16_t default_bitrate = 0;
  if(default_bitrate_int < 0xFFFF)
  {
     default_bitrate = default_bitrate_int;
  }
  else
  {
     default_bitrate = 0xFFFF;//65535
  }

  const uint8_t default_initial_qp = 43;
  const uint8_t default_min_qp = 23;
  const uint8_t default_max_qp = 43;
  const uint8_t default_gop_size = 1;

#ifndef AVG_HPEL
  const int num_bufs_per_frame = 4;
  const bool store_interpolated = true;
#else
  const int num_bufs_per_frame = 1;
  const bool store_interpolated = false;
#endif
  const int totalmbs = width * height / MB_SIZE_Y;

  const int padded_width = width + PAD_HORZ_Y  * 2;
  const int padded_height = height + PAD_VERT_Y * 2;
  const int buf_size_one = (padded_width * padded_height * 3 / 2) * sizeof (uint8_t);
  const int num_bufs = num_bufs_per_frame * (num_ref_frames + 1);
  const int mem_size = buf_size_one * num_bufs;

  encoder->sequence.sps_id = (uint8_t) -1;
  encoder->sequence.pps_id = (uint8_t) -1;
  encoder->sequence.idr_pic_id = (uint8_t) -1;
  encoder->sequence.num_ref_frames = (uint8_t) num_ref_frames;
  /* NOTE: Currently there is no check whether the combination of level, width,
   * height and number of reference frames actually is valid. Level is simply
   * used to set the value in the SPS. */
  encoder->sequence.level = valid_level (profile_level);
  encoder->sequence.crop_right = (uint8_t) (width - encoder->output_width);
  encoder->sequence.crop_bottom = (uint8_t) (height - encoder->output_height);
  encoder->sequence.sar_width = sar_width;
  encoder->sequence.sar_height = sar_height;
  encoder->sequence.num_frames_since_intra = 1;

  // Initialize pointers so that in case of an allocation error we know what
  // to free.
  encoder->dpb.mem_base = NULL;
  encoder->bitwriter = NULL;
  encoder->control = NULL;

  encoder->dpb.max_width = width;
  encoder->dpb.max_height = height;
  encoder->dpb.mem_base = TAA_H264_MM_MALLOC (mem_size, 64);
  if (!encoder->dpb.mem_base)
  {
    err = TAA_H264_ALLOCATION_ERROR_DPB_MEM_BASE;
    goto alloc_failed;
  }

  taa_h264_enc_dpb_init (&encoder->dpb,
                         encoder->refframes_meta,
                         (uint8_t) num_ref_frames,
                         width,
                         height,
                         store_interpolated,
                         &encoder->error_handler);

  taa_h264_reset_gop (&encoder->gop, default_gop_size, (uint8_t) temporal_levels);
  if (!taa_h264_frameinfo_create (&encoder->frameinfo,
                                  width,
                                  height,
                                  &encoder->error_handler))
  {
    err = TAA_H264_ALLOCATION_ERROR_FRAMEINFO_CREATE;
    goto alloc_failed;
  }

  encoder->bitwriter = taa_h264_bitwriter_create ();
  if (!encoder->bitwriter)
  {
    err = TAA_H264_ALLOCATION_ERROR_BITWRITER_CREATE;
    goto alloc_failed;
  }

  encoder->resizer = taa_h264_resizer_create();
  if (!encoder->resizer)
  {
    err = TAA_H264_ALLOCATION_ERROR_RESIZER_CREATE;
    goto alloc_failed;
  }

  encoder->control = taa_h264_control_create ((uint8_t) temporal_levels, (uint16_t) totalmbs);
  if (!encoder->control)
  {
    err = TAA_H264_ALLOCATION_ERROR_CONTROL_CREATE;
    goto alloc_failed;
  }

  taa_h264_control_init (
    encoder->control,
    default_gop_size,
    default_framerate,
    default_bitrate,
    default_initial_qp,
    default_min_qp,
    default_max_qp,
    (uint16_t) totalmbs);

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
  {
    encoder->data_position = sprintf(encoder->data_need_send,
                                  "control_info  --> num_layers=%d skip_frames=%d enable_frame_skip=%d fixed_qp=%d frame_qp=%d intra_period=%d frames_since_intra=%d;\n",
                                    encoder->control->num_layers,
                                    encoder->control->skip_frames,
                                    encoder->control->enable_frame_skip,
                                    encoder->control->fixed_qp,
                                    encoder->control->frame_qp,
                                    encoder->control->intra_period,
                                    encoder->control->frames_since_intra);
    if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
    #ifdef ENV_WIN32
    if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1,0) < 0) printf("ERROR writing to socket");
    #else
    if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
    #endif

    encoder->data_position = sprintf(encoder->data_need_send, "ratecontrol  -->  min_qp=%d max_qp=%d framerate=%d bitrate=%d;\n",
            encoder->control->ratecontrol->min_qp,
            encoder->control->ratecontrol->max_qp,
            encoder->control->ratecontrol->framerate,
            encoder->control->ratecontrol->bitrate);
    if(encoder->fp_meta_info != NULL) fprintf(encoder->fp_meta_info, "%s", encoder->data_need_send);
    #ifdef ENV_WIN32
    if(encoder->sockfd > 0) if(send(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1,0) < 0) printf("ERROR writing to socket");
    #else
    if(encoder->sockfd > 0) if(write(encoder->sockfd, encoder->data_need_send, encoder->data_position + 1) < 0) printf("ERROR writing to socket");
    #endif
  }
#endif

  return true;

alloc_failed:
  TAA_H264_CRITICAL (&encoder->error_handler, err);
  TAA_H264_MM_FREE (encoder->dpb.mem_base);
  taa_h264_bitwriter_delete (encoder->bitwriter);
  taa_h264_control_delete (encoder->control);
  // frameinfo cleans up after itself if there is an error
  return false;
}


void taa_h264_enc_free (
  encoder_t * encoder)
{
  frameinfo_t * frameinfo = &encoder->frameinfo;

  TAA_H264_FREE (frameinfo->num_coeffs_y);
  TAA_H264_FREE (frameinfo->num_coeffs_uv);
  TAA_H264_FREE (frameinfo->intra_modes);

  TAA_H264_FREE (frameinfo->mbtypes);
  TAA_H264_FREE (frameinfo->motion_vectors_curr);
  TAA_H264_FREE (frameinfo->motion_vectors_prev);
  TAA_H264_FREE (frameinfo->motion_vectors_curr_16x16.data);
  TAA_H264_FREE (frameinfo->motion_vectors_prev_16x16.data);
  TAA_H264_FREE (frameinfo->mbinfo_stored);
  TAA_H264_FREE (frameinfo->mb_deblock_info);

  frameinfo->mb_deblock_info_size = 0;

  TAA_H264_MM_FREE (encoder->dpb.mem_base);

  taa_h264_meinfo_delete (frameinfo->meinfo);
  taa_h264_control_delete (encoder->control);
  taa_h264_bitwriter_delete (encoder->bitwriter);
  taa_h264_resizer_delete (encoder->resizer);

#ifdef TAA_SAVE_264_MEINFO
  if(encoder->debug_flag)
  {
    if(encoder->fp_meta_info != NULL)
    {
      fclose(encoder->fp_meta_info);
      encoder->fp_meta_info = NULL;
    }
    if(encoder->sockfd > 0)
    {
#ifdef ENV_DARWIN
      close(encoder->sockfd);
#endif

#ifdef ENV_WIN32
      closesocket(encoder->sockfd);
      WSACleanup();
#endif

      encoder->sockfd  = -1;
    }
    free(encoder->data_need_send);
  }

#endif

}



