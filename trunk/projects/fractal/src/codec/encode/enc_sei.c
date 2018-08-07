#include "enc_sei.h"
#include "enc_headers.h"
#include "enc_putvlc.h"
#include "com_sei.h"


static int taa_h264_write_sei_header (
  bitwriter_t * w,
  int           type,
  unsigned      size)
{
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS,
                  "\n\n******* SEI (type %d, size %u) *******\n\n", type, size);

  int numbits = 0;

  while (type >= 255)
  {
    numbits += TAA_H264_WRITE_U (w, 0xff, 8, "ff_byte");
    type -= 255;
  }
  numbits += TAA_H264_WRITE_U (w, type, 8, "last_payload_type_byte");

  while (size >= 255)
  {
    numbits += TAA_H264_WRITE_U (w, 0xff, 8, "ff_byte");
    size -= 255;
  }
  numbits += TAA_H264_WRITE_U (w, size, 8, "last_payload_size_byte");

  return numbits;
}


/**
 * Writes a SEI message of type "user data unregistered". DATA is the buffer
 * to write and DATA_SIZE is the length of it. Returns the number of bits
 * written.
 */
int taa_h264_write_user_data_sei (
  bitwriter_t *   w,
  const uint8_t * data,
  int        data_size)
{
  const int uuid_size = sizeof (uuid) / sizeof (uuid[0]);

  int numbits = taa_h264_write_nalu_header (
    w, NALU_PRIORITY_DISPOSABLE, NALU_TYPE_SEI, true);

  numbits += taa_h264_write_sei_header (
    w, SEI_TYPE_USER_DATA_UNREGISTERED, uuid_size + data_size);

  for (int i = 0; i < uuid_size; i++)
    TAA_H264_WRITE_U (w, uuid[i], 8, "uuid");
  numbits += uuid_size * 8;

  int i = 0;
  while (i < data_size)
  {
    // We need to be careful to not overflow the code buffer (may happen when
    // the user data is large )
    int codes_before_flush = taa_h264_available_codes_before_flush (w);
    for (int j = 0; j < codes_before_flush && i < data_size; j++, i++)
      TAA_H264_WRITE_U (w, data[i], 8, "user_data");

    taa_h264_flush_code_buffer (w);
  }
  numbits += data_size * 8;

  // Since the user data is byte aligned and its size is an integer number of
  // bytes we don't need to check for alignment at the end

  return numbits;
}

static int write_one_ref_pic_marking_sei (
  bitwriter_t * w,
  const rpm_t * rpm)
{
  int bits = TAA_H264_WRITE_U (
    w, SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION, 8, "last_payload_type_byte");

  /* We don't know the size at this point. Reserve space for it. */
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "last_payload_size_byte reserved\n");
  uint32_t * size_code_ptr = taa_h264_reserve_code (w);

  int payload_bits = TAA_H264_WRITE_U (w, rpm->idr_flag, 1, "original_idr_flag");
  payload_bits += TAA_H264_WRITE_UE (w, rpm->frame_num, "original_frame_num");
  payload_bits += taa_h264_write_ref_pic_marking (w, rpm);

  /* TODO: As reserved code can only hold 16 bits, so we need to make sure that
   * the payload size is not bigger than (0xff + 254) = 511 bytes. Actually,
   * the current implementation only supports 254 bytes */
  int payload_size = (payload_bits + 7) / 8;
  TAA_H264_DEBUG_ASSERT (payload_size < 255);
  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS,
                  "last_payload_size_byte %d\n", payload_size);
  taa_h264_write_reserved_code (size_code_ptr, 8, payload_size);
  bits += 8 + payload_bits;

  int mod8 = bits % 8;
  if (mod8)
  {
    int n = 8 - mod8;
    TAA_H264_WRITE_U (w, 0x1 << (n - 1), n, "sei payload alignment");
  }

  return bits;
}

/**
 * Writes a dec_ref_pic_marking_repetition SEI message.
 * Returns the number of bits written. */
int taa_h264_write_ref_pic_marking_sei (
  bitwriter_t * w,
  ref_meta_t ** repeat_list,
  int           repeat_count,
  int           temporal_id)
{
  /* H.264 recommendation, section 7.4.1: "nal_ref_idc shall be equal to 0 for
   * all NAL units having nal_unit_type equal to 6, 9, 10, 11, or 12."
   * We are violating this at the moment since we have hijacked the nal_ref_idc
   * to also indicate temporal layer, and we want the RPM repetition be sent on
   * the same layer as the current frame. Hopefully, this should not cause any
   * interop issues. */
  int bits = taa_h264_write_nalu_header (
    w, taa_h264_get_priority (NALU_TYPE_SEI, temporal_id), NALU_TYPE_SEI, true);

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS,
                  "\n\n******* SEI DEC REF PIC MARKING REPETITION *******\n\n");

  for (int i = 0; i < repeat_count; i++)
    bits += write_one_ref_pic_marking_sei(w, &repeat_list[i]->rpm);

  return bits;
}
