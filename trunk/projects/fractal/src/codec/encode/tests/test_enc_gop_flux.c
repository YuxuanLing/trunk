#include "enc_gop.h"
#include "fractaltest.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void test_decide_refframes (void)
{
  ref_meta_t   *  refframes_meta [16];
  uint8_t total_ref_frames;
  const uint8_t max_ref_frames = 6;
  flux_t flux;
  bool is_super_p = false;
  ref_control_t * refcontrol;
  bool res;

  flux.ack_status = malloc (max_ref_frames * sizeof (int));
  memset (flux.ack_status, 0, max_ref_frames * sizeof (int));
  refcontrol = malloc (sizeof (ref_control_t));
  for (int i = 0; i < 16; i++)
  {
    refframes_meta[i] = malloc (sizeof (ref_meta_t));
    memset (refframes_meta[i], 0, sizeof (ref_meta_t));
  }


  // --------------------------------------------------------
  //          case 0: vacant slots in LT buffer
  // --------------------------------------------------------
  total_ref_frames                  =  3;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  1;

  refframes_meta[2]->age            =  2;
  refframes_meta[2]->long_term_idx  =  2;

  flux.ack_status[0] = 0;
  flux.ack_status[1] = 0;
  flux.ack_status[2] = 0;

  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 3);

  // ------------------------------------------------------------
  //          case 1: vacant slots in LT buffer, last slot
  // ------------------------------------------------------------

  total_ref_frames                  =  5;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  2;
  refframes_meta[2]->long_term_idx  =  1;

  refframes_meta[3]->age            =  3;
  refframes_meta[3]->long_term_idx  =  3;

  refframes_meta[4]->age            =  4;
  refframes_meta[4]->long_term_idx  =  2;

  flux.ack_status[0] =  0;
  flux.ack_status[1] =  0;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  1;
  flux.ack_status[4] = -1;

  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 4);


  // -------------------------------------------------------------------
  //   case 2: buffer full, one nacked slot, overwrite the nacked slot
  // -------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  5;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  10;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  15;
  refframes_meta[4]->long_term_idx  =  1;

  refframes_meta[5]->age            =  3;
  refframes_meta[5]->long_term_idx  =  3;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  0;
  flux.ack_status[2] =  0;
  flux.ack_status[3] = -1;
  flux.ack_status[4] =  0;

  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 3);

  // ---------------------------------------------------------------
  //    case 3: buffer full, two nacked slots, overwrite oldest
  // ---------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  5;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  10;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  15;
  refframes_meta[4]->long_term_idx  =  1;

  refframes_meta[5]->age            =  3;
  refframes_meta[5]->long_term_idx  =  3;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  0;
  flux.ack_status[2] = -1;
  flux.ack_status[3] = -1;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 2);

  // ---------------------------------------------------------------
  //    case 4: buffer full, outdated overwriteable slot (age > 100)
  // ---------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  45;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  74;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  155;
  refframes_meta[4]->long_term_idx  =  1;

  refframes_meta[5]->age            =  21;
  refframes_meta[5]->long_term_idx  =  3;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;

  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 1);

  // ---------------------------------------------------------------------------
  //    case 5: buffer full, multiple outdated overwriteable slot (age > 100)
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  145;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  74;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  155;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  21;
  refframes_meta[5]->long_term_idx  =  1;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 3);

  // ---------------------------------------------------------------------------
  //    case 6: overwriteable ACKs
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  45;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  21;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  11;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  72;
  refframes_meta[5]->long_term_idx  =  1;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  1;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  1;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 1);

  // ---------------------------------------------------------------------------
  //    case 6: overwriteable ACKs (all ACKed)
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  88;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  21;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  11;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  20;
  refframes_meta[5]->long_term_idx  =  1;


  flux.ack_status[0] =  1;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  1;
  flux.ack_status[3] =  1;
  flux.ack_status[4] =  1;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 2);

  // ---------------------------------------------------------------------------
  //    case 7: overwriteable unknowns (older than an ACK)
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  88;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  21;
  refframes_meta[3]->long_term_idx  =  4;

  refframes_meta[4]->age            =  11;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  20;
  refframes_meta[5]->long_term_idx  =  1;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 2);

  // ---------------------------------------------------------------------------
  //    case 8: overwriteable unknowns 2 (older than an ACK)
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  9;
  refframes_meta[1]->long_term_idx  =  4;

  refframes_meta[2]->age            =  4;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  5;
  refframes_meta[3]->long_term_idx  =  1;

  refframes_meta[4]->age            =  11;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  2;
  refframes_meta[5]->long_term_idx  =  0;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  1;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 3);

  // ---------------------------------------------------------------------------
  //    case 9: no candidates, pick oldest in buffer
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  9;
  refframes_meta[1]->long_term_idx  =  4;

  refframes_meta[2]->age            =  4;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  5;
  refframes_meta[3]->long_term_idx  =  1;

  refframes_meta[4]->age            =  11;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  2;
  refframes_meta[5]->long_term_idx  =  0;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  0;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 0);
  FASSERT (refcontrol->longterm_idx == 3);

  // ---------------------------------------------------------------------------
  //    case 10: no candidates, pick oldest in buffer 2
  // ---------------------------------------------------------------------------

  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  22;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  23;
  refframes_meta[1]->long_term_idx  =  4;

  refframes_meta[2]->age            =  33;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  9;
  refframes_meta[3]->long_term_idx  =  1;

  refframes_meta[4]->age            =  18;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  99;
  refframes_meta[5]->long_term_idx  =  0;


  flux.ack_status[0] =  0;
  flux.ack_status[1] =  0;
  flux.ack_status[2] =  0;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;


  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);

  FASSERT (res == true);
  FASSERT (refcontrol->ref_idx == 3);
  FASSERT (refcontrol->longterm_idx == 0);

  // cleanup
  for (int i = 0; i < 16; i++)
    free (refframes_meta[i]);
  free (flux.ack_status);
  free (refcontrol);
}


