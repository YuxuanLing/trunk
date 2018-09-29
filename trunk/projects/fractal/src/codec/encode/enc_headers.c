#include "enc_headers.h"
#include "enc_config.h"
#include "enc_writebits.h"
#include "enc_putvlc.h"
#include "com_mbutils.h"
#include "com_intmath.h"


/* TODO: Separate Annex B functionality into encoder application */
static unsigned taa_h264_write_annexb_nalu_prefix (
  bitwriter_t * writer,
  bool          long_start_code)
{
  /* Byte stream NAL unit syntax (Annex B), sec B.1.1 */

  /* Gstreamer's rtph264pay element expects a long start code for all NALUs.
   * This is a bug compared to the standard, but let's just make a
   * workaround for now. */
  const bool workaround_gst_rtph264pay_bug = true;

  if (long_start_code || workaround_gst_rtph264pay_bug)
    taa_h264_write_byte_no_check (writer, 0x0);

  taa_h264_write_byte_no_check (writer, 0x0);
  taa_h264_write_byte_no_check (writer, 0x0);
  taa_h264_write_byte_no_check (writer, 0x1);

  return long_start_code ? 32 : 24;
}


nalref_t taa_h264_get_priority (
  nalutype_t type,
  int        temporal_id)
{
  nalref_t priority;

  switch (type)
  {
  case NALU_TYPE_SPS:
  case NALU_TYPE_PPS:
  case NALU_TYPE_IDR:
    /* Highest priority. Not able to decode without these. */
    priority = NALU_PRIORITY_HIGH;
    break;
  case NALU_TYPE_SLICE:
  case NALU_TYPE_SEI:
    /* Set priority based on temporal id. Lower temporal id means
     * higher importance since it's part of the lower framerate layer
     * and more than one frame probably refers to it. */

    /* TODO: Set NALU_PRIORITY_DISPOSABLE if no future frames will
     * reference this one. */
    priority = (nalref_t) max (NALU_PRIORITY_HIGH - temporal_id, NALU_PRIORITY_LOW);
    break;
  default:
    priority = NALU_PRIORITY_LOW;
    break;
  }

  return priority;
}


unsigned taa_h264_write_nalu_header (
  bitwriter_t * writer,
  nalref_t      idc,
  nalutype_t    type,
  bool          long_start_code)
{
  unsigned numbits = taa_h264_write_annexb_nalu_prefix (writer, long_start_code);
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* NAL UNIT HEADER *******\n\n");
  numbits += TAA_H264_WRITE_U (writer, 0, 1, "forbidden zero bit");
  numbits += TAA_H264_WRITE_U (writer, idc, 2, "nal_ref_idc");
  numbits += TAA_H264_WRITE_U (writer, type, 5, "nalu_type");
  return numbits;
}

