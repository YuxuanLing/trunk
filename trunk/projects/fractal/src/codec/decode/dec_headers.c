#include "dec_headers.h"
#include "dec_readbits.h"
#include "dec_getvlc.h"
#include "com_mbutils.h"
#include "com_intmath.h"
#include "com_prediction.h"
#include "com_error.h"

#ifdef NDEBUG
/* If we're in release mode, we will get some warings about variables
 * that are only used for debug mode. Disbale these warnings. */
# pragma warning (disable: 177)
# pragma warning (disable: 593)
#endif

#define MIN_QUANT 0
#define MAX_QUANT 51

void taa_h264_read_nalu_header (
  bitreader_t *     reader,
  nalref_t *        idc,
  nalutype_t *      type,
  error_handler_t * eh)
{
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* NAL UNIT HEADER *******\n\n");

  int zerobit = TAA_H264_READ_U (reader, 1, "forbidden zero bit");
  if (zerobit != 0)
  {
    TAA_H264_ERROR (eh, TAA_H264_INVALID_ZERO_BIT);
    return;
  }
  *idc = (nalref_t) TAA_H264_READ_U (reader, 2, "nal_ref_idc");
  *type = (nalutype_t) TAA_H264_READ_U (reader, 5, "nal_unit_type");

  /* if (*type == NALU_TYPE_PREFIX || *type == NALU_TYPE_SLICE_SCALABLE) */
  /* { */
  /*   TAA_H264_READ_U (reader, 1, "reserved_one_bit"); */
  /*   TAA_H264_READ_U (reader, 1, "idr_flag"); */
  /*   TAA_H264_READ_U (reader, 6, "priority_id"); */
  /*   TAA_H264_READ_U (reader, 1, "no_inter_layer_pred_flag"); */
  /*   TAA_H264_READ_U (reader, 3, "dependency_id"); */
  /*   TAA_H264_READ_U (reader, 4, "quality_id"); */
  /*   TAA_H264_READ_U (reader, 3, "temporal_id"); */
  /*   TAA_H264_READ_U (reader, 1, "use_ref_base_pic_flag"); */
  /*   TAA_H264_READ_U (reader, 1, "discardable_flag"); */
  /*   TAA_H264_READ_U (reader, 1, "output_flag"); */
  /*   TAA_H264_READ_U (reader, 2, "reserved_three_2bits"); */
  /* } */
}

static void taa_h264_read_hrd_parameters (
  bitreader_t * r)
{
  int cpb_cnt_minus1 = TAA_H264_READ_UE (r, "cpb_cnt_minus1");
  TAA_H264_READ_U (r, 4, "bit_rate_scale");
  TAA_H264_READ_U (r, 4, "cpb_size_scale");
  for (int i = 0; i <= cpb_cnt_minus1; i++)
  {
    TAA_H264_READ_UE (r, "bit_rate_value_minus1");
    TAA_H264_READ_UE (r, "cpb_size_value_minus1");
    TAA_H264_READ_U (r, 1, "cbr_flag");
  }
  TAA_H264_READ_U (r, 5, "initial_cpb_removal_delay_length_minus1");
  TAA_H264_READ_U (r, 5, "cpb_removal_delay_length_minus1");
  TAA_H264_READ_U (r, 5, "dpb_output_delay_length_minus1");
  TAA_H264_READ_U (r, 5, "time_offset_length");
}

static void taa_h264_default_vui_parameters (
  seq_parset_t * sps)
{
  // sample aspect ratio
  sps->sar_height = 0;
  sps->sar_width  = 0;

  // other defaults...?
}

