#include "stdafx.h"
#include "h264_debug.h"

FILE* h264_dbgfile = NULL;
//存放信息的字符串
extern char tempstr[1000];
extern char outputstr[100000];
//#define printf(...) fprintf((h264_dbgfile == NULL ? stdout : h264_dbgfile), __VA_ARGS__)
//MFC下控制台输出
//#define printf(...) TRACE( __VA_ARGS__)
//组合成适合MFC的字符串
//注意MFC中EditControl的换行符是\r\n，需要单独添加
#define printf(...) sprintf_s( tempstr,__VA_ARGS__);\
	strcat_s(tempstr,"\r\n");						\
	strcat_s(outputstr,tempstr);



/***************************** debug ******************************/



void debug_sps(sps_t* sps)
{
	printf("======= SPS =======\n");
	printf(" profile_idc : %d \n", sps->profile_idc);
	printf(" constraint_set0_flag : %d \n", sps->constraint_set0_flag);
	printf(" constraint_set1_flag : %d \n", sps->constraint_set1_flag);
	printf(" constraint_set2_flag : %d \n", sps->constraint_set2_flag);
	printf(" constraint_set3_flag : %d \n", sps->constraint_set3_flag);
	printf(" constraint_set4_flag : %d \n", sps->constraint_set4_flag);
	printf(" constraint_set5_flag : %d \n", sps->constraint_set5_flag);
	printf(" reserved_zero_2bits : %d \n", sps->reserved_zero_2bits);
	printf(" level_idc : %d \n", sps->level_idc);
	printf(" seq_parameter_set_id : %d \n", sps->seq_parameter_set_id);
	printf(" chroma_format_idc : %d \n", sps->chroma_format_idc);
	printf(" residual_colour_transform_flag : %d \n", sps->residual_colour_transform_flag);
	printf(" bit_depth_luma_minus8 : %d \n", sps->bit_depth_luma_minus8);
	printf(" bit_depth_chroma_minus8 : %d \n", sps->bit_depth_chroma_minus8);
	printf(" qpprime_y_zero_transform_bypass_flag : %d \n", sps->qpprime_y_zero_transform_bypass_flag);
	printf(" seq_scaling_matrix_present_flag : %d \n", sps->seq_scaling_matrix_present_flag);
	//  int seq_scaling_list_present_flag[8];
	//  void* ScalingList4x4[6];
	//  int UseDefaultScalingMatrix4x4Flag[6];
	//  void* ScalingList8x8[2];
	//  int UseDefaultScalingMatrix8x8Flag[2];
	printf(" log2_max_frame_num_minus4 : %d \n", sps->log2_max_frame_num_minus4);
	printf(" pic_order_cnt_type : %d \n", sps->pic_order_cnt_type);
	printf("   log2_max_pic_order_cnt_lsb_minus4 : %d \n", sps->log2_max_pic_order_cnt_lsb_minus4);
	printf("   delta_pic_order_always_zero_flag : %d \n", sps->delta_pic_order_always_zero_flag);
	printf("   offset_for_non_ref_pic : %d \n", sps->offset_for_non_ref_pic);
	printf("   offset_for_top_to_bottom_field : %d \n", sps->offset_for_top_to_bottom_field);
	printf("   num_ref_frames_in_pic_order_cnt_cycle : %d \n", sps->num_ref_frames_in_pic_order_cnt_cycle);
	//  int offset_for_ref_frame[256];
	printf(" num_ref_frames : %d \n", sps->num_ref_frames);
	printf(" gaps_in_frame_num_value_allowed_flag : %d \n", sps->gaps_in_frame_num_value_allowed_flag);
	printf(" pic_width_in_mbs_minus1 : %d \n", sps->pic_width_in_mbs_minus1);
	printf(" pic_height_in_map_units_minus1 : %d \n", sps->pic_height_in_map_units_minus1);
	printf(" frame_mbs_only_flag : %d \n", sps->frame_mbs_only_flag);
	printf(" mb_adaptive_frame_field_flag : %d \n", sps->mb_adaptive_frame_field_flag);
	printf(" direct_8x8_inference_flag : %d \n", sps->direct_8x8_inference_flag);
	printf(" frame_cropping_flag : %d \n", sps->frame_cropping_flag);
	printf("   frame_crop_left_offset : %d \n", sps->frame_crop_left_offset);
	printf("   frame_crop_right_offset : %d \n", sps->frame_crop_right_offset);
	printf("   frame_crop_top_offset : %d \n", sps->frame_crop_top_offset);
	printf("   frame_crop_bottom_offset : %d \n", sps->frame_crop_bottom_offset);
	printf(" vui_parameters_present_flag : %d \n", sps->vui_parameters_present_flag);

	printf("=== VUI ===\n");
	printf(" aspect_ratio_info_present_flag : %d \n", sps->vui.aspect_ratio_info_present_flag);
	printf("   aspect_ratio_idc : %d \n", sps->vui.aspect_ratio_idc);
	printf("     sar_width : %d \n", sps->vui.sar_width);
	printf("     sar_height : %d \n", sps->vui.sar_height);
	printf(" overscan_info_present_flag : %d \n", sps->vui.overscan_info_present_flag);
	printf("   overscan_appropriate_flag : %d \n", sps->vui.overscan_appropriate_flag);
	printf(" video_signal_type_present_flag : %d \n", sps->vui.video_signal_type_present_flag);
	printf("   video_format : %d \n", sps->vui.video_format);
	printf("   video_full_range_flag : %d \n", sps->vui.video_full_range_flag);
	printf("   colour_description_present_flag : %d \n", sps->vui.colour_description_present_flag);
	printf("     colour_primaries : %d \n", sps->vui.colour_primaries);
	printf("   transfer_characteristics : %d \n", sps->vui.transfer_characteristics);
	printf("   matrix_coefficients : %d \n", sps->vui.matrix_coefficients);
	printf(" chroma_loc_info_present_flag : %d \n", sps->vui.chroma_loc_info_present_flag);
	printf("   chroma_sample_loc_type_top_field : %d \n", sps->vui.chroma_sample_loc_type_top_field);
	printf("   chroma_sample_loc_type_bottom_field : %d \n", sps->vui.chroma_sample_loc_type_bottom_field);
	printf(" timing_info_present_flag : %d \n", sps->vui.timing_info_present_flag);
	printf("   num_units_in_tick : %d \n", sps->vui.num_units_in_tick);
	printf("   time_scale : %d \n", sps->vui.time_scale);
	printf("   fixed_frame_rate_flag : %d \n", sps->vui.fixed_frame_rate_flag);
	printf(" nal_hrd_parameters_present_flag : %d \n", sps->vui.nal_hrd_parameters_present_flag);
	printf(" vcl_hrd_parameters_present_flag : %d \n", sps->vui.vcl_hrd_parameters_present_flag);
	printf("   low_delay_hrd_flag : %d \n", sps->vui.low_delay_hrd_flag);
	printf(" pic_struct_present_flag : %d \n", sps->vui.pic_struct_present_flag);
	printf(" bitstream_restriction_flag : %d \n", sps->vui.bitstream_restriction_flag);
	printf("   motion_vectors_over_pic_boundaries_flag : %d \n", sps->vui.motion_vectors_over_pic_boundaries_flag);
	printf("   max_bytes_per_pic_denom : %d \n", sps->vui.max_bytes_per_pic_denom);
	printf("   max_bits_per_mb_denom : %d \n", sps->vui.max_bits_per_mb_denom);
	printf("   log2_max_mv_length_horizontal : %d \n", sps->vui.log2_max_mv_length_horizontal);
	printf("   log2_max_mv_length_vertical : %d \n", sps->vui.log2_max_mv_length_vertical);
	printf("   num_reorder_frames : %d \n", sps->vui.num_reorder_frames);
	printf("   max_dec_frame_buffering : %d \n", sps->vui.max_dec_frame_buffering);

	printf("=== HRD ===\n");
	printf(" cpb_cnt_minus1 : %d \n", sps->hrd.cpb_cnt_minus1);
	printf(" bit_rate_scale : %d \n", sps->hrd.bit_rate_scale);
	printf(" cpb_size_scale : %d \n", sps->hrd.cpb_size_scale);
	int SchedSelIdx;
	for (SchedSelIdx = 0; SchedSelIdx <= sps->hrd.cpb_cnt_minus1; SchedSelIdx++)
	{
		printf("   bit_rate_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.bit_rate_value_minus1[SchedSelIdx]); // up to cpb_cnt_minus1, which is <= 31
		printf("   cpb_size_value_minus1[%d] : %d \n", SchedSelIdx, sps->hrd.cpb_size_value_minus1[SchedSelIdx]);
		printf("   cbr_flag[%d] : %d \n", SchedSelIdx, sps->hrd.cbr_flag[SchedSelIdx]);
	}
	printf(" initial_cpb_removal_delay_length_minus1 : %d \n", sps->hrd.initial_cpb_removal_delay_length_minus1);
	printf(" cpb_removal_delay_length_minus1 : %d \n", sps->hrd.cpb_removal_delay_length_minus1);
	printf(" dpb_output_delay_length_minus1 : %d \n", sps->hrd.dpb_output_delay_length_minus1);
	printf(" time_offset_length : %d \n", sps->hrd.time_offset_length);
}


void debug_pps(pps_t* pps)
{
	printf("======= PPS =======\n");
	printf(" pic_parameter_set_id : %d \n", pps->pic_parameter_set_id);
	printf(" seq_parameter_set_id : %d \n", pps->seq_parameter_set_id);
	printf(" entropy_coding_mode_flag : %d \n", pps->entropy_coding_mode_flag);
	printf(" pic_order_present_flag : %d \n", pps->pic_order_present_flag);
	printf(" num_slice_groups_minus1 : %d \n", pps->num_slice_groups_minus1);
	printf(" slice_group_map_type : %d \n", pps->slice_group_map_type);
	//  int run_length_minus1[8]; // up to num_slice_groups_minus1, which is <= 7 in Baseline and Extended, 0 otheriwse
	//  int top_left[8];
	//  int bottom_right[8];
	//  int slice_group_change_direction_flag;
	//  int slice_group_change_rate_minus1;
	//  int pic_size_in_map_units_minus1;
	//  int slice_group_id[256]; // FIXME what size?
	printf(" num_ref_idx_l0_active_minus1 : %d \n", pps->num_ref_idx_l0_active_minus1);
	printf(" num_ref_idx_l1_active_minus1 : %d \n", pps->num_ref_idx_l1_active_minus1);
	printf(" weighted_pred_flag : %d \n", pps->weighted_pred_flag);
	printf(" weighted_bipred_idc : %d \n", pps->weighted_bipred_idc);
	printf(" pic_init_qp_minus26 : %d \n", pps->pic_init_qp_minus26);
	printf(" pic_init_qs_minus26 : %d \n", pps->pic_init_qs_minus26);
	printf(" chroma_qp_index_offset : %d \n", pps->chroma_qp_index_offset);
	printf(" deblocking_filter_control_present_flag : %d \n", pps->deblocking_filter_control_present_flag);
	printf(" constrained_intra_pred_flag : %d \n", pps->constrained_intra_pred_flag);
	printf(" redundant_pic_cnt_present_flag : %d \n", pps->redundant_pic_cnt_present_flag);
	printf(" transform_8x8_mode_flag : %d \n", pps->transform_8x8_mode_flag);
	printf(" pic_scaling_matrix_present_flag : %d \n", pps->pic_scaling_matrix_present_flag);
	//  int pic_scaling_list_present_flag[8];
	//  void* ScalingList4x4[6];
	//  int UseDefaultScalingMatrix4x4Flag[6];
	//  void* ScalingList8x8[2];
	//  int UseDefaultScalingMatrix8x8Flag[2];
	printf(" second_chroma_qp_index_offset : %d \n", pps->second_chroma_qp_index_offset);
}

void debug_slice_header(slice_header_t* sh)
{
	printf("======= Slice Header =======\n");
	printf(" first_mb_in_slice : %d \n", sh->first_mb_in_slice);
	const char* slice_type_name;
	switch (sh->slice_type)
	{
	case SH_SLICE_TYPE_P:       slice_type_name = "P slice"; break;
	case SH_SLICE_TYPE_B:       slice_type_name = "B slice"; break;
	case SH_SLICE_TYPE_I:       slice_type_name = "I slice"; break;
	case SH_SLICE_TYPE_SP:      slice_type_name = "SP slice"; break;
	case SH_SLICE_TYPE_SI:      slice_type_name = "SI slice"; break;
	case SH_SLICE_TYPE_P_ONLY:  slice_type_name = "P slice only"; break;
	case SH_SLICE_TYPE_B_ONLY:  slice_type_name = "B slice only"; break;
	case SH_SLICE_TYPE_I_ONLY:  slice_type_name = "I slice only"; break;
	case SH_SLICE_TYPE_SP_ONLY: slice_type_name = "SP slice only"; break;
	case SH_SLICE_TYPE_SI_ONLY: slice_type_name = "SI slice only"; break;
	default:                    slice_type_name = "Unknown"; break;
	}
	printf(" slice_type : %d ( %s ) \n", sh->slice_type, slice_type_name);

	printf(" pic_parameter_set_id : %d \n", sh->pic_parameter_set_id);
	printf(" frame_num : %d \n", sh->frame_num);
	printf(" field_pic_flag : %d \n", sh->field_pic_flag);
	printf(" bottom_field_flag : %d \n", sh->bottom_field_flag);
	printf(" idr_pic_id : %d \n", sh->idr_pic_id);
	printf(" pic_order_cnt_lsb : %d \n", sh->pic_order_cnt_lsb);
	printf(" delta_pic_order_cnt_bottom : %d \n", sh->delta_pic_order_cnt_bottom);
	// int delta_pic_order_cnt[ 2 ];
	printf(" redundant_pic_cnt : %d \n", sh->redundant_pic_cnt);
	printf(" direct_spatial_mv_pred_flag : %d \n", sh->direct_spatial_mv_pred_flag);
	printf(" num_ref_idx_active_override_flag : %d \n", sh->num_ref_idx_active_override_flag);
	printf(" num_ref_idx_l0_active_minus1 : %d \n", sh->num_ref_idx_l0_active_minus1);
	printf(" num_ref_idx_l1_active_minus1 : %d \n", sh->num_ref_idx_l1_active_minus1);
	printf(" cabac_init_idc : %d \n", sh->cabac_init_idc);
	printf(" slice_qp_delta : %d \n", sh->slice_qp_delta);
	printf(" sp_for_switch_flag : %d \n", sh->sp_for_switch_flag);
	printf(" slice_qs_delta : %d \n", sh->slice_qs_delta);
	printf(" disable_deblocking_filter_idc : %d \n", sh->disable_deblocking_filter_idc);
	printf(" slice_alpha_c0_offset_div2 : %d \n", sh->slice_alpha_c0_offset_div2);
	printf(" slice_beta_offset_div2 : %d \n", sh->slice_beta_offset_div2);
	printf(" slice_group_change_cycle : %d \n", sh->slice_group_change_cycle);

	printf("=== Prediction Weight Table ===\n");
	printf(" luma_log2_weight_denom : %d \n", sh->pwt.luma_log2_weight_denom);
	printf(" chroma_log2_weight_denom : %d \n", sh->pwt.chroma_log2_weight_denom);
	//   printf(" luma_weight_l0_flag : %d \n", sh->pwt.luma_weight_l0_flag );
	// int luma_weight_l0[64];
	// int luma_offset_l0[64];
	//    printf(" chroma_weight_l0_flag : %d \n", sh->pwt.chroma_weight_l0_flag );
	// int chroma_weight_l0[64][2];
	// int chroma_offset_l0[64][2];
	//   printf(" luma_weight_l1_flag : %d \n", sh->pwt.luma_weight_l1_flag );
	// int luma_weight_l1[64];
	// int luma_offset_l1[64];
	//    printf(" chroma_weight_l1_flag : %d \n", sh->pwt.chroma_weight_l1_flag );
	// int chroma_weight_l1[64][2];
	// int chroma_offset_l1[64][2];

	printf("=== Ref Pic List Reordering ===\n");
	printf(" ref_pic_list_reordering_flag_l0 : %d \n", sh->rplr.ref_pic_list_reordering_flag_l0);
	printf(" ref_pic_list_reordering_flag_l1 : %d \n", sh->rplr.ref_pic_list_reordering_flag_l1);
	// int reordering_of_pic_nums_idc;
	// int abs_diff_pic_num_minus1;
	// int long_term_pic_num;

	printf("=== Decoded Ref Pic Marking ===\n");
	printf(" no_output_of_prior_pics_flag : %d \n", sh->drpm.no_output_of_prior_pics_flag);
	printf(" long_term_reference_flag : %d \n", sh->drpm.long_term_reference_flag);
	printf(" adaptive_ref_pic_marking_mode_flag : %d \n", sh->drpm.adaptive_ref_pic_marking_mode_flag);
	// int memory_management_control_operation;
	// int difference_of_pic_nums_minus1;
	// int long_term_pic_num;
	// int long_term_frame_idx;
	// int max_long_term_frame_idx_plus1;

}

void debug_aud(aud_t* aud)
{
	printf("======= Access Unit Delimiter =======\n");
	const char* primary_pic_type_name;
	switch (aud->primary_pic_type)
	{
	case AUD_PRIMARY_PIC_TYPE_I:       primary_pic_type_name = "I"; break;
	case AUD_PRIMARY_PIC_TYPE_IP:      primary_pic_type_name = "I, P"; break;
	case AUD_PRIMARY_PIC_TYPE_IPB:     primary_pic_type_name = "I, P, B"; break;
	case AUD_PRIMARY_PIC_TYPE_SI:      primary_pic_type_name = "SI"; break;
	case AUD_PRIMARY_PIC_TYPE_SISP:    primary_pic_type_name = "SI, SP"; break;
	case AUD_PRIMARY_PIC_TYPE_ISI:     primary_pic_type_name = "I, SI"; break;
	case AUD_PRIMARY_PIC_TYPE_ISIPSP:  primary_pic_type_name = "I, SI, P, SP"; break;
	case AUD_PRIMARY_PIC_TYPE_ISIPSPB: primary_pic_type_name = "I, SI, P, SP, B"; break;
	default: primary_pic_type_name = "Unknown"; break;
	}
	printf(" primary_pic_type : %d ( %s ) \n", aud->primary_pic_type, primary_pic_type_name);
}

void debug_seis(h264_stream_t* h)
{
	sei_t** seis = h->seis;
	int num_seis = h->num_seis;

	printf("======= SEI =======\n");
	const char* sei_type_name;
	int i;
	for (i = 0; i < num_seis; i++)
	{
		sei_t* s = seis[i];
		switch (s->payloadType)
		{
		case SEI_TYPE_BUFFERING_PERIOD:          sei_type_name = "Buffering period"; break;
		case SEI_TYPE_PIC_TIMING:                sei_type_name = "Pic timing"; break;
		case SEI_TYPE_PAN_SCAN_RECT:             sei_type_name = "Pan scan rect"; break;
		case SEI_TYPE_FILLER_PAYLOAD:            sei_type_name = "Filler payload"; break;
		case SEI_TYPE_USER_DATA_REGISTERED_ITU_T_T35: sei_type_name = "User data registered ITU-T T35"; break;
		case SEI_TYPE_USER_DATA_UNREGISTERED:    sei_type_name = "User data unregistered"; break;
		case SEI_TYPE_RECOVERY_POINT:            sei_type_name = "Recovery point"; break;
		case SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION: sei_type_name = "Dec ref pic marking repetition"; break;
		case SEI_TYPE_SPARE_PIC:                 sei_type_name = "Spare pic"; break;
		case SEI_TYPE_SCENE_INFO:                sei_type_name = "Scene info"; break;
		case SEI_TYPE_SUB_SEQ_INFO:              sei_type_name = "Sub seq info"; break;
		case SEI_TYPE_SUB_SEQ_LAYER_CHARACTERISTICS: sei_type_name = "Sub seq layer characteristics"; break;
		case SEI_TYPE_SUB_SEQ_CHARACTERISTICS:   sei_type_name = "Sub seq characteristics"; break;
		case SEI_TYPE_FULL_FRAME_FREEZE:         sei_type_name = "Full frame freeze"; break;
		case SEI_TYPE_FULL_FRAME_FREEZE_RELEASE: sei_type_name = "Full frame freeze release"; break;
		case SEI_TYPE_FULL_FRAME_SNAPSHOT:       sei_type_name = "Full frame snapshot"; break;
		case SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_START: sei_type_name = "Progressive refinement segment start"; break;
		case SEI_TYPE_PROGRESSIVE_REFINEMENT_SEGMENT_END: sei_type_name = "Progressive refinement segment end"; break;
		case SEI_TYPE_MOTION_CONSTRAINED_SLICE_GROUP_SET: sei_type_name = "Motion constrained slice group set"; break;
		case SEI_TYPE_FILM_GRAIN_CHARACTERISTICS: sei_type_name = "Film grain characteristics"; break;
		case SEI_TYPE_DEBLOCKING_FILTER_DISPLAY_PREFERENCE: sei_type_name = "Deblocking filter display preference"; break;
		case SEI_TYPE_STEREO_VIDEO_INFO:         sei_type_name = "Stereo video info"; break;
		default: sei_type_name = "Unknown"; break;
		}
		printf("=== %s ===\n", sei_type_name);
		printf(" payloadType : %d \n", s->payloadType);
		printf(" payloadSize : %d \n", s->payloadSize);

		printf(" payload : ");
		debug_bytes(s->payload, s->payloadSize);
	}
}

/**
Print the contents of a NAL unit to standard output.
The NAL which is printed out has a type determined by nal and data which comes from other fields within h depending on its type.
@param[in]      h          the stream object
@param[in]      nal        the nal unit
*/
void debug_nal(h264_stream_t* h, nal_t* nal)
{
	printf("==================== NAL ====================\n");
	printf(" forbidden_zero_bit : %d \n", nal->forbidden_zero_bit);
	printf(" nal_ref_idc : %d \n", nal->nal_ref_idc);
	// TODO make into subroutine
	const char* nal_unit_type_name;
	switch (nal->nal_unit_type)
	{
	case  NAL_UNIT_TYPE_UNSPECIFIED:                   nal_unit_type_name = "Unspecified"; break;
	case  NAL_UNIT_TYPE_CODED_SLICE_NON_IDR:           nal_unit_type_name = "Coded slice of a non-IDR picture"; break;
	case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A:  nal_unit_type_name = "Coded slice data partition A"; break;
	case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B:  nal_unit_type_name = "Coded slice data partition B"; break;
	case  NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C:  nal_unit_type_name = "Coded slice data partition C"; break;
	case  NAL_UNIT_TYPE_CODED_SLICE_IDR:               nal_unit_type_name = "Coded slice of an IDR picture"; break;
	case  NAL_UNIT_TYPE_SEI:                           nal_unit_type_name = "Supplemental enhancement information (SEI)"; break;
	case  NAL_UNIT_TYPE_SPS:                           nal_unit_type_name = "Sequence parameter set"; break;
	case  NAL_UNIT_TYPE_PPS:                           nal_unit_type_name = "Picture parameter set"; break;
	case  NAL_UNIT_TYPE_AUD:                           nal_unit_type_name = "Access unit delimiter"; break;
	case  NAL_UNIT_TYPE_END_OF_SEQUENCE:               nal_unit_type_name = "End of sequence"; break;
	case  NAL_UNIT_TYPE_END_OF_STREAM:                 nal_unit_type_name = "End of stream"; break;
	case  NAL_UNIT_TYPE_FILLER:                        nal_unit_type_name = "Filler data"; break;
	case  NAL_UNIT_TYPE_SPS_EXT:                       nal_unit_type_name = "Sequence parameter set extension"; break;
		// 14..18    // Reserved
	case  NAL_UNIT_TYPE_CODED_SLICE_AUX:               nal_unit_type_name = "Coded slice of an auxiliary coded picture without partitioning"; break;
		// 20..23    // Reserved
		// 24..31    // Unspecified
	default:                                           nal_unit_type_name = "Unknown"; break;
	}
	printf(" nal_unit_type : %d ( %s ) \n", nal->nal_unit_type, nal_unit_type_name);

	if (nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) { debug_slice_header(h->sh); }
	else if (nal->nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) { debug_slice_header(h->sh); }
	else if (nal->nal_unit_type == NAL_UNIT_TYPE_SPS) { debug_sps(h->sps); }
	else if (nal->nal_unit_type == NAL_UNIT_TYPE_PPS) { debug_pps(h->pps); }
	else if (nal->nal_unit_type == NAL_UNIT_TYPE_AUD) { debug_aud(h->aud); }
	else if (nal->nal_unit_type == NAL_UNIT_TYPE_SEI) { debug_seis(h); }
}

void debug_bytes(uint8_t* buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf("%02X ", buf[i]);
		if ((i + 1) % 16 == 0) { printf("\n"); }
	}
	printf("\n");
}