static void test_decide_refframes_weird_input (void)
{
  /* Tests to verify that we get a valid input even though the input not
   * entirely as expected. */
  ref_meta_t   *  refframes_meta [16];
  uint8_t total_ref_frames;
  const uint8_t max_ref_frames = 6;
  flux_t flux;
  bool is_super_p = false;
  ref_control_t * refcontrol;
  bool res;

  flux.ack_status = malloc (max_ref_frames * sizeof (int));
  memset (flux.ack_status, 0, max_ref_frames * sizeof (int));
  refcontrol = malloc (sizeof (ref_control_t));
  for (int i = 0; i < 16; i++)
  {
    refframes_meta[i] = malloc (sizeof (ref_meta_t));
    memset (refframes_meta[i], 0, sizeof (ref_meta_t));
  }

  // ---------------------------------------------------------------------------
  //    case 1: two acked frames with the same age, rest unknown
  //            bug, was returning ltfi -1
  // ---------------------------------------------------------------------------
  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  55;
  refframes_meta[1]->long_term_idx  =  0;

  refframes_meta[2]->age            =  51;
  refframes_meta[2]->long_term_idx  =  1;

  refframes_meta[3]->age            =  55;
  refframes_meta[3]->long_term_idx  =  2;

  refframes_meta[4]->age            =  47;
  refframes_meta[4]->long_term_idx  =  3;

  refframes_meta[5]->age            =  43;
  refframes_meta[5]->long_term_idx  =  4;

  flux.ack_status[0] =  1;
  flux.ack_status[1] =  0;
  flux.ack_status[2] =  1;
  flux.ack_status[3] =  0;
  flux.ack_status[4] =  0;

  res = taa_h264_flux_decide_refframes ((ref_meta_t **)&refframes_meta,
                                        total_ref_frames,
                                        max_ref_frames,
                                        refcontrol,
                                        &flux,
                                        is_super_p);


  // Must choose on of the acked frames with oldest age. Chose newest as reference.
  FASSERT (res == true);
  FASSERT (refcontrol->longterm_idx != -1);
  FASSERT (refcontrol->ref_idx == 0);


  for (int i = 0; i < 16; i++)
    free (refframes_meta[i]);
  free (flux.ack_status);
  free (refcontrol);
}