static void taa_h264_read_vui_parameters (
  bitreader_t *  r,
  seq_parset_t * sps)
{
  if (TAA_H264_READ_U (r, 1, "aspect_ratio_info_present_flag"))
  {
    const uint16_t sar_tab[][2] = {
      {0, 0}, {1, 1}, {12, 11}, {10, 11}, {16, 11}, {40, 33}, {24, 11}, {20, 11},
      {32, 11}, {80, 33}, {18, 11}, {15, 11}, {64, 33}, {160, 99}, {4, 3}, {3, 2},
      {2, 1}
    };
    uint32_t sar_idc = TAA_H264_READ_U (r, 8, "aspect_ratio_idc");
    if (sar_idc < (sizeof sar_tab / sizeof sar_tab[0]))
    {
      sps->sar_width = sar_tab[sar_idc][0];
      sps->sar_height = sar_tab[sar_idc][1];
    }
    else if (sar_idc == 255)
    {
      sps->sar_width = (uint16_t) TAA_H264_READ_U (r, 16, "sar_width");
      sps->sar_height = (uint16_t) TAA_H264_READ_U (r, 16, "sar_height");
    }
  }
  else
  {
    sps->sar_width = 0;
    sps->sar_height = 0;
  }

  if (TAA_H264_READ_U (r, 1, "overscan_info_present_flag"))
    TAA_H264_READ_U (r, 1, "overscan_appropriate_flag");

  if (TAA_H264_READ_U (r, 1, "video_signal_type_present_flag"))
  {
    TAA_H264_READ_U (r, 3, "video_format");
    TAA_H264_READ_U (r, 1, "video_full_range_flag");
    if (TAA_H264_READ_U (r, 1, "colour_description_present_flag"))
    {
      TAA_H264_READ_U (r, 8, "colour_primaries");
      TAA_H264_READ_U (r, 8, "transfer_characteristics");
      TAA_H264_READ_U (r, 8, "matrix_coefficients");
    }
  }

  if (TAA_H264_READ_U (r, 1, "chroma_loc_info_present_flag"))
  {
    TAA_H264_READ_UE (r, "chroma_sample_loc_type_top_field");
    TAA_H264_READ_UE (r, "chroma_sample_loc_type_bottom_field");
  }

  if (TAA_H264_READ_U (r, 1, "timing_info_present_flag"))
  {
    TAA_H264_READ_U (r, 32, "num_units_in_tick");
    TAA_H264_READ_U (r, 32, "time_scale");
    TAA_H264_READ_U (r, 1, "fixed_frame_rate_flag");
  }

  int nal_hrd;
  int vcl_hrd;

  nal_hrd = TAA_H264_READ_U (r, 1, "nal_hrd_parameters_present_flag");
  if (nal_hrd)
    taa_h264_read_hrd_parameters (r);

  vcl_hrd = TAA_H264_READ_U (r, 1, "vcl_hrd_parameters_present_flag");
  if (vcl_hrd)
    taa_h264_read_hrd_parameters (r);

  if (vcl_hrd || nal_hrd)
    TAA_H264_READ_U (r, 1, "low_delay_hrd_flag");

  TAA_H264_READ_U (r, 1, "pic_struct_present_flag");
  if (TAA_H264_READ_U (r, 1, "bitstream_restriction_flag"))
  {
    TAA_H264_READ_U (r, 1, "motion_vectors_over_pic_boundaries_flag");
    TAA_H264_READ_UE (r, "max_bytes_per_pic_denom");
    TAA_H264_READ_UE (r, "max_bits_per_mb_denom");
    TAA_H264_READ_UE (r, "log2_max_mv_length_horizontal");
    TAA_H264_READ_UE (r, "log2_max_mv_length_vertical");
    TAA_H264_READ_UE (r, "num_reorder_frames");
    TAA_H264_READ_UE (r, "max_dec_frame_buffering");
  }
}


static void taa_h264_read_sps_data (
  bitreader_t *  r,
  seq_parset_t * sps)
{
  sps->log2_max_frame_num = TAA_H264_READ_UE (r, "log2_max_frame_num_minus4") + 4;

  sps->poc_type = (uint8_t) TAA_H264_READ_UE (r, "pic_order_cnt_type");
  if (sps->poc_type == 0)
  {
    int val = TAA_H264_READ_UE (r, "log2_max_pic_order_cnt_lsb_minus4");
    sps->log2_max_pic_order_cnt_lsb = val + 4;
  }
  else if (sps->poc_type == 1)
  {
    sps->poc_delta_always_zero = TAA_H264_READ_U (r, 1, "delta_pic_order_always_zero_flag");
    TAA_H264_READ_SE (r, "offset_for_non_ref_pic");
    TAA_H264_READ_SE (r, "offset_for_top_to_bottom_field");
    int poc_cycle = TAA_H264_READ_UE (r, "num_ref_frames_in_pic_order_cnt_cycle");
    for (int i = 0; i < poc_cycle; i++)
      TAA_H264_READ_SE (r, "offset_for_ref_frame");
  }

  sps->num_ref_frames = (uint8_t) TAA_H264_READ_UE (r, "num_ref_frames");
  sps->gaps_in_frame_num_allowed = TAA_H264_READ_U (r, 1, "gaps_in_frame_num_allowed");
  sps->width_mbs = TAA_H264_READ_UE (r, "pic_width_in_mbs_minus1") + 1;
  sps->height_mbs = TAA_H264_READ_UE (r, "pic_height_in_map_units_minus1") + 1;

#ifdef TAA_H264_ASSERTS_ENABLED
  int frame_mbs_only_flag = TAA_H264_READ_U (r, 1, "frame_mbs_only_flag");
  TAA_H264_DEBUG_ASSERT (frame_mbs_only_flag == 1);
#else
  TAA_H264_READ_U (r, 1, "frame_mbs_only_flag");
#endif

  /* direct_8x8_inference_flag should not be relevant for baseline since
   * there are no B-frames. */
  TAA_H264_READ_U (r, 1, "direct_8x8_inference_flag");

  if (TAA_H264_READ_U (r, 1, "frame_cropping_flag"))
  {
    sps->crop_left = TAA_H264_READ_UE (r, "frame_crop_left_offset");
    sps->crop_right = TAA_H264_READ_UE (r, "frame_crop_right_offset");
    sps->crop_top = TAA_H264_READ_UE (r, "frame_crop_top_offset");
    sps->crop_bottom = TAA_H264_READ_UE (r, "frame_crop_bottom_offset");
  }
  else
  {
    sps->crop_left = 0;
    sps->crop_right = 0;
    sps->crop_top = 0;
    sps->crop_bottom = 0;
  }

  if (TAA_H264_READ_U (r, 1, "vui_parameters_present_flag"))
    taa_h264_read_vui_parameters (r, sps);
  else
    taa_h264_default_vui_parameters (sps);
}