void debug_avcc(avcc_t* avcc)
{
  printf("======= AVC Decoder Configuration Record =======\n");
  printf(" configurationVersion: %d\n", avcc->configurationVersion );
  printf(" AVCProfileIndication: %d\n", avcc->AVCProfileIndication );
  printf(" profile_compatibility: %d\n", avcc->profile_compatibility );
  printf(" AVCLevelIndication: %d\n", avcc->AVCLevelIndication );
  printf(" lengthSizeMinusOne: %d\n", avcc->lengthSizeMinusOne );

  printf("\n");
  printf(" numOfSequenceParameterSets: %d\n", avcc->numOfSequenceParameterSets );
  for (int i = 0; i < avcc->numOfSequenceParameterSets; i++)
  {
    //printf(" sequenceParameterSetLength\n", avcc->sequenceParameterSetLength );
    if (avcc->sps_table[i] == NULL) { printf(" null sps\n"); continue; }
    debug_sps(avcc->sps_table[i]);
  }

  printf("\n");
  printf(" numOfPictureParameterSets: %d\n", avcc->numOfPictureParameterSets );
  for (int i = 0; i < avcc->numOfPictureParameterSets; i++)
  {
    //printf(" pictureParameterSetLength\n", avcc->pictureParameterSetLength );
    if (avcc->pps_table[i] == NULL) { printf(" null pps\n"); continue; }
    debug_pps(avcc->pps_table[i]);
  }
}