static unsigned write_vui_parameters (
  bitwriter_t *      writer,
  const sequence_t * sequence)
{
  unsigned bits = 0;

  // The only reason for VUI parameters in our case is sample aspect ratio
  if (sequence->sar_width == 0 || sequence->sar_height == 0)
  {
    bits += TAA_H264_WRITE_U (writer, 0, 1, "vui_parameters_present_flag");
    return bits;
  }

  // Write SAR as "extended" for simplicity
  bits += TAA_H264_WRITE_U (writer, 1, 1, "vui_parameters_present_flag");
  bits += TAA_H264_WRITE_U (writer, 1, 1, "aspect_ratio_info_present_flag");
  bits += TAA_H264_WRITE_U (writer, 255, 8, "aspect_ratio_idc");
  bits += TAA_H264_WRITE_U (writer, sequence->sar_width, 16, "sar_width");
  bits += TAA_H264_WRITE_U (writer, sequence->sar_height, 16, "sar_height");

  // The rest is not present
  bits += TAA_H264_WRITE_U (writer, 0, 1, "overscan_info_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "video_signal_type_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "chroma_loc_info_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "timing_info_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "nal_hrd_parameters_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "vcl_hrd_parameters_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "pic_struct_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "bitstream_restriction_flag");

  return bits;
}

static unsigned taa_h264_write_sps_data (
  bitwriter_t *      writer,
  const sequence_t * sequence,
  profile_t          profile,
  unsigned               width,
  unsigned               height)
{
  int bits = 0;

  bits += TAA_H264_WRITE_U (writer, profile, 8, "profile_idc");
  bits += TAA_H264_WRITE_U (writer, 1, 1, "constraint_set0_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "constraint_set1_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "constraint_set2_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 5, "reserved_zero_5bits");
  bits += TAA_H264_WRITE_U (writer, sequence->level, 8, "profile_idc");
  bits += TAA_H264_WRITE_UE (writer, sequence->sps_id, "seq_parameter_set_id");
  if (profile == SCALABLE_BASELINE_PROFILE)
  {
    bits += TAA_H264_WRITE_UE (writer, 1, "chroma_format_idc");
    bits += TAA_H264_WRITE_UE (writer, 0, "bit_depth_luma_minus8");
    bits += TAA_H264_WRITE_UE (writer, 0, "bit_depth_chroma_minus8");
    bits += TAA_H264_WRITE_U (writer, 0, 1, "qpprime_y_zero_transform_bypass_flag");
    bits += TAA_H264_WRITE_U (writer, 0, 1, "seq_scaling_matrix_present_flag");
  }
  bits += TAA_H264_WRITE_UE (writer, LOG2_MAX_FRAME_NUM - 4, "log2_max_frame_num_minus4");
  /* picture order count 2 results in an output order that is the same as
   * the decoding order.  */
  bits += TAA_H264_WRITE_UE (writer, 2, "pic_order_cnt_type");
  bits += TAA_H264_WRITE_UE (writer, sequence->num_ref_frames, "num_ref_frames");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "gaps_in_frame_num_value_allowed_flag");
  bits += TAA_H264_WRITE_UE (writer, width / MB_WIDTH_Y - 1, "pic_width_in_mbs_minus1");
  bits += TAA_H264_WRITE_UE (writer, height / MB_HEIGHT_Y - 1, "pic_height_in_map_units_minus1");
  bits += TAA_H264_WRITE_U (writer, 1, 1, "frame_mbs_only_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "direct_8x8_inference_flag");
  if (sequence->crop_bottom == 0 && sequence->crop_right == 0)
  {
    bits += TAA_H264_WRITE_U (writer, 0, 1, "frame_cropping_flag");
  }
  else
  {
    bits += TAA_H264_WRITE_U (writer, 1, 1, "frame_cropping_flag");
    bits += TAA_H264_WRITE_UE (writer, 0, "frame_crop_left_offset");
    bits += TAA_H264_WRITE_UE (writer, sequence->crop_right / 2, "frame_crop_right_offset");
    bits += TAA_H264_WRITE_UE (writer, 0, "frame_crop_top_offset");
    bits += TAA_H264_WRITE_UE (writer, sequence->crop_bottom / 2, "frame_crop_bottom_offset");
  }
  bits += write_vui_parameters (writer, sequence);

  return bits;
}



unsigned taa_h264_write_sequence_parameter_set (
  bitwriter_t *      writer,
  const sequence_t * sequence,
  unsigned               width,
  unsigned               height)
{
  unsigned bits = 0;
  bits += taa_h264_write_nalu_header (
    writer,
    taa_h264_get_priority (NALU_TYPE_SPS, 0),
    NALU_TYPE_SPS,
    true);
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* SEQUENCE PARAMETER SET *******\n\n");
  bits += taa_h264_write_sps_data (writer, sequence, BASELINE_PROFILE, width, height);
  return bits;
}


unsigned taa_h264_write_picture_parameter_set (
  bitwriter_t *      writer,
  const sequence_t * sequence)
{
  unsigned bits = 0;
  bits += taa_h264_write_nalu_header (
    writer,
    taa_h264_get_priority (NALU_TYPE_PPS, 0),
    NALU_TYPE_PPS,
    true);
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* PICTURE PARAMETER SET *******\n\n");
  bits += TAA_H264_WRITE_UE (writer, sequence->pps_id, "pic_parameter_set_id");
  bits += TAA_H264_WRITE_UE (writer, sequence->sps_id, "seq_parameter_set_id");
  bits += TAA_H264_WRITE_U (writer, sequence->entropy_coding_mode_flag, 1, "entropy_coding_mode_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "pic_order_present_flag");
  bits += TAA_H264_WRITE_UE (writer, 0, "num_slice_groups_minus1");
  /* We always use just one active refrence frame. */
  bits += TAA_H264_WRITE_UE (writer, 0, "num_ref_idx_l0_active_minus1");
  bits += TAA_H264_WRITE_UE (writer, 0, "num_ref_idx_l1_active_minus1");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "weighted_pred_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 2, "weighted_bipred_idc");
  bits += TAA_H264_WRITE_SE (writer, sequence->pps_qp - 26, "pic_init_qp_minus26");
  bits += TAA_H264_WRITE_SE (writer, 0, "pic_init_qs_minus26");
  bits += TAA_H264_WRITE_SE (writer, 0, "chroma_qp_index_offset");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "deblocking_filter_control_present_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "constrained_intra_pred_flag");
  bits += TAA_H264_WRITE_U (writer, 0, 1, "redundant_pic_cnt_present_flag");
  return bits;
}


static int taa_h264_write_mmco (
  bitwriter_t *  writer,
  const mmco_t * mmco)
{
  int bits = 0;
  int i = 0;
  do
  {
    bits += TAA_H264_WRITE_UE (writer, mmco[i].op, "memory_management_control_operation");
    if (mmco[i].op != 0 && mmco[i].op != 5)
    {
#ifdef TAA_H264_TRACE_ENABLED
      const char * str;
      switch (mmco[i].op)
      {
      case 1:
      case 3:
        str = "difference_of_pic_nums_minus1";
        break;
      case 2:
        str = "long_term_pic_num";
        break;
      case 4:
        str = "max_long_term_frame_idx_plus1";
        break;
      case 6:
        str = "long_term_frame_idx";
        break;
      default:
        str = "";
        break;
      }
      bits += TAA_H264_WRITE_UE (writer, mmco[i].val1, str);
#else
      bits += TAA_H264_WRITE_UE (writer, mmco[i].val1, "");
#endif
      if (mmco[i].op == 3)
        bits += TAA_H264_WRITE_UE (writer, mmco[i].val2, "long_term_frame_idx");
    }
  } while (mmco[i++].op != 0);

  return bits;
}


int taa_h264_write_ref_pic_marking (
  bitwriter_t * writer,
  const rpm_t * rpm)
{
  int bits = 0;
  if (rpm->idr_flag)
  {
    bool lt_flag = rpm->mmco[0].op != 0;
    bits += TAA_H264_WRITE_U (writer, 0, 1, "no_output_of_prior_pics_flag");
    bits += TAA_H264_WRITE_U (writer, lt_flag, 1, "long_term_reference_flag");
  }
  else
  {
    bool adaptive_marking = rpm->mmco[0].op != 0;
    bits += TAA_H264_WRITE_U (writer, adaptive_marking, 1, "adaptive_ref_pic_marking_mode_flag");
    if (adaptive_marking)
      bits += taa_h264_write_mmco (writer, rpm->mmco);
  }
  return bits;
}

static int taa_h264_write_rplr (
  bitwriter_t *  writer,
  const rplr_t * rplr)
{
  int bits = 0;
  int i = 0;

  do
  {
    bits += TAA_H264_WRITE_UE (writer, rplr[i].idc, "reordering_of_pic_nums_idc");

    if (rplr[i].idc < 3)
      bits += TAA_H264_WRITE_UE (writer, rplr[i].val, rplr[i].idc == 2 ?
                                 "long_term_pic_num" : "abs_diff_pic_num_minus1");
  } while (rplr[i++].idc != 3);

  return bits;
}


unsigned taa_h264_write_slice_header (
  bitwriter_t *      writer,
  const sequence_t * sequence,
  nalutype_t         nalutype,
  bool               islice,
  int                frame_num,
  int                first_mb,
  int                slice_qp,
  int                temporal_id,
  const rpm_t *      rpm,
  rplr_t *           rplr)
{
  unsigned bits = 0;
  slicetype_t slicetype = islice ? SLICE_TYPE_I : SLICE_TYPE_P;
  bool new_frame = first_mb == 0;
  nalref_t priority = taa_h264_get_priority (nalutype, temporal_id);

  bits += taa_h264_write_nalu_header (
    writer,
    priority,
    nalutype,
    new_frame);

  /* For now, each slice contains a whole frame  */

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* SLICE HEADER *******\n\n");

  bits += TAA_H264_WRITE_UE (writer, first_mb, "first_mb_in_slice");
  /* The whole picture is of same type */
  bits += TAA_H264_WRITE_UE (writer, slicetype + 5, "slice_type");
  bits += TAA_H264_WRITE_UE (writer, sequence->pps_id, "pic_parameter_set_id");
  /* FIXME: wrapping of frame num and resetting on idr frames */
  bits += TAA_H264_WRITE_U (writer, frame_num, LOG2_MAX_FRAME_NUM, "frame_num");

  if (nalutype == NALU_TYPE_IDR)
    bits += TAA_H264_WRITE_UE (writer, sequence->idr_pic_id, "idr_pic_id");

  /* pic_order_cnt != 0 */
  /* pic_order_cnt != 1 */
  /* redundant_pic_cnt_present_flag != 1 */
  /* slicetype_t != B */

  if (slicetype == SLICE_TYPE_P)
  {
    /* Don't override the number of active ref frames, we always use
     * only one (which is set in the PPS). */
    bits += TAA_H264_WRITE_U (writer, 0, 1, "num_ref_idx_active_override_flag");
  }

  /* ref_pic_list_reordering() */
  if (slicetype == SLICE_TYPE_P)
  {
    bool rplr_flag = rplr[0].idc != 3;
    bits += TAA_H264_WRITE_U (writer, rplr_flag, 1, "ref_pic_list_reordring_flag_l0");
    if (rplr_flag)
      bits += taa_h264_write_rplr (writer, rplr);
  }

  /* weighted_pred_flag != 1 */
  /* weighted_bipred_idc == 0 */

  if (priority != NALU_PRIORITY_DISPOSABLE)
  {
    bits += taa_h264_write_ref_pic_marking (writer, rpm);
  }

  /* entropy_coding_mode_flag != 1 */
  //Bo Add cabac:

  bits += TAA_H264_WRITE_SE (writer, slice_qp - sequence->pps_qp, "slice_qp_delta");

  /* if deblocking_filter_control_present_flag: */
  /* disable deblocking filter */
  /* val = 1; */
  /* taa_h264_put_uvlc (writer, val); */
  /* TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, "disable_deblocking_filter_idc", val); */

  /* num_slice_groups_minus1 == 0 */

  if (sequence->entropy_coding_mode_flag)
  {
	  //byte alien
	  int bitrest = 0;
	  taa_h264_flush_code_buffer(writer);
	  bitrest = taa_h264_bitwriter_get_bitrest(writer);
	  
	  if (bitrest % 8 != 0)
	  {
		  int bits_buffered = 32 - bitrest;
		  int bits_to_padding = 8 - (bits_buffered % 8);
		  bits += taa_h264_write_bits(writer, bits_to_padding, (0xff >> (8 - bitrest)));
	  }
  }

  return bits;
}


/* Writes the macbroblock_layer() (section 7.3.5) until residual. Returns the
 * number of bits written. */
unsigned taa_h264_write_mb_header (
  mbinfo_t *    mb,
  bitwriter_t * writer,
  uint8_t *     intra_modes,
  int           mbrun,
  bool          islice,
  uint8_t *     last_coded_qp)
{
  static const int block_to_raster_idx[16] = {
    0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
  };

  unsigned numbits = 0;
  bool inter = !MB_TYPE_IS_INTRA (mb->mbtype);

  /* mb_skip_run is really a part of slice_data(), section 7.3.4 */
  if (islice == false)
    numbits += TAA_H264_WRITE_UE (writer, mbrun, "mb_skip_run");

  /* mb_type has different meaning for I and P slices, section 7.4.5 */
  int mbtype_tmp = mb->mbtype;
  if (mb->mbtype == I_16x16)
  {
    int offset_cbp_luma = taa_h264_get_cbp_luma (mb) ? 12 : 0;
    int offset_cbp_chroma = 4 * taa_h264_get_cbp_chroma (mb);
    int offset_pred_mode = mb->best_i16x16_mode;
    mbtype_tmp += offset_pred_mode + offset_cbp_luma + offset_cbp_chroma;
  }
  if (islice)
    mbtype_tmp -= MBTYPE_ISLICE_OFFSET;

  numbits += TAA_H264_WRITE_UE (writer, mbtype_tmp, "mb_type");

  if (MB_TYPE_IS_INTRA (mb->mbtype))
  {
    if (mb->mbtype == I_4x4)
    {
      int imode_offset = mb->mbpos * NUM_4x4_BLOCKS_Y;
      imode4_t * pred_intra_modes = &mb->pred_intra_modes [0];
      for (int k = 0; k < 16; k++)
      {
        int raster_idx     = block_to_raster_idx[k];
        imode4_t pred_mode = pred_intra_modes [raster_idx];
        uint8_t mode       = intra_modes [imode_offset + raster_idx];

        if (pred_mode == mode)
        {
          numbits += TAA_H264_WRITE_U (writer, 1, 1, "prev_intra_4x4_pred_mode");
        }
        else
        {
          int rem_intra4x4_pred_mode = mode;
          if (mode >= pred_mode)
            rem_intra4x4_pred_mode -= 1;
          numbits += TAA_H264_WRITE_U (writer, 0, 1, "prev_intra_4x4_pred_mode");
          numbits += TAA_H264_WRITE_U (writer, rem_intra4x4_pred_mode, 3, "rem_intra4x4_pred_mode");
        }
      }
    }
    numbits += TAA_H264_WRITE_UE (writer, mb->best_i8x8_mode_chroma, "intra_chroma_pred_mode");
  }
  else
  {
    /* Since we always have only one active ref frame we don't need to
     * send ref_idx_l0, only the vectors. */

    TAA_H264_DEBUG_ASSERT (mb->mbtype <= P_8x16 && mb->mbtype >= P_16x16);
    TAA_H264_DEBUG_ASSERT (mb->mv1.ref_num == 0);

    const int num_parts[] = { 1, 2, 2};

    numbits += TAA_H264_WRITE_SE (writer, mb->mv1.x - mb->pred1.x, "mvd_l0 (x)");
    numbits += TAA_H264_WRITE_SE (writer, mb->mv1.y - mb->pred1.y, "mvd_l0 (y)");

    if (num_parts[mb->mbtype] == 2)
    {
      TAA_H264_DEBUG_ASSERT (mb->mv2.ref_num == 0);
      numbits += TAA_H264_WRITE_SE (writer, mb->mv2.x - mb->pred2.x, "mvd_l0 (x)");
      numbits += TAA_H264_WRITE_SE (writer, mb->mv2.y - mb->pred2.y, "mvd_l0 (y)");
    }
  }
  int cbp_luma = taa_h264_get_cbp_luma (mb);
  int cbp_chroma = taa_h264_get_cbp_chroma (mb);
  if (mb->mbtype != I_16x16)
  {
    unsigned cbp_code = taa_h264_quote_cbp (cbp_luma, cbp_chroma, inter);
    numbits += TAA_H264_WRITE_UE (writer, cbp_code, "coded_block_pattern");
  }

  if (cbp_luma > 0 || cbp_chroma > 0 || mb->mbtype == I_16x16)
  {
    const int min_quant = 0;
    const int max_quant = 51;
    const int quant_levels = (max_quant - min_quant) + 1;
    const int min_dquant = -(quant_levels / 2);
    const int max_dquant = (quant_levels / 2) - 1;

    int delta_qp = mb->mquant - *last_coded_qp;
    *last_coded_qp = (uint8_t) mb->mquant;

    if (delta_qp < min_dquant)
      delta_qp += quant_levels;
    else if (delta_qp > max_dquant)
      delta_qp -= quant_levels;

    numbits += TAA_H264_WRITE_SE (writer, delta_qp, "mb_qp_delta");
  }

  return numbits;
}


#if REF_FRAME_CHECKSUM
void taa_h264_write_checksum (
  bitwriter_t * w,
  uint32_t      checksum,
  int           long_term_frame_idx,
  int           original_frame_num)
{
  taa_h264_write_nalu_header (w, NALU_PRIORITY_HIGH, NALU_TYPE_CHECKSUM, true);
  TAA_H264_WRITE_U (w, checksum >> 16, 16, "checksum_msb");
  TAA_H264_WRITE_U (w, checksum & 0xFFFF, 16, "checksum_lsb");
  TAA_H264_WRITE_U (w, long_term_frame_idx, 8, "long_term_frame_idx");
  TAA_H264_WRITE_U (w, original_frame_num, 16, "original_frame_num");
}
#endif