/* TODO: Notify warning/error if there are some parameters that we
 * don't support. */
/* Returns the seq_parameter_set_id */
int taa_h264_read_sequence_parameter_set (
  bitreader_t *  r,
  seq_parset_t * sps_array)
{
  int sps_id;
  seq_parset_t * sps;

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* SEQUENCE PARAMETER SET *******\n\n");

#ifdef TAA_H264_ASSERTS_ENABLED
  bool baseline_profile = (TAA_H264_READ_U (r, 8, "profile_idc") == BASELINE_PROFILE);
  bool baseline_constraint = TAA_H264_READ_U (r, 1, "constraint_set0_flag");
  TAA_H264_DEBUG_ASSERT (baseline_profile || baseline_constraint);
#else
  TAA_H264_READ_U (r, 8, "profile_idc");
  TAA_H264_READ_U (r, 1, "constraint_set0_flag");
#endif

  TAA_H264_READ_U (r, 1, "constraint_set1_flag");
  TAA_H264_READ_U (r, 1, "constraint_set2_flag");
  TAA_H264_READ_U (r, 1, "constraint_set3_flag");
#ifdef TAA_H264_ASSERTS_ENABLED
  int zerobits = TAA_H264_READ_U (r, 4, "reserved_zero_4bits");
  TAA_H264_DEBUG_ASSERT (zerobits == 0x0);
#else
  TAA_H264_READ_U (r, 4, "reserved_zero_4bits");
#endif

  TAA_H264_READ_U (r, 8, "level_idc");

  sps_id = TAA_H264_READ_UE (r, "seq_parameter_set_id");
  sps_id %= MAX_NUM_SEQ_PARSETS;
  sps = &sps_array [sps_id];
  sps->is_valid = true;

  taa_h264_read_sps_data (r, sps);

  taa_h264_advance_rbsp_trailing_bits (r);

  return sps_id;
}

/* Returns the pic_parameter_set_id */
int taa_h264_read_picture_parameter_set (
  bitreader_t *  r,
  pic_parset_t * pps_array)
{
  int val;
  int pps_id;
  pic_parset_t * pps;

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* PICTURE PARAMETER SET *******\n\n");

  pps_id = TAA_H264_READ_UE (r, "pic_parameter_set_id");
  pps_id %= MAX_NUM_PIC_PARSETS;
  pps = &pps_array [pps_id];
  pps->is_valid = true;

  pps->seq_parset_id = (uint8_t) TAA_H264_READ_UE (r, "seq_parameter_set_id");
  pps->seq_parset_id %= MAX_NUM_SEQ_PARSETS;

  val = TAA_H264_READ_U (r, 1, "entropy_coding_mode_flag");
  TAA_H264_DEBUG_ASSERT (val == 0);

  pps->pic_order_present = TAA_H264_READ_U (r, 1, "pic_order_present_flag");

  val = TAA_H264_READ_UE (r, "num_slice_groups_minus1");
  TAA_H264_DEBUG_ASSERT (val == 0);

  pps->num_ref_idx_l0_active = (uint8_t) (TAA_H264_READ_UE (r, "num_ref_idx_l0_active_minus1") + 1);

  TAA_H264_READ_UE (r, "num_ref_idx_l1_active_minus1");

  val = TAA_H264_READ_U (r, 1, "weighted_pred_flag");
  TAA_H264_DEBUG_ASSERT (val == 0);

  val = TAA_H264_READ_U (r, 2, "weighted_bipred_idc");
  TAA_H264_DEBUG_ASSERT (val == 0);

  pps->init_qp = (uint8_t) (TAA_H264_READ_SE (r, "pic_init_qp_minus26") + 26);

  /* pic_init_qp_minus26 is irrelevant for baseline */
  TAA_H264_READ_SE (r, "pic_init_qs_minus26");

  pps->chroma_qp_index_offset = (uint8_t) TAA_H264_READ_SE (r, "chroma_qp_index_offset");
  //TAA_H264_DEBUG_ASSERT (pps->chroma_qp_index_offset == 0);

  pps->deblocking_control_present = TAA_H264_READ_U (r, 1, "deblocking_filter_control_present_flag");

  pps->constrained_intra_pred_flag = TAA_H264_READ_U (r, 1, "constrained_intra_pred_flag");

  val = TAA_H264_READ_U (r, 1, "redundant_pic_cnt_present_flag");
  TAA_H264_DEBUG_ASSERT (val == 0);

  taa_h264_advance_rbsp_trailing_bits (r);

  return pps_id;
}

