#include "dec_sei.h"
#include "dec_getvlc.h"
#include "dec_headers.h"
#include "com_sei.h"
#include "com_error.h"


static void taa_h264_read_sei_header (
  bitreader_t * r,
  int *         payload_type,
  int *         payload_size)
{
  int type = 0;
  int size = 0;
  int val;

  do
  {
    val = TAA_H264_READ_U (r, 8, "last_payload_type_byte");
    type += val;
  } while (val == 0xFF);

  do
  {
    val = TAA_H264_READ_U (r, 8, "last_payload_type_size");
    size += val;
  } while (val == 0xFF);

  *payload_type = type;
  *payload_size = size;
}

static void taa_h264_sei_skip_payload (
  bitreader_t * r,
  int           size)
{
  while (size > 4)
  {
    taa_h264_read_bits (r, 32);
    size -= 4;
  }
  taa_h264_read_bits (r, size * 8);
}

/**
 * Reads the user data of a SEI of type "user data unregistered" and puts the
 * read data into DATA_OUT. DATA_OUT must be of size larger or equal to
 * (payload_size - 16). DATA_LEN is the size of the buffer on input and returns
 * the value of the read bytes into the buffer.
 */
static bool taa_h264_read_sei_user_data (
  bitreader_t *     r,
  int               payload_size,
  callbacks_t *     cb,
  error_handler_t * eh)
{
  bool valid = true;
  const int uuid_size = sizeof (uuid) / sizeof (uuid[0]);

  if (taa_h264_reader_bytes_left (r) < payload_size)
  {
    TAA_H264_WARNING (eh, TAA_H264_UNEXPECTED_EOS);
    return false;
  }

  if (payload_size < uuid_size)
  {
    valid = false;
  }
  else
  {
    for (int i = 0; i < uuid_size; i++)
    {
      int val = TAA_H264_READ_U (r, 8, "uuid");
      if (val != uuid[i])
        valid = false;
    }
    payload_size -= uuid_size;
  }

  if (valid)
  {
    /* The user_data buffer is only available until the callback returns. Thus,
     * the application should copy the buffer if it wants to use it later. */
    uint8_t data[payload_size];

    for (int i = 0; i < payload_size; i++)
      data[i] = (uint8_t) TAA_H264_READ_U (r, 8, "user_data");

    if (cb->received_sei_user_data)
      cb->received_sei_user_data (cb->context, data, payload_size);
  }
  else
  {
    taa_h264_sei_skip_payload (r, payload_size);
  }

  return valid;
}


static void taa_h264_read_sei_ref_pic_marking (
  bitreader_t *              r,
  decoded_picture_buffer_t * dpb,
  callbacks_t *              cb)
{
  bool idr_flag = TAA_H264_READ_U (r, 1, "original_idr_flag");
  int frame_num = TAA_H264_READ_UE (r, "original_frame_num");
  bool idr_is_long_term = false;
  bool adaptive_marking = false;
  mmco_t mmco[MAX_NUM_MMCO_CMDS];

  if (idr_flag)
  {
    /* TODO: Handle no_output_of_prior_pics_flag */
    TAA_H264_READ_U (r, 1, "no_output_of_prior_pics_flag");
    idr_is_long_term = TAA_H264_READ_U (r, 1, "long_term_ref_flag");
  }
  else
  {
    adaptive_marking = TAA_H264_READ_U (r, 1, "adaptive_ref_pic_marking_mode_flag");
    if (adaptive_marking)
      taa_h264_read_mmco (mmco, r);
  }
  taa_h264_dec_dpb_rpm_repetition (
    dpb, frame_num, idr_flag, idr_is_long_term, adaptive_marking, mmco, cb);
}


bool taa_h264_read_sei (
  bitreader_t * r,
  decoder_t *   decoder)
{
  int payload_type;
  int payload_size;
  bool ret = true;

  callbacks_t * cb = &decoder->callbacks;
  error_handler_t * eh = &decoder->error_handler;

  TAA_H264_TRACE (TAA_H264_TRACE_HEADERS, "\n\n******* SEI *******\n\n");

  while (taa_h264_reader_has_more_data(r))
  {
    taa_h264_read_sei_header (r, &payload_type, &payload_size);

    switch (payload_type)
    {
    case SEI_TYPE_USER_DATA_UNREGISTERED:
    {
      ret = taa_h264_read_sei_user_data (r, payload_size, cb, eh);
      break;
    }
    case SEI_TYPE_DEC_REF_PIC_MARKING_REPETITION:
    {
      if (!decoder->dont_decode)
        taa_h264_read_sei_ref_pic_marking (r, &decoder->dpb, cb);
      else
        // Don't process the RPMs if the decoder is not ready for it
        taa_h264_sei_skip_payload (r, payload_size);
      break;
    }
    default:
    {
      // Not a warning since SEI is optional
      //TAA_H264_WARNING (eh, TAA_H264_UNSUPPORTED_SEI_TYPE);
      taa_h264_sei_skip_payload (r, payload_size);
      break;
    }
    };

    taa_h264_advance_to_byte (r);
  }
  return ret;
}