static void test_super_p_control (void)
{
  ref_meta_t   *  refframes_meta [16];
  uint8_t total_ref_frames;
  const uint8_t max_ref_frames = 6;
  flux_t flux;
  ref_control_t * refcontrol;
  bool found;

  flux.ack_status = malloc (max_ref_frames * sizeof (int));
  memset (flux.ack_status, 0, max_ref_frames * sizeof (int));
  refcontrol = malloc (sizeof (ref_control_t));
  for (int i = 0; i < 16; i++)
  {
    refframes_meta[i] = malloc (sizeof (ref_meta_t));
    memset (refframes_meta[i], 0, sizeof (ref_meta_t));
  }


  // -----------------------------------------------------------
  //   case 0: buffer not full + one acked frame, use this one
  // -----------------------------------------------------------
  total_ref_frames                  =  3;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  1;

  refframes_meta[2]->age            =  2;
  refframes_meta[2]->long_term_idx  =  2;

  flux.ack_status[0] = 0;
  flux.ack_status[1] = 1;
  flux.ack_status[2] = 0;

  found = taa_h264_flux_super_p_control (
    (ref_meta_t **)&refframes_meta,
    total_ref_frames,
    max_ref_frames,
    refcontrol,
    &flux);

  FASSERT (found == true);
  FASSERT (refcontrol->ref_idx == 1);
  FASSERT (refcontrol->longterm_idx == 3);

  // -----------------------------------------------------------
  //   case 1: two acked frames, use most recent
  // -----------------------------------------------------------
  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  3;

  refframes_meta[2]->age            =  2;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  3;
  refframes_meta[3]->long_term_idx  =  1;

  refframes_meta[4]->age            =  4;
  refframes_meta[4]->long_term_idx  =  4;

  refframes_meta[5]->age            =  5;
  refframes_meta[5]->long_term_idx  =  5;

  flux.ack_status[0] = 0;
  flux.ack_status[1] = 1;
  flux.ack_status[2] = 1;
  flux.ack_status[3] = 0;
  flux.ack_status[4] = 0;
  flux.ack_status[5] = 0;

  found = taa_h264_flux_super_p_control (
    (ref_meta_t **)&refframes_meta,
    total_ref_frames,
    max_ref_frames,
    refcontrol,
    &flux);

  FASSERT (found == true);
  FASSERT (refcontrol->ref_idx == 2);
  FASSERT (refcontrol->longterm_idx == 1);

  // -----------------------------------------------------------
  //   case 2: no acked frames, return false (override intra)
  // -----------------------------------------------------------
  total_ref_frames                  =  6;

  refframes_meta[0]->age            =  0;
  refframes_meta[0]->long_term_idx  = -1;

  refframes_meta[1]->age            =  1;
  refframes_meta[1]->long_term_idx  =  3;

  refframes_meta[2]->age            =  2;
  refframes_meta[2]->long_term_idx  =  2;

  refframes_meta[3]->age            =  3;
  refframes_meta[3]->long_term_idx  =  1;

  refframes_meta[4]->age            =  4;
  refframes_meta[4]->long_term_idx  =  4;

  refframes_meta[5]->age            =  5;
  refframes_meta[5]->long_term_idx  =  5;

  flux.ack_status[0] = 0;
  flux.ack_status[1] = 0;
  flux.ack_status[2] = 0;
  flux.ack_status[3] = 0;
  flux.ack_status[4] = 0;
  flux.ack_status[5] = 0;

  found = taa_h264_flux_super_p_control (
    (ref_meta_t **)&refframes_meta,
    total_ref_frames,
    max_ref_frames,
    refcontrol,
    &flux);

  FASSERT (found == false);
  if (found)
  {
    FASSERT (refcontrol->ref_idx == -1);
    FASSERT (refcontrol->longterm_idx == -1);
  }
  // cleanup
  for (int i = 0; i < 16; i++)
    free (refframes_meta[i]);
  free (flux.ack_status);
  free (refcontrol);

}

void test_enc_gop_flux (void)
{
  test_decide_refframes ();
  test_decide_refframes_weird_input ();
  test_super_p_control ();
}