static bool taa_h264_read_rplr (
  rplr_t *      rplr,
  bitreader_t * r)
{
  int i = 0;
  rplr[i].idc = TAA_H264_READ_UE (r, "reordering_of_pic_nums_idc");

  while (rplr[i].idc < 3 && i < MAX_NUM_RPLR_CMDS-1)
  {
    /* Read value of current command */
    rplr[i].val = TAA_H264_READ_UE (r, rplr[i].idc == 2 ?
                                    "long_term_pic_num" : "abs_diff_pic_num_minus1");

    /* Get the idc of the next command */
    i++;
    rplr[i].idc = TAA_H264_READ_UE (r, "reordering_of_pic_nums_idc");
  }

  // If true we have a corrupt stream or more commands than we can handle
  if (rplr[i].idc != 3)
    return false;

  return true;
}


bool taa_h264_read_mmco (
  mmco_t *      mmco,
  bitreader_t * r)
{
  int i = 0;
  mmco[i].op = TAA_H264_READ_UE (r, "memory_management_control_operation");

  while (mmco[i].op > 0 && mmco[i].op <= 6 && i < MAX_NUM_MMCO_CMDS-1)
  {
    if (mmco[i].op != 5)
    {
#ifdef TAA_H264_TRACE_ENABLED
      const char * desc_tab[7] = {
        "",
        "difference_of_pic_nums_minus1",
        "long_term_pic_num",
        "difference_of_pic_nums_minus1",
        "max_long_term_frame_idx_plus1",
        "",
        "long_term_frame_idx"
      };
      mmco[i].val1 = TAA_H264_READ_UE (r, desc_tab[mmco[i].op]);
#else
      mmco[i].val1 = TAA_H264_READ_UE (r, "");
#endif
      if (mmco[i].op == 3)
        mmco[i].val2 = TAA_H264_READ_UE (r, "long_term_frame_idx");
    }


    /* Get the next operation */
    i++;
    TAA_H264_DEBUG_ASSERT (i < MAX_NUM_MMCO_CMDS);
    mmco[i].op = TAA_H264_READ_UE (r, "memory_management_control_operation");

  }

  // If true we have a corrupt stream or more commands than we can handle
  if (mmco[i].op != 0)
    return false;

  return true;
}

