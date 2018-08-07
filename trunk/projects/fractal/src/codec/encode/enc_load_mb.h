#include "enc_typedefs.h"
#include <emmintrin.h>

int32_t generate_coeffs(
  int8_t ret[64][8],
  int nt,
  int np,
  int rsz,
  int csum
  );

/* ============== global constants  ================*/
#define RSA_SHFT  20
#define SHFT      (RSA_SHFT - 1)
#define RSA       (1 << (RSA_SHFT - 1))
#define LOG_NTxNP  9
#define NTxNP     (1 << LOG_NTxNP)
#define MAX_NPxNT (1 << LOG_NTxNP)

#define LOG_NUMTAPS   3
#define NUMTAPS       (1 << LOG_NUMTAPS)
#define LOG_NUMPHASES (LOG_NTxNP - LOG_NUMTAPS)
#define NUMPHASES     (1 << LOG_NUMPHASES)
#define PHMASK        (NUMPHASES - 1)

#define NUM_FIR_TYPES           7


resizer_t * taa_h264_resizer_create (void);
void taa_h264_resizer_delete (resizer_t * r);
void taa_h264_resizer_init (resizer_t * r,
                            int         fact);

void taa_h264_load_resized_mb (
  resizer_t *        resizer,
  cpuinfo_t *        cpuinfo,
  const uint8_t *    in_y,
  const uint8_t *    in_u,
  const uint8_t *    in_v,
  uint8_t *          mb_y,
  uint8_t *          mb_uv,
  int                width_in,
  int                height_in,
  int                xpos_in,
  int                ypos_in,
  unsigned int       fineH,
  unsigned int       fineV);