void taa_h264_read_slice_header (
  bitreader_t *        r,
  nalref_t             idc,
  nalutype_t           type,
  const seq_parset_t * sps_array,
  const pic_parset_t * pps_array,
  sliceinfo_t *        slice,
  error_handler_t *    error_handler)
{
  int val;
  const pic_parset_t * pps;
  const seq_parset_t * sps;

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* SLICE HEADER *******\n\n");

  slice->nalref_idc = idc;

  /* For now, each slice contains a whole frame  */
  slice->first_mb = TAA_H264_READ_UE (r, "first_mb_in_slice");

  val = TAA_H264_READ_UE (r, "slice_type");
  slice->islice = (val == SLICE_TYPE_I || val == (SLICE_TYPE_I + 5));

  slice->pic_parset_id = (uint8_t) TAA_H264_READ_UE (r, "pic_parameter_set_id");
  slice->pic_parset_id %= MAX_NUM_PIC_PARSETS;
  pps = &pps_array [slice->pic_parset_id];
  if (!pps->is_valid)
  {
    TAA_H264_ERROR (error_handler, TAA_H264_INVALID_PPS_REFERENCED);
    return;
  }
  sps = &sps_array [pps->seq_parset_id];
  if (!sps->is_valid)
  {
    TAA_H264_ERROR (error_handler, TAA_H264_INVALID_SPS_REFERENCED);
    return;
  }

  slice->frame_num = TAA_H264_READ_U (r, sps->log2_max_frame_num, "frame_num");

  if (type == NALU_TYPE_IDR)
    slice->idr_pic_id = (int16_t) TAA_H264_READ_UE (r, "idr_pic_id");
  else
    slice->idr_pic_id = IDR_PIC_ID_NOT_AVAILABLE;

  slice->poc.type = sps->poc_type;
  if (sps->poc_type == 0)
  {
    slice->poc.lsb = TAA_H264_READ_U (r, sps->log2_max_pic_order_cnt_lsb, "pic_order_cnt_lsb");
    if (pps->pic_order_present)
    {
      slice->poc.delta_bottom = TAA_H264_READ_SE (r, "delta_pic_order_cnt_bottom");
      TAA_H264_DEBUG_ASSERT (slice->poc.delta_bottom == 0);
    }
  }
  else if (sps->poc_type == 1 && !sps->poc_delta_always_zero)
  {
    slice->poc.delta_0 = TAA_H264_READ_SE (r, "delta_pic_order_cnt[0]");

    if (pps->pic_order_present)
    {
      slice->poc.delta_1 = TAA_H264_READ_SE (r, "delta_pic_order_cnt[1]");
      TAA_H264_DEBUG_ASSERT (slice->poc.delta_1 == 0);
    }
  }

  /* redundant_pic_cnt_present_flag != 1 */
  /* slicetype_t != B */

  if (slice->islice == false)
  {
    if (TAA_H264_READ_U (r, 1, "num_ref_idx_active_override_flag"))
    {
      slice->num_ref_idx_l0_active = (uint8_t) (
        TAA_H264_READ_UE (r, "num_ref_idx_l0_active_minus1") + 1);
    }
    else
    {
      slice->num_ref_idx_l0_active = pps->num_ref_idx_l0_active;
    }
    slice->num_ref_idx_l0_active =
      (uint8_t) min (slice->num_ref_idx_l0_active, MAX_NUM_REF_FRAMES);
  }

  /* ref_pic_list_reordring() */
  if (slice->islice == false)
  {
    slice->reordering_flag = TAA_H264_READ_U (r, 1, "ref_pic_list_reordering_flag_l0");
    if (slice->reordering_flag)
      if (!taa_h264_read_rplr (slice->rplr, r))
      {
        TAA_H264_ERROR (error_handler, TAA_H264_DPB_INVALID_RPLR);
        return;
      }
  }

  /* weighted_pred_flag != 1 */
  /* weighted_bipred_idc == 0 */

  if (idc != NALU_PRIORITY_DISPOSABLE)
  {
    if (type == NALU_TYPE_IDR)
    {
      /* TODO: Hanlde no_output_of_prior_pics_flag */
      TAA_H264_READ_U (r, 1, "no_output_of_prior_pics_flag");
      slice->long_term_ref_flag = TAA_H264_READ_U (r, 1, "long_term_ref_flag");
    }
    else
    {
      slice->long_term_ref_flag = false;
      slice->adaptive_marking_flag = TAA_H264_READ_U (r, 1, "adaptive_ref_pic_marking_mode_flag");
      if (slice->adaptive_marking_flag)
        if (!taa_h264_read_mmco (slice->mmco, r))
        {
          TAA_H264_ERROR (error_handler, TAA_H264_DPB_INVALID_MMCO);
          return;
        }
    }
  }

  /* entropy_coding_mode_flag != 1 */

  slice->qp = pps->init_qp + TAA_H264_READ_SE (r, "slice_qp_delta");
  if (slice->qp < MIN_QUANT || slice->qp > MAX_QUANT)
  {
    TAA_H264_ERROR (error_handler, TAA_H264_INVALID_QP);
    return;
  }

  slice->disable_deblock = 0;
  slice->filter_offset_a = 0;
  slice->filter_offset_b = 0;
  if (pps->deblocking_control_present)
  {
    slice->disable_deblock = (uint8_t) TAA_H264_READ_UE (r, "disable_deblocking_filter_idc");
    if (slice->disable_deblock != 1)
    {
      val = TAA_H264_READ_SE (r, "slice_alpha_c0_offset_div2");
      slice->filter_offset_a = (uint8_t)(val * 2);

      val = TAA_H264_READ_SE (r, "slice_beta_offset_div2");
      slice->filter_offset_b = (uint8_t)(val * 2);
    }
  }

  /* num_slice_groups_minus1 == 0 */
}


static bool taa_h264_read_intra_pred_modes (
  mbinfo_t *       mb,
  uint8_t *        intra_modes_ptr,
  int              intra_modes_stride,
  bool             constrained_intra_pred,
  mbinfo_store_t * mbinfo_stored,
  bitreader_t *    r)
{
  static const int block_to_raster [16] =
  { 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15 };

  if (mb->mbtype == I_4x4)
  {
    bool mb_left = MB_LEFT (mb->avail_flags);
    bool mb_up = MB_UP (mb->avail_flags);
    if (constrained_intra_pred)
    {
      mb_left = mb_left && (mbinfo_stored[-1].flags & MB_TYPE_INTRA_);
      mb_up = mb_up && (mbinfo_stored[-mb->mbxmax].flags & MB_TYPE_INTRA_);
    }

    for (int k = 0; k < 16; k++)
    {
      int raster_idx = block_to_raster[k];
      bool block_left = (raster_idx % 4 != 0);
      bool block_up = (raster_idx / 4 != 0);

      imode4_t mode;
      imode4_t pred_mode = taa_h264_predict_intra_4x4_mode (
        &intra_modes_ptr[raster_idx],
        intra_modes_stride,
        block_left,
        block_up,
        mb_left,
        mb_up);

      if (TAA_H264_READ_U (r, 1, "prev_intra_4x4_pred_mode"))
      {
        /* predicted mode is the same as actual mode */
        mode = pred_mode;
      }
      else
      {
        int rem = TAA_H264_READ_U (r, 3, "rem_intra4x4_pred_mode");
        if (rem < pred_mode)
          mode = (imode4_t) rem;
        else
          mode = (imode4_t)(rem + 1);
      }

      if (mode > MAX_PRED)
        return false;
      intra_modes_ptr[raster_idx] = (uint8_t) mode;
    }
  }
  else
  {
    for (int i = 0; i < 16; i++)
      intra_modes_ptr[i] = DC_PRED;
  }

  mb->intra_mode_chroma = (imode8_t) TAA_H264_READ_UE (r, "intra_chroma_pred_mode");
  if (mb->intra_mode_chroma > MAX_PRED_8)
    return false;

  return true;
}


static void taa_h264_read_motion_vectors (
  mbinfo_t *    mb,
  mv_t *        motion_vectors_ptr,
  int           motion_vectors_stride,
  int           num_active_ref_frames,
  bitreader_t * r)
{
  int sub_mb_type[4];
  int refnums[4];
  int partnum;
  bool mbtype_ref0 = false;

  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_up = MB_UP (mb->avail_flags);
  bool mb_upright = MB_UP_RIGHT (mb->avail_flags);
  bool mb_upleft = MB_UP_LEFT (mb->avail_flags);

  int left_flags;
  int up_flags;
  int upright_flags;
  int upleft_flags;

  taa_h264_set_4x4_blocks_neighbor_flags (
    mb_left, mb_up, mb_upright, mb_upleft,
    &left_flags, &up_flags, &upright_flags, &upleft_flags);

  if (mb->mbtype == P_8x8ref0)
  {
    mb->mbtype = P_8x8;
    mbtype_ref0 = true;
  }

  TAA_H264_DEBUG_ASSERT (mb->mbtype >= 0 && mb->mbtype <= 3);

  mb->all_P_8x8 = false;
  if (mb->mbtype == P_8x8)
  {
    /* Read how P_8x8 partitions are partitioned (sub-partitions). */
    mb->all_P_8x8 = true;
    for (int i = 0; i < 4; i++)
    {
      sub_mb_type[i] = TAA_H264_READ_UE (r, "sub_mb_type");
      if (sub_mb_type[i] > 0)
        mb->all_P_8x8 = false;
    }
  }

  int partsize_x = (((~mb->mbtype >> 1) & 0x1) + 1) * 8;
  int partsize_y = (((~mb->mbtype     ) & 0x1) + 1) * 8;

  if (num_active_ref_frames > 1 && mbtype_ref0 == false)
  {
    /* Reference frames is signaled in stream */
    partnum = 0;
    for (int i = 0; i < MB_HEIGHT_Y; i += partsize_y)
    {
      for (int j = 0; j < MB_WIDTH_Y; j += partsize_x)
      {
        if (num_active_ref_frames == 2)
          refnums[partnum] = !TAA_H264_READ_U (r, 1, "ref_idx_l0 != 0");
        else
          refnums[partnum] = TAA_H264_READ_UE (r, "ref_idx_l0");
        partnum++;
      }
    }
  }
  else
  {
    /* Reference frame must be 0 */
    for (partnum = 0; partnum < 4; partnum++)
      refnums[partnum] = 0;
  }

  /* Read MVDs and set the resulting MVs */
  partnum = 0;
  for (int i = 0; i < MB_HEIGHT_Y; i += partsize_y)
  {
    for (int j = 0; j < MB_WIDTH_Y; j += partsize_x)
    {
      mv_t mv;
      mv_t mvd;
      mv_t pred;
      int refnum = refnums[partnum++];
      int subpartsize_x = partsize_x;
      int subpartsize_y = partsize_y;

      if (mb->mbtype == P_8x8)
      {
        int k = i / 4 +  j / 8;
        int subtype = sub_mb_type[k];
        subpartsize_x = (((~subtype >> 1) & 0x1) + 1) * 4;
        subpartsize_y = (((~subtype     ) & 0x1) + 1) * 4;
      }

      for (int m = 0; m < partsize_y; m += subpartsize_y)
      {
        for (int n = 0; n < partsize_x; n += subpartsize_x)
        {
          int blocknum_top_left = i + m + (j + n) / 4;
          int blocknum_top_right = blocknum_top_left + subpartsize_x / 4 - 1;

          bool block_left = NEIGHBOR_BLOCK_AVAILABLE (left_flags, blocknum_top_left);
          bool block_up = NEIGHBOR_BLOCK_AVAILABLE (up_flags, blocknum_top_left);
          bool block_upright = NEIGHBOR_BLOCK_AVAILABLE (upright_flags, blocknum_top_right);
          bool block_upleft = NEIGHBOR_BLOCK_AVAILABLE (upleft_flags, blocknum_top_left);

          mvd.x = (uint16_t) TAA_H264_READ_SE (r, "mvd_l0 (x)");
          mvd.y = (uint16_t) TAA_H264_READ_SE (r, "mvd_l0 (y)");;

          pred = taa_h264_predict_motion_vector (
            motion_vectors_ptr,
            motion_vectors_stride,
            blocknum_top_left,
            subpartsize_y,
            subpartsize_x,
            refnum,
            block_left,
            block_up,
            block_upright,
            block_upleft);

          mv.x = (int16_t)(mvd.x + pred.x);
          mv.y = (int16_t)(mvd.y + pred.y);
          mv.ref_num = (int8_t) refnum;

          TAA_H264_TRACE (TAA_H264_TRACE_MV,
                          "pos = %4d, pred (%3d, %3d), mvd (%3d, %3d) mv (%3d, %3d)\n",
                          mb->mbpos, pred.x, pred.y, mvd.x, mvd.y,
                          mv.x, mv.y);

          for (int k = 0; k < subpartsize_y; k += 4)
          {
            for (int l = 0; l < subpartsize_x; l += 4)
            {
              int blocknum = blocknum_top_left + k + l / 4;
              motion_vectors_ptr[blocknum] = mv;
            }
          }
        }
      }
    }
  }
}

static void set_motion_vectors_not_avail (
  mv_t * motion_vectors_ptr)
{
  for (int i = 0; i < NUM_MVS_MB; i++)
  {
    motion_vectors_ptr[i].x = 0;
    motion_vectors_ptr[i].y = 0;
    motion_vectors_ptr[i].ref_num = -1;
  }
}

/* Read the macroblock header. Returns TAA_H264_NO_ERROR if everything is OK. */
int taa_h264_read_mb_header (
  frameinfo_t * frameinfo,
  mbinfo_t *    mb,
  bool          islice,
  bitreader_t * r,
  unsigned *    prev_qp)
{
  int mbpos = mb->mbpos;
  int mbxmax = mb->mbxmax;

  int mbtype_val = TAA_H264_READ_UE (r, "mb_type");
  if (islice)
    mbtype_val += MBTYPE_ISLICE_OFFSET;
  if (mbtype_val > I_4x4 && mbtype_val < I_PCM)
  {
    mb->mbtype = I_16x16;
    mbtype_val -= I_16x16;
    mb->cbp = taa_h264_inverse_cbp_code_i16 (mbtype_val);
    mb->intra_16x16_mode = (imode16_t)(mbtype_val % 4);
  }
  else
  {
    mb->mbtype = (mbtype_t) mbtype_val;
  }

  if (!TAA_H264_IS_IN_RANGE (mb->mbtype, MIN_MBTYPE, MAX_MBTYPE))
    return TAA_H264_INVALID_MBTYPE;

  if (mb->mbtype == I_PCM)
  {
    /* pcm_alignment_zero_bit: Don't bother reading each zero bit and
     * verify that it's zero. Just go to the end of zero bits. */
    taa_h264_advance_to_byte (r);
    return TAA_H264_NO_ERROR;
  }

  if (MB_TYPE_IS_INTRA (mb->mbtype))
  {
    if (!taa_h264_read_intra_pred_modes (
          mb,
          &frameinfo->intra_modes[mbpos * NUM_4x4_BLOCKS_Y],
          mbxmax * NUM_4x4_BLOCKS_Y,
          frameinfo->constrained_intra_pred,
          &frameinfo->mbinfo_stored[mb->mbpos],
          r))
    {
      return TAA_H264_INVALID_INTRA_MODE;
    }
    set_motion_vectors_not_avail (&frameinfo->motion_vectors[mbpos * NUM_MVS_MB]);
  }
  else
  {
    taa_h264_read_motion_vectors (
      mb,
      &frameinfo->motion_vectors[mbpos * NUM_MVS_MB],
      mbxmax * NUM_MVS_MB,
      frameinfo->num_active_ref_frames,
      r);
  }

  if (mb->mbtype != I_16x16)
  {
    int val = TAA_H264_READ_UE (r, "coded_block_pattern");
    if (val >= 48)
      return TAA_H264_INVALID_CBP;
    mb->cbp = taa_h264_inverse_cbp_code (val, mb->mbtype != I_4x4);
  }

  if (mb->cbp != 0 || mb->mbtype == I_16x16)
  {
    const int quant_levels = (MAX_QUANT - MIN_QUANT) + 1;
    int val = TAA_H264_READ_SE (r, "mb_qp_delta");
    TAA_H264_DEBUG_ASSERT (val >= -26 && val <= 25);

    /* mb->mquant =  (prev_quant + val + 52) % 52; Eq. 7-23 */
    int qp = *prev_qp + val;
    if (qp < MIN_QUANT)
      qp += quant_levels;
    else if (qp > MAX_QUANT)
      qp -= quant_levels;
    *prev_qp = mb->mquant = qp;

    if (qp < MIN_QUANT || qp > MAX_QUANT)
      return TAA_H264_INVALID_QP;

    TAA_H264_DEBUG_ASSERT (qp >= MIN_QUANT && qp <= MAX_QUANT);
  }
  else
  {
    mb->mquant = *prev_qp;
  }

  return TAA_H264_NO_ERROR;
}


bool taa_h264_read_prefix (
  bitreader_t * r,
  nalref_t      idc,
  int *         layer_index)
{
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* PREFIX NAL UNIT *******\n\n");

  {
    /* HACK: This is really a part of the NAL unit. */
    TAA_H264_READ_U (r, 1, "reserved_one_bit");
    TAA_H264_READ_U (r, 1, "idr_flag");
    TAA_H264_READ_U (r, 6, "priority_id");
    TAA_H264_READ_U (r, 1, "no_inter_layer_pred_flag");
    *layer_index = TAA_H264_READ_U (r, 3, "dependency_id");
    TAA_H264_READ_U (r, 4, "quality_id");
    TAA_H264_READ_U (r, 3, "temporal_id");
    TAA_H264_READ_U (r, 1, "use_ref_base_pic_flag");
    TAA_H264_READ_U (r, 1, "discardable_flag");
    TAA_H264_READ_U (r, 1, "output_flag");
    TAA_H264_READ_U (r, 2, "reserved_three_2bits");
  }

  if (idc != NALU_PRIORITY_DISPOSABLE)
  {
    TAA_H264_READ_U (r, 1, "store_ref_base_pic_flag");
    TAA_H264_READ_U (r, 1, "prefix_nal_unit_additional_extension_flag");
  }

  return true;
}


#if REF_FRAME_CHECKSUM

void taa_h264_read_checksum (
  bitreader_t * r,
  uint32_t *    checksum,
  int *         long_term_frame_idx,
  int *         original_frame_num)
{
  *checksum = TAA_H264_READ_U (r, 16, "checksum_mbs") << 16;
  *checksum |= TAA_H264_READ_U (r, 16, "checksum_lsb");
  *long_term_frame_idx = TAA_H264_READ_U (r, 8, "long_term_frame_idx");
  *original_frame_num = TAA_H264_READ_U (r, 16, "original_frame_num");
}

#endif
