#include "com_interpolation.h"

void taa_h264_save_best_chroma_8x8(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *  best_uv
  )
{
  if ((xfrac == 0) && (yfrac == 0)) // no interpolate
  {
    _mm_store_pd((double *)(best_uv + 16*0), _mm_set_pd (*((double *)(full_v + cstride*0)), *((double *)(full_u + cstride*0))));
    _mm_store_pd((double *)(best_uv + 16*1), _mm_set_pd (*((double *)(full_v + cstride*1)), *((double *)(full_u + cstride*1))));
    _mm_store_pd((double *)(best_uv + 16*2), _mm_set_pd (*((double *)(full_v + cstride*2)), *((double *)(full_u + cstride*2))));
    _mm_store_pd((double *)(best_uv + 16*3), _mm_set_pd (*((double *)(full_v + cstride*3)), *((double *)(full_u + cstride*3))));
    _mm_store_pd((double *)(best_uv + 16*4), _mm_set_pd (*((double *)(full_v + cstride*4)), *((double *)(full_u + cstride*4))));
    _mm_store_pd((double *)(best_uv + 16*5), _mm_set_pd (*((double *)(full_v + cstride*5)), *((double *)(full_u + cstride*5))));
    _mm_store_pd((double *)(best_uv + 16*6), _mm_set_pd (*((double *)(full_v + cstride*6)), *((double *)(full_u + cstride*6))));
    _mm_store_pd((double *)(best_uv + 16*7), _mm_set_pd (*((double *)(full_v + cstride*7)), *((double *)(full_u + cstride*7))));
  }
  else if (yfrac == 0)  // interpolate between 2 horizontal
  {
    if (xfrac == 4)     // simple avg
    {
      for (int y = 0; y < 8; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_avg_epu8(R0, R1));
      }
    }
    else
    {
      __m128i const O  = _mm_setzero_si128();
      __m128i const C0 = _mm_set1_epi16((int16_t)((8 - xfrac) * 8));
      __m128i const C1 = _mm_set1_epi16((int16_t)((xfrac    ) * 8));
      for (int y = 0; y < 8; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
        __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
        __m128i xmm1 = _mm_unpacklo_epi8(R1, O);
        __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
        __m128i xmm5 = _mm_unpackhi_epi8(R1, O);
        xmm0 = _mm_mullo_epi16(xmm0, C0);
        xmm1 = _mm_mullo_epi16(xmm1, C1);
        xmm4 = _mm_mullo_epi16(xmm4, C0);
        xmm5 = _mm_mullo_epi16(xmm5, C1);
        xmm1 = _mm_add_epi16(xmm1, xmm0);
        xmm1 = _mm_add_epi16(xmm1, _mm_set1_epi16(32));
        xmm1 = _mm_srai_epi16(xmm1, 6);
        xmm5 = _mm_add_epi16(xmm5, xmm4);
        xmm5 = _mm_add_epi16(xmm5, _mm_set1_epi16(32));
        xmm5 = _mm_srai_epi16(xmm5, 6);
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm1, xmm5));
      }
    }
  }
  else if (xfrac == 0)  // interpolate between 2 vertical
  {
    if (yfrac == 4)     // simple avg
    {
      for (int y = 0; y < 8; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_avg_epu8(R0, R2));
      }
    }
    else
    {
      __m128i const O  = _mm_setzero_si128();
      __m128i const C0 = _mm_set1_epi16((int16_t)(8 * (8 - yfrac)));
      __m128i const C2 = _mm_set1_epi16((int16_t)(8 * (yfrac    )));

      for (int y = 0; y < 8; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));

        __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
        __m128i xmm2 = _mm_unpacklo_epi8(R2, O);
        __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
        __m128i xmm6 = _mm_unpackhi_epi8(R2, O);

        xmm0 = _mm_mullo_epi16(xmm0, C0);
        xmm2 = _mm_mullo_epi16(xmm2, C2);
        xmm4 = _mm_mullo_epi16(xmm4, C0);
        xmm6 = _mm_mullo_epi16(xmm6, C2);

        xmm2 = _mm_add_epi16(xmm2, xmm0);
        xmm2 = _mm_add_epi16(xmm2, _mm_set1_epi16(32));
        xmm2 = _mm_srai_epi16(xmm2, 6);

        xmm6 = _mm_add_epi16(xmm6, xmm4);
        xmm6 = _mm_add_epi16(xmm6, _mm_set1_epi16(32));
        xmm6 = _mm_srai_epi16(xmm6, 6);

        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm2, xmm6));
      }
    }
  }
  else                  // interpolate between all 4 points
  {
    __m128i const O  = _mm_setzero_si128();
    __m128i const C0 = _mm_set1_epi16((int16_t)((8 - xfrac) * (8 - yfrac)));
    __m128i const C1 = _mm_set1_epi16((int16_t)((xfrac    ) * (8 - yfrac)));
    __m128i const C2 = _mm_set1_epi16((int16_t)((8 - xfrac) * (yfrac    )));
    __m128i const C3 = _mm_set1_epi16((int16_t)((xfrac    ) * (yfrac    )));

    for (int y = 0; y < 8; y++)
    {
      __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
      __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
      __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));
      __m128i R3 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+1])), *((double *)(&full_u[cstride*(y+1)+1]))));

      __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
      __m128i xmm1 = _mm_unpacklo_epi8(R1, O);
      __m128i xmm2 = _mm_unpacklo_epi8(R2, O);
      __m128i xmm3 = _mm_unpacklo_epi8(R3, O);
      __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
      __m128i xmm5 = _mm_unpackhi_epi8(R1, O);
      __m128i xmm6 = _mm_unpackhi_epi8(R2, O);
      __m128i xmm7 = _mm_unpackhi_epi8(R3, O);

      xmm0 = _mm_mullo_epi16(xmm0, C0);
      xmm1 = _mm_mullo_epi16(xmm1, C1);
      xmm2 = _mm_mullo_epi16(xmm2, C2);
      xmm3 = _mm_mullo_epi16(xmm3, C3);
      xmm4 = _mm_mullo_epi16(xmm4, C0);
      xmm5 = _mm_mullo_epi16(xmm5, C1);
      xmm6 = _mm_mullo_epi16(xmm6, C2);
      xmm7 = _mm_mullo_epi16(xmm7, C3);

      xmm1 = _mm_add_epi16(xmm1, xmm0);
      xmm2 = _mm_add_epi16(xmm2, xmm1);
      xmm3 = _mm_add_epi16(xmm3, xmm2);
      xmm3 = _mm_add_epi16(xmm3, _mm_set1_epi16(32));
      xmm3 = _mm_srai_epi16(xmm3, 6);

      xmm5 = _mm_add_epi16(xmm5, xmm4);
      xmm6 = _mm_add_epi16(xmm6, xmm5);
      xmm7 = _mm_add_epi16(xmm7, xmm6);
      xmm7 = _mm_add_epi16(xmm7, _mm_set1_epi16(32));
      xmm7 = _mm_srai_epi16(xmm7, 6);

      _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm3, xmm7));
    }
  }
}


void taa_h264_save_best_chroma_8x4(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *  best_uv
  )
{
  if ((xfrac == 0) && (yfrac == 0))
  {
    for (int y = 0; y < 4; y++)
    {
      _mm_store_pd((double *)(best_uv + 16*y), _mm_set_pd (*((double *)(full_v + cstride*y)), *((double *)(full_u + cstride*y))));
    }
  }
  else if (yfrac == 0)  // interpolate between 2 horizontal
  {
    if (xfrac == 4)     // simple avg
    {
      for (int y = 0; y < 4; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_avg_epu8(R0, R1));
      }
    }
    else
    {
      __m128i const O  = _mm_setzero_si128();
      __m128i const C0 = _mm_set1_epi16((int16_t)((8 - xfrac) * 8));
      __m128i const C1 = _mm_set1_epi16((int16_t)((xfrac    ) * 8));
      for (int y = 0; y < 4; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
        __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
        __m128i xmm1 = _mm_unpacklo_epi8(R1, O);
        __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
        __m128i xmm5 = _mm_unpackhi_epi8(R1, O);
        xmm0 = _mm_mullo_epi16(xmm0, C0);
        xmm1 = _mm_mullo_epi16(xmm1, C1);
        xmm4 = _mm_mullo_epi16(xmm4, C0);
        xmm5 = _mm_mullo_epi16(xmm5, C1);
        xmm1 = _mm_add_epi16(xmm1, xmm0);
        xmm1 = _mm_add_epi16(xmm1, _mm_set1_epi16(32));
        xmm1 = _mm_srai_epi16(xmm1, 6);
        xmm5 = _mm_add_epi16(xmm5, xmm4);
        xmm5 = _mm_add_epi16(xmm5, _mm_set1_epi16(32));
        xmm5 = _mm_srai_epi16(xmm5, 6);
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm1, xmm5));
      }
    }
  }
  else if (xfrac == 0)  // interpolate between 2 vertical
  {
    if (yfrac == 4)     // simple avg
    {
      for (int y = 0; y < 4; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));
        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_avg_epu8(R0, R2));
      }
    }
    else
    {
      __m128i const O  = _mm_setzero_si128();
      __m128i const C0 = _mm_set1_epi16((int16_t)(8 * (8 - yfrac)));
      __m128i const C2 = _mm_set1_epi16((int16_t)(8 * (yfrac    )));

      for (int y = 0; y < 4; y++)
      {
        __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
        __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));

        __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
        __m128i xmm2 = _mm_unpacklo_epi8(R2, O);
        __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
        __m128i xmm6 = _mm_unpackhi_epi8(R2, O);

        xmm0 = _mm_mullo_epi16(xmm0, C0);
        xmm2 = _mm_mullo_epi16(xmm2, C2);
        xmm4 = _mm_mullo_epi16(xmm4, C0);
        xmm6 = _mm_mullo_epi16(xmm6, C2);

        xmm2 = _mm_add_epi16(xmm2, xmm0);
        xmm2 = _mm_add_epi16(xmm2, _mm_set1_epi16(32));
        xmm2 = _mm_srai_epi16(xmm2, 6);

        xmm6 = _mm_add_epi16(xmm6, xmm4);
        xmm6 = _mm_add_epi16(xmm6, _mm_set1_epi16(32));
        xmm6 = _mm_srai_epi16(xmm6, 6);

        _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm2, xmm6));
      }
    }
  }
  else
  {
    __m128i const O  = _mm_setzero_si128();
    __m128i const C0 = _mm_set1_epi16((int16_t)((8 - xfrac) * (8 - yfrac)));
    __m128i const C1 = _mm_set1_epi16((int16_t)((xfrac    ) * (8 - yfrac)));
    __m128i const C2 = _mm_set1_epi16((int16_t)((8 - xfrac) * (yfrac    )));
    __m128i const C3 = _mm_set1_epi16((int16_t)((xfrac    ) * (yfrac    )));

    for (int y = 0; y < 4; y++)
    {
      __m128i R0 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+0])), *((double *)(&full_u[cstride*(y+0)+0]))));
      __m128i R1 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+0)+1])), *((double *)(&full_u[cstride*(y+0)+1]))));
      __m128i R2 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+0])), *((double *)(&full_u[cstride*(y+1)+0]))));
      __m128i R3 = _mm_castpd_si128(_mm_set_pd (*((double *)(&full_v[cstride*(y+1)+1])), *((double *)(&full_u[cstride*(y+1)+1]))));

      __m128i xmm0 = _mm_unpacklo_epi8(R0, O);
      __m128i xmm1 = _mm_unpacklo_epi8(R1, O);
      __m128i xmm2 = _mm_unpacklo_epi8(R2, O);
      __m128i xmm3 = _mm_unpacklo_epi8(R3, O);
      __m128i xmm4 = _mm_unpackhi_epi8(R0, O);
      __m128i xmm5 = _mm_unpackhi_epi8(R1, O);
      __m128i xmm6 = _mm_unpackhi_epi8(R2, O);
      __m128i xmm7 = _mm_unpackhi_epi8(R3, O);

      xmm0 = _mm_mullo_epi16(xmm0, C0);
      xmm1 = _mm_mullo_epi16(xmm1, C1);
      xmm2 = _mm_mullo_epi16(xmm2, C2);
      xmm3 = _mm_mullo_epi16(xmm3, C3);
      xmm4 = _mm_mullo_epi16(xmm4, C0);
      xmm5 = _mm_mullo_epi16(xmm5, C1);
      xmm6 = _mm_mullo_epi16(xmm6, C2);
      xmm7 = _mm_mullo_epi16(xmm7, C3);

      xmm1 = _mm_add_epi16(xmm1, xmm0);
      xmm2 = _mm_add_epi16(xmm2, xmm1);
      xmm3 = _mm_add_epi16(xmm3, xmm2);
      xmm3 = _mm_add_epi16(xmm3, _mm_set1_epi16(32));
      xmm3 = _mm_srai_epi16(xmm3, 6);

      xmm5 = _mm_add_epi16(xmm5, xmm4);
      xmm6 = _mm_add_epi16(xmm6, xmm5);
      xmm7 = _mm_add_epi16(xmm7, xmm6);
      xmm7 = _mm_add_epi16(xmm7, _mm_set1_epi16(32));
      xmm7 = _mm_srai_epi16(xmm7, 6);

      _mm_store_si128((__m128i *)&best_uv[16*y], _mm_packus_epi16(xmm3, xmm7));
    }
  }
}

void taa_h264_save_best_chroma_4x8(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *  best_uv
  )
{
  if ((xfrac == 0) && (yfrac == 0))
  {
    for (int y = 0; y < 8; y++)
    {
      *(int *)(&best_uv[16*y + 0]) = *(int *)(&full_u[cstride*y]);
      *(int *)(&best_uv[16*y + 8]) = *(int *)(&full_v[cstride*y]);
    }
  }
  else if (yfrac == 0)  // interpolate between 2 horizontal
  {
    if (xfrac == 4)     // simple avg
    {
      for (int y = 0; y < 8; y++)
      {
        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+0] = (uint8_t)((full_u[cstride*(y+0)+x+0] + full_u[cstride*(y+0)+x+1] + 1) >> 1);
          best_uv[16*y+x+8] = (uint8_t)((full_v[cstride*(y+0)+x+0] + full_v[cstride*(y+0)+x+1] + 1) >> 1);
        }
      }
    }
    else
    {
      const int c0 = (8 - xfrac) * (8 - yfrac);
      const int c1 = (xfrac    ) * (8 - yfrac);
      for (int y = 0; y < 8; y++)
      {
        uint8_t r0[4];
        uint8_t r1[4];
        *(int *)&r0[0] = *(int *)&full_u[cstride*(y+0)+0];
        *(int *)&r1[0] = *(int *)&full_u[cstride*(y+0)+1];
        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+0] = (uint8_t)((c0 * r0[x] + c1 * r1[x] + 32) >> 6);
        }
      }
      for (int y = 0; y < 8; y++)
      {
        uint8_t r0[4];
        uint8_t r1[4];
        *(int *)&r0[0] = *(int *)&full_v[cstride*(y+0)+0];
        *(int *)&r1[0] = *(int *)&full_v[cstride*(y+0)+1];
        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+8] = (uint8_t)((c0 * r0[x] + c1 * r1[x] + 32) >> 6);
        }
      }
    }
  }
  else if (xfrac == 0)  // interpolate between 2 vertical
  {
    if (yfrac == 4)     // simple avg
    {
      for (int y = 0; y < 8; y++)
      {
        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+0] = (uint8_t)((full_u[cstride*(y+0)+x+0] + full_u[cstride*(y+1)+x+0] + 1) >> 1);
          best_uv[16*y+x+8] = (uint8_t)((full_v[cstride*(y+0)+x+0] + full_v[cstride*(y+1)+x+0] + 1) >> 1);
        }
      }
    }
    else
    {
      const int c0 = (8 - xfrac) * (8 - yfrac);
      const int c2 = (8 - xfrac) * (yfrac    );

      for (int y = 0; y < 8; y++)
      {
        uint8_t r0[4];
        uint8_t r2[4];

        *(int *)&r0[0] = *(int *)&full_u[cstride*(y+0)+0];
        *(int *)&r2[0] = *(int *)&full_u[cstride*(y+1)+0];

        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+0] = (uint8_t)((c0 * r0[x] + c2 * r2[x] + 32) >> 6);
        }
      }
      for (int y = 0; y < 8; y++)
      {
        uint8_t r0[4];
        uint8_t r2[4];

        *(int *)&r0[0] = *(int *)&full_v[cstride*(y+0)+0];
        *(int *)&r2[0] = *(int *)&full_v[cstride*(y+1)+0];
        for (int x = 0; x < 4; x++)
        {
          best_uv[16*y+x+8] = (uint8_t)((c0 * r0[x] + c2 * r2[x] + 32) >> 6);
        }
      }
    }
  }
  else                   // interpolate between all 4 points
  {
    const int c0 = (8 - xfrac) * (8 - yfrac);
    const int c1 = (xfrac    ) * (8 - yfrac);
    const int c2 = (8 - xfrac) * (yfrac    );
    const int c3 = (xfrac    ) * (yfrac    );

    for (int y = 0; y < 8; y++)
    {
      uint8_t r0[4];
      uint8_t r1[4];
      uint8_t r2[4];
      uint8_t r3[4];

      *(int *)&r0[0] = *(int *)&full_u[cstride*(y+0)+0];
      *(int *)&r1[0] = *(int *)&full_u[cstride*(y+0)+1];
      *(int *)&r2[0] = *(int *)&full_u[cstride*(y+1)+0];
      *(int *)&r3[0] = *(int *)&full_u[cstride*(y+1)+1];

      for (int x = 0; x < 4; x++)
      {
        best_uv[16*y+x+0] = (uint8_t)((c0 * r0[x] + c1 * r1[x] + c2 * r2[x] + c3 * r3[x] + 32) >> 6);
      }
    }
    for (int y = 0; y < 8; y++)
    {
      uint8_t r0[4];
      uint8_t r1[4];
      uint8_t r2[4];
      uint8_t r3[4];

      *(int *)&r0[0] = *(int *)&full_v[cstride*(y+0)+0];
      *(int *)&r1[0] = *(int *)&full_v[cstride*(y+0)+1];
      *(int *)&r2[0] = *(int *)&full_v[cstride*(y+1)+0];
      *(int *)&r3[0] = *(int *)&full_v[cstride*(y+1)+1];
      for (int x = 0; x < 4; x++)
      {
        best_uv[16*y+x+8] = (uint8_t)((c0 * r0[x] + c1 * r1[x] + c2 * r2[x] + c3 * r3[x] + 32) >> 6);
      }
    }
  }
}

#if defined(_M_X64) || defined (__x86_64__) || !defined (__INTEL_COMPILER)
static
void taa_h264_save_16x16_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 8)
  {
    for (int x = 0; x < 16; x++) best[(y + 0) * 16 + x] = fpel[(y + 0) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 1) * 16 + x] = fpel[(y + 1) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 2) * 16 + x] = fpel[(y + 2) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 3) * 16 + x] = fpel[(y + 3) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 4) * 16 + x] = fpel[(y + 4) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 5) * 16 + x] = fpel[(y + 5) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 6) * 16 + x] = fpel[(y + 6) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 7) * 16 + x] = fpel[(y + 7) * stride + x];
  }
}

static
void taa_h264_save_16x16_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);

    // E - 5 * F + 20 * G + 20 * H - 5 * I + J
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);


    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));

    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));

    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)hpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)&best[y * 16], xmm14);
  }
}

static
void taa_h264_save_16x16_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)hpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x + 1] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)vpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_2, pos_10));
  }
}

static
void taa_h264_save_16x16_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)&best[y * 16], xmm14);
  }
}

static
void taa_h264_save_16x16_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_8, pos_10));
  }
}

static
void taa_h264_save_16x16_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    xmm3 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], xmm3);
  }
}

static
void taa_h264_save_16x16_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_8, pos_10));
  }
}

static
void taa_h264_save_16x16_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)vpel, xmm14);

    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[(y + 1) * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 16; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_2, pos_10));
  }
}

static
void taa_h264_save_16x16_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 16; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}


#else

static
void taa_h264_save_16x16_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 8)
  {
    for (int x = 0; x < 16; x++) best[(y + 0) * 16 + x] = fpel[(y + 0) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 1) * 16 + x] = fpel[(y + 1) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 2) * 16 + x] = fpel[(y + 2) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 3) * 16 + x] = fpel[(y + 3) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 4) * 16 + x] = fpel[(y + 4) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 5) * 16 + x] = fpel[(y + 5) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 6) * 16 + x] = fpel[(y + 6) * stride + x];
    for (int x = 0; x < 16; x++) best[(y + 7) * 16 + x] = fpel[(y + 7) * stride + x];
  }
}

static
void taa_h264_save_16x16_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      hpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x16_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      hpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x + 1] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x16_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x16_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[(y + 1) * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x16_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 16; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}
#endif

void taa_h264_save_best_16x16(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *  best
  )
{
  switch(zfrac)
  {
  case 0: taa_h264_save_16x16_0(stride, fpel, best); break;
  case 1: taa_h264_save_16x16_1(stride, fpel, best); break;
  case 2: taa_h264_save_16x16_2(stride, fpel, best); break;
  case 3: taa_h264_save_16x16_3(stride, fpel, best); break;
  case 4: taa_h264_save_16x16_4(stride, fpel, best); break;
  case 5: taa_h264_save_16x16_5(stride, fpel, best); break;
  case 6: taa_h264_save_16x16_6(stride, fpel, best); break;
  case 7: taa_h264_save_16x16_7(stride, fpel, best); break;
  case 8: taa_h264_save_16x16_8(stride, fpel, best); break;
  case 9: taa_h264_save_16x16_9(stride, fpel, best); break;
  case 10: taa_h264_save_16x16_10(stride, fpel, best); break;
  case 11: taa_h264_save_16x16_11(stride, fpel, best); break;
  case 12: taa_h264_save_16x16_12(stride, fpel, best); break;
  case 13: taa_h264_save_16x16_13(stride, fpel, best); break;
  case 14: taa_h264_save_16x16_14(stride, fpel, best); break;
  case 15: taa_h264_save_16x16_15(stride, fpel, best); break;
  }
}


#if defined(_M_X64) || defined (__x86_64__) || !defined (__INTEL_COMPILER)
static
void taa_h264_save_16x8_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);

  for (int x = 0; x < 16; x++) best[0 * 16 + x] = fpel[0 * stride + x];
  for (int x = 0; x < 16; x++) best[1 * 16 + x] = fpel[1 * stride + x];
  for (int x = 0; x < 16; x++) best[2 * 16 + x] = fpel[2 * stride + x];
  for (int x = 0; x < 16; x++) best[3 * 16 + x] = fpel[3 * stride + x];
  for (int x = 0; x < 16; x++) best[4 * 16 + x] = fpel[4 * stride + x];
  for (int x = 0; x < 16; x++) best[5 * 16 + x] = fpel[5 * stride + x];
  for (int x = 0; x < 16; x++) best[6 * 16 + x] = fpel[6 * stride + x];
  for (int x = 0; x < 16; x++) best[7 * 16 + x] = fpel[7 * stride + x];
}

static
void taa_h264_save_16x8_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)hpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)&best[y * 16], xmm14);
  }
}

static
void taa_h264_save_16x8_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)hpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x + 1] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)vpel, xmm14);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_2, pos_10));
  }
}

static
void taa_h264_save_16x8_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)&best[y * 16], xmm14);
  }
}

static
void taa_h264_save_16x8_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_8, pos_10));
  }
}

static
void taa_h264_save_16x8_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    xmm3 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], xmm3);
  }
}

static
void taa_h264_save_16x8_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_8, pos_10));
  }
}

static
void taa_h264_save_16x8_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
    xmm8  = _mm_unpackhi_epi8(xmm0, O);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm9  = _mm_unpackhi_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm10 = _mm_unpackhi_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm11 = _mm_unpackhi_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm12 = _mm_unpackhi_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm13 = _mm_unpackhi_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_store_si128((__m128i *)vpel, xmm14);

    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[(y + 1) * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[24];
  __assume_aligned(best, 64);

  for (int y = 0; y < 8; y++)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[0], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[8], xmm6);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 12]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 12]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 12]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 12]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 12]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 12]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    _mm_store_si128((__m128i *)&vert[16], xmm6);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm2 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[12]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[16]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[13]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[17]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[14]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm6 = _mm_loadl_epi64((__m128i *)&vert[15]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    _mm_store_si128((__m128i *)&best[y * 16], _mm_avg_epu8(pos_2, pos_10));
  }
}

static
void taa_h264_save_16x8_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  for (int y = 0; y < 8; y++)
  {
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
      xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
      xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
      xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
      xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
      xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 6]);
      xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 7]);
      xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 8]);
      xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 9]);
      xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 10]);
      xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 11]);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm8  = _mm_unpacklo_epi8(xmm8, O);
      xmm9  = _mm_unpacklo_epi8(xmm9, O);
      xmm10 = _mm_unpacklo_epi8(xmm10, O);
      xmm11 = _mm_unpacklo_epi8(xmm11, O);
      xmm12 = _mm_unpacklo_epi8(xmm12, O);
      xmm13 = _mm_unpacklo_epi8(xmm13, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)hpel, xmm14);
    }
    {
      const __m128i O = _mm_setzero_si128();
      __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
      xmm0  = _mm_loadu_si128((__m128i *)&fpel[(y - 2) * stride + 1]);
      xmm1  = _mm_loadu_si128((__m128i *)&fpel[(y - 1) * stride + 1]);
      xmm2  = _mm_loadu_si128((__m128i *)&fpel[(y + 0) * stride + 1]);
      xmm3  = _mm_loadu_si128((__m128i *)&fpel[(y + 1) * stride + 1]);
      xmm4  = _mm_loadu_si128((__m128i *)&fpel[(y + 2) * stride + 1]);
      xmm5  = _mm_loadu_si128((__m128i *)&fpel[(y + 3) * stride + 1]);
      xmm8  = _mm_unpackhi_epi8(xmm0, O);
      xmm0  = _mm_unpacklo_epi8(xmm0, O);
      xmm9  = _mm_unpackhi_epi8(xmm1, O);
      xmm1  = _mm_unpacklo_epi8(xmm1, O);
      xmm10 = _mm_unpackhi_epi8(xmm2, O);
      xmm2  = _mm_unpacklo_epi8(xmm2, O);
      xmm11 = _mm_unpackhi_epi8(xmm3, O);
      xmm3  = _mm_unpacklo_epi8(xmm3, O);
      xmm12 = _mm_unpackhi_epi8(xmm4, O);
      xmm4  = _mm_unpacklo_epi8(xmm4, O);
      xmm13 = _mm_unpackhi_epi8(xmm5, O);
      xmm5  = _mm_unpacklo_epi8(xmm5, O);
      xmm6  = _mm_add_epi16(xmm2, xmm3);
      xmm14 = _mm_add_epi16(xmm10, xmm11);
      xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
      xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
      xmm7  = _mm_add_epi16(xmm1, xmm4);
      xmm15 = _mm_add_epi16(xmm9, xmm12);
      xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
      xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
      xmm6  = _mm_sub_epi16(xmm6, xmm7);
      xmm14 = _mm_sub_epi16(xmm14, xmm15);
      xmm6  = _mm_add_epi16(xmm6, xmm0);
      xmm14 = _mm_add_epi16(xmm14, xmm8);
      xmm6  = _mm_add_epi16(xmm6, xmm5);
      xmm14 = _mm_add_epi16(xmm14, xmm13);
      xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
      xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
      xmm6  = _mm_srai_epi16 (xmm6, 5);
      xmm14 = _mm_srai_epi16 (xmm14, 5);
      xmm14 = _mm_packus_epi16(xmm6, xmm14);
      _mm_store_si128((__m128i *)vpel, xmm14);
    }
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((hpel[x] + vpel[x] + 1) >> 1);
  }
}

#else

static
void taa_h264_save_16x8_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = fpel[y * stride + x];
}

static
void taa_h264_save_16x8_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      hpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x8_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t hpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      hpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x + 1] + hpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[y * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[y * stride + x] + p3[y * stride + x]) + (-5) * (p1[y * stride + x] + p4[y * stride + x]) + p0[y * stride + x] + p5[y * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x8_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
}

static
void taa_h264_save_16x8_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((fpel[(y + 1) * stride + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x] + fpel[(y + 1) * stride + x]) + (-5) * (fpel[(y - 1) * stride + x] + fpel[(y + 2) * stride + x]) + fpel[(y - 2) * stride + x] + fpel[(y + 3) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  TAA_H264_ALIGN(64) int32_t buff[16];
  TAA_H264_ALIGN(64) int16_t vert[24];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  const int16_t * q0 = &vert[4] - 2;
  const int16_t * q1 = &vert[4] + a[1];
  const int16_t * q2 = &vert[4] + a[2];
  const int16_t * q3 = &vert[4] + a[3];
  const int16_t * q4 = &vert[4] + a[4];
  const int16_t * q5 = &vert[4] + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
    for (int x = 0; x < 24; x++)
      vert[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x - 4] + fpel[(y + 1) * stride + x - 4]) + (-5) * (fpel[(y - 1) * stride + x - 4] + fpel[(y + 2) * stride + x - 4]) + fpel[(y - 2) * stride + x - 4] + fpel[(y + 3) * stride + x - 4]);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x]  = q0[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q1[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q2[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q3[x] * 20;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q4[x] * (-5);
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += q5[x];
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] += 512;
# pragma unroll(4)
    for(int x = 0; x < 16; x++)
      buff[x] = buff[x] >> 10;
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t) ((buff[x] > 32767) ? 32767 : (buff[x] < -32768) ? -32768 : buff[x]);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}

static
void taa_h264_save_16x8_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) uint8_t vpel[16];
  TAA_H264_ALIGN(64) int16_t temp[16];
  const int a[6] = { -2, -1, 0, 1, 2, 3};
  const unsigned char * p0 = fpel - 2;
  const unsigned char * p1 = fpel + a[1];
  const unsigned char * p2 = fpel + a[2];
  const unsigned char * p3 = fpel + a[3];
  const unsigned char * p4 = fpel + a[4];
  const unsigned char * p5 = fpel + a[5];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for (int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (p2[(y + 1) * stride + x] + p3[(y + 1) * stride + x]) + (-5) * (p1[(y + 1) * stride + x] + p4[(y + 1) * stride + x]) + p0[(y + 1) * stride + x] + p5[(y + 1) * stride + x] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
  }
  for (int y = 0; y < 8; y++)
  {
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(20 * (fpel[(y + 0) * stride + x + 1] + fpel[(y + 1) * stride + x + 1]) + (-5) * (fpel[(y - 1) * stride + x + 1] + fpel[(y + 2) * stride + x + 1]) + fpel[(y - 2) * stride + x + 1] + fpel[(y + 3) * stride + x + 1] + 16);
# pragma unroll(2)
    for(int x = 0; x < 16; x++)
      temp[x] = (int16_t)(temp[x] >> 5);
    for(int x = 0; x < 16; x++)
      vpel[x] = (uint8_t)((temp[x] > 255) ? 255 : (temp[x] < 0) ? 0 : temp[x]);
    for (int x = 0; x < 16; x++)
      best[y * 16 + x] = (uint8_t)((best[y * 16 + x] + vpel[x] + 1) >> 1);
  }
}
#endif

void taa_h264_save_best_16x8(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *  best
  )
{
  switch(zfrac)
  {
  case 0: taa_h264_save_16x8_0(stride, fpel, best); break;
  case 1: taa_h264_save_16x8_1(stride, fpel, best); break;
  case 2: taa_h264_save_16x8_2(stride, fpel, best); break;
  case 3: taa_h264_save_16x8_3(stride, fpel, best); break;
  case 4: taa_h264_save_16x8_4(stride, fpel, best); break;
  case 5: taa_h264_save_16x8_5(stride, fpel, best); break;
  case 6: taa_h264_save_16x8_6(stride, fpel, best); break;
  case 7: taa_h264_save_16x8_7(stride, fpel, best); break;
  case 8: taa_h264_save_16x8_8(stride, fpel, best); break;
  case 9: taa_h264_save_16x8_9(stride, fpel, best); break;
  case 10: taa_h264_save_16x8_10(stride, fpel, best); break;
  case 11: taa_h264_save_16x8_11(stride, fpel, best); break;
  case 12: taa_h264_save_16x8_12(stride, fpel, best); break;
  case 13: taa_h264_save_16x8_13(stride, fpel, best); break;
  case 14: taa_h264_save_16x8_14(stride, fpel, best); break;
  case 15: taa_h264_save_16x8_15(stride, fpel, best); break;
  }
}

static
void taa_h264_save_8x16_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y++)
  {
    _mm_storel_epi64((__m128i *)&best[y * 16], _mm_loadl_epi64((__m128i *)&fpel[y * stride]));
  }
}

static
void taa_h264_save_8x16_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride])), *(((double *)(&fpel[(y + 0) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_storel_pd((double *)&best[(y + 0) * 16], _mm_castsi128_pd(xmm14));
    _mm_storeh_pd((double *)&best[(y + 1) * 16], _mm_castsi128_pd(xmm14));
  }
}

static
void taa_h264_save_8x16_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride + 1])), *(((double *)(&fpel[(y + 0) * stride + 1]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  //__assume_aligned(best, 64);
  __assume_aligned(best, 8);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride])), *(((double *)(&fpel[(y + 0) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  //__assume_aligned(best, 64);
  __assume_aligned(best, 8);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x16_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_2, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x16_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_storel_pd((double *)&best[(y + 0) * 16], _mm_castsi128_pd(xmm14));
    _mm_storeh_pd((double *)&best[(y + 1) * 16], _mm_castsi128_pd(xmm14));
  }
}

static
void taa_h264_save_8x16_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_8, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    xmm3 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(xmm3);
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_8, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    __m128d const avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 2) * stride])), *(((double *)(&fpel[(y + 1) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x16_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_2, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x16_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 16; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}


void taa_h264_save_best_8x16(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *  best
  )
{
  switch(zfrac)
  {
  case 0: taa_h264_save_8x16_0(stride, fpel, best); break;
  case 1: taa_h264_save_8x16_1(stride, fpel, best); break;
  case 2: taa_h264_save_8x16_2(stride, fpel, best); break;
  case 3: taa_h264_save_8x16_3(stride, fpel, best); break;
  case 4: taa_h264_save_8x16_4(stride, fpel, best); break;
  case 5: taa_h264_save_8x16_5(stride, fpel, best); break;
  case 6: taa_h264_save_8x16_6(stride, fpel, best); break;
  case 7: taa_h264_save_8x16_7(stride, fpel, best); break;
  case 8: taa_h264_save_8x16_8(stride, fpel, best); break;
  case 9: taa_h264_save_8x16_9(stride, fpel, best); break;
  case 10: taa_h264_save_8x16_10(stride, fpel, best); break;
  case 11: taa_h264_save_8x16_11(stride, fpel, best); break;
  case 12: taa_h264_save_8x16_12(stride, fpel, best); break;
  case 13: taa_h264_save_8x16_13(stride, fpel, best); break;
  case 14: taa_h264_save_8x16_14(stride, fpel, best); break;
  case 15: taa_h264_save_8x16_15(stride, fpel, best); break;
  }
}

static
void taa_h264_save_8x8_0(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y++)
  {
    _mm_storel_epi64((__m128i *)&best[y * 16], _mm_loadl_epi64((__m128i *)&fpel[y * stride]));
  }
}

static
void taa_h264_save_8x8_1(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride])), *(((double *)(&fpel[(y + 0) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_2(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_storel_pd((double *)&best[(y + 0) * 16], _mm_castsi128_pd(xmm14));
    _mm_storeh_pd((double *)&best[(y + 1) * 16], _mm_castsi128_pd(xmm14));
  }
}

static
void taa_h264_save_8x8_3(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride + 1])), *(((double *)(&fpel[(y + 0) * stride + 1]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_4(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    const __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 1) * stride])), *(((double *)(&fpel[(y + 0) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_5(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x8_6(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_2, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_7(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[y * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[y * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x8_8(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    _mm_storel_pd((double *)&best[(y + 0) * 16], _mm_castsi128_pd(xmm14));
    _mm_storeh_pd((double *)&best[(y + 1) * 16], _mm_castsi128_pd(xmm14));
  }
}

static
void taa_h264_save_8x8_9(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_8, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_10(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    xmm3 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(xmm3);
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_11(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_8 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_8, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_12(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    xmm14 = _mm_packus_epi16(xmm6, xmm14);
    __m128d const avg = _mm_castsi128_pd(_mm_avg_epu8(_mm_castpd_si128(_mm_set_pd(*((double *)(&fpel[(y + 2) * stride])), *(((double *)(&fpel[(y + 1) * stride]))))), xmm14));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_13(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}

static
void taa_h264_save_8x8_14(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  TAA_H264_ALIGN(64) int16_t vert[32];
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i pos_2 = _mm_packus_epi16(xmm6, xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride - 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride - 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride - 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride - 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride - 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 0], xmm6);
    _mm_store_si128((__m128i *)&vert[16], xmm14);

    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 4]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 4]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 4]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 4]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 4]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 4]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 4]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);

    _mm_store_si128((__m128i *)&vert[ 8], xmm6);
    _mm_store_si128((__m128i *)&vert[24], xmm14);

    xmm0 = _mm_loadl_epi64((__m128i *)&vert[2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
    xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
    xmm0 = _mm_srai_epi32(xmm0, 16);
    xmm1 = _mm_srai_epi32(xmm1, 16);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[3]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[4]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[8]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[5]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[9]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(20));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(20));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[6]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[10]);
    xmm4 = _mm_unpacklo_epi16(xmm4, O);
    xmm5 = _mm_unpacklo_epi16(xmm5, O);
    xmm4 = _mm_madd_epi16(xmm4, _mm_set1_epi32(-5));
    xmm5 = _mm_madd_epi16(xmm5, _mm_set1_epi32(-5));
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm4 = _mm_loadl_epi64((__m128i *)&vert[7]);
    xmm5 = _mm_loadl_epi64((__m128i *)&vert[11]);
    xmm4 = _mm_unpacklo_epi16(xmm4, xmm4);
    xmm5 = _mm_unpacklo_epi16(xmm5, xmm5);
    xmm4 = _mm_srai_epi32(xmm4, 16);
    xmm5 = _mm_srai_epi32(xmm5, 16);
    xmm0 = _mm_add_epi32(xmm0, xmm4);
    xmm1 = _mm_add_epi32(xmm1, xmm5);

    xmm0 = _mm_add_epi32(xmm0, _mm_set1_epi32(512));
    xmm1 = _mm_add_epi32(xmm1, _mm_set1_epi32(512));
    xmm0 = _mm_srai_epi32(xmm0, 10);
    xmm1 = _mm_srai_epi32(xmm1, 10);

    xmm2 = _mm_loadl_epi64((__m128i *)&vert[18]);
    xmm3 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
    xmm3 = _mm_unpacklo_epi16(xmm3, xmm3);
    xmm2 = _mm_srai_epi32(xmm2, 16);
    xmm3 = _mm_srai_epi32(xmm3, 16);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[19]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[20]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[24]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[21]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[25]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(20));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(20));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[22]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[26]);
    xmm6 = _mm_unpacklo_epi16(xmm6, O);
    xmm7 = _mm_unpacklo_epi16(xmm7, O);
    xmm6 = _mm_madd_epi16(xmm6, _mm_set1_epi32(-5));
    xmm7 = _mm_madd_epi16(xmm7, _mm_set1_epi32(-5));
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm6 = _mm_loadl_epi64((__m128i *)&vert[23]);
    xmm7 = _mm_loadl_epi64((__m128i *)&vert[27]);
    xmm6 = _mm_unpacklo_epi16(xmm6, xmm6);
    xmm7 = _mm_unpacklo_epi16(xmm7, xmm7);
    xmm6 = _mm_srai_epi32(xmm6, 16);
    xmm7 = _mm_srai_epi32(xmm7, 16);
    xmm2 = _mm_add_epi32(xmm2, xmm6);
    xmm3 = _mm_add_epi32(xmm3, xmm7);

    xmm2 = _mm_add_epi32(xmm2, _mm_set1_epi32(512));
    xmm3 = _mm_add_epi32(xmm3, _mm_set1_epi32(512));
    xmm2 = _mm_srai_epi32(xmm2, 10);
    xmm3 = _mm_srai_epi32(xmm3, 10);

    xmm1 = _mm_packs_epi32(xmm0, xmm1);
    xmm3 = _mm_packs_epi32(xmm2, xmm3);
    __m128i pos_10 = _mm_packus_epi16(xmm1, xmm3);

    __m128d avg = _mm_castsi128_pd(_mm_avg_epu8(pos_2, pos_10));
    _mm_storel_pd((double *)&best[(y + 0) * 16], avg);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], avg);
  }
}

static
void taa_h264_save_8x8_15(
  const int          stride,
  uint8_t const *    fpel,
  uint8_t *  best)
{
  __assume_aligned(best, 64);
  for (int y = 0; y < 8; y += 2)
  {
    const __m128i O = _mm_setzero_si128();
    __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
    xmm0 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 2]);
    xmm1 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride - 1]);
    xmm2 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 0]);
    xmm3 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 2]);
    xmm5 = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 3]);
    xmm8  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 2]);
    xmm9  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride - 1]);
    xmm10 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 0]);
    xmm11 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm12 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 2]);
    xmm13 = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 3]);
    xmm0 = _mm_unpacklo_epi8(xmm0, O);
    xmm1 = _mm_unpacklo_epi8(xmm1, O);
    xmm2 = _mm_unpacklo_epi8(xmm2, O);
    xmm3 = _mm_unpacklo_epi8(xmm3, O);
    xmm4 = _mm_unpacklo_epi8(xmm4, O);
    xmm5 = _mm_unpacklo_epi8(xmm5, O);
    xmm8  = _mm_unpacklo_epi8(xmm8, O);
    xmm9  = _mm_unpacklo_epi8(xmm9, O);
    xmm10 = _mm_unpacklo_epi8(xmm10, O);
    xmm11 = _mm_unpacklo_epi8(xmm11, O);
    xmm12 = _mm_unpacklo_epi8(xmm12, O);
    xmm13 = _mm_unpacklo_epi8(xmm13, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i hpel2 = _mm_packus_epi16(xmm6, xmm14);
    xmm0  = _mm_loadl_epi64((__m128i *)&fpel[(y - 2) * stride + 1]);
    xmm1  = _mm_loadl_epi64((__m128i *)&fpel[(y - 1) * stride + 1]);
    xmm2  = _mm_loadl_epi64((__m128i *)&fpel[(y + 0) * stride + 1]);
    xmm3  = _mm_loadl_epi64((__m128i *)&fpel[(y + 1) * stride + 1]);
    xmm4  = _mm_loadl_epi64((__m128i *)&fpel[(y + 2) * stride + 1]);
    xmm5  = _mm_loadl_epi64((__m128i *)&fpel[(y + 3) * stride + 1]);
    xmm6  = _mm_loadl_epi64((__m128i *)&fpel[(y + 4) * stride + 1]);
    xmm0  = _mm_unpacklo_epi8(xmm0, O);
    xmm8  = _mm_unpacklo_epi8(xmm1, O);
    xmm1  = _mm_unpacklo_epi8(xmm1, O);
    xmm9  = _mm_unpacklo_epi8(xmm2, O);
    xmm2  = _mm_unpacklo_epi8(xmm2, O);
    xmm10 = _mm_unpacklo_epi8(xmm3, O);
    xmm3  = _mm_unpacklo_epi8(xmm3, O);
    xmm11 = _mm_unpacklo_epi8(xmm4, O);
    xmm4  = _mm_unpacklo_epi8(xmm4, O);
    xmm12 = _mm_unpacklo_epi8(xmm5, O);
    xmm5  = _mm_unpacklo_epi8(xmm5, O);
    xmm13 = _mm_unpacklo_epi8(xmm6, O);
    xmm6  = _mm_add_epi16(xmm2, xmm3);
    xmm14 = _mm_add_epi16(xmm10, xmm11);
    xmm6  = _mm_mullo_epi16(xmm6, _mm_set1_epi16(20));
    xmm14 = _mm_mullo_epi16(xmm14, _mm_set1_epi16(20));
    xmm7  = _mm_add_epi16(xmm1, xmm4);
    xmm15 = _mm_add_epi16(xmm9, xmm12);
    xmm7  = _mm_mullo_epi16(xmm7, _mm_set1_epi16(5));
    xmm15 = _mm_mullo_epi16(xmm15, _mm_set1_epi16(5));
    xmm6  = _mm_sub_epi16(xmm6, xmm7);
    xmm14 = _mm_sub_epi16(xmm14, xmm15);
    xmm6  = _mm_add_epi16(xmm6, xmm0);
    xmm14 = _mm_add_epi16(xmm14, xmm8);
    xmm6  = _mm_add_epi16(xmm6, xmm5);
    xmm14 = _mm_add_epi16(xmm14, xmm13);
    xmm6  = _mm_add_epi16(xmm6, _mm_set1_epi16(16));
    xmm14 = _mm_add_epi16(xmm14, _mm_set1_epi16(16));
    xmm6  = _mm_srai_epi16 (xmm6, 5);
    xmm14 = _mm_srai_epi16 (xmm14, 5);
    __m128i vpel2 = _mm_packus_epi16(xmm6, xmm14);
    __m128d zpel = _mm_castsi128_pd(_mm_avg_epu8(hpel2, vpel2));
    _mm_storel_pd((double *)&best[(y + 0) * 16], zpel);
    _mm_storeh_pd((double *)&best[(y + 1) * 16], zpel);
  }
}


void taa_h264_save_best_8x8(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *  best
  )
{
  switch(zfrac)
  {
  case 0: taa_h264_save_8x8_0(stride, fpel, best); break;
  case 1: taa_h264_save_8x8_1(stride, fpel, best); break;
  case 2: taa_h264_save_8x8_2(stride, fpel, best); break;
  case 3: taa_h264_save_8x8_3(stride, fpel, best); break;
  case 4: taa_h264_save_8x8_4(stride, fpel, best); break;
  case 5: taa_h264_save_8x8_5(stride, fpel, best); break;
  case 6: taa_h264_save_8x8_6(stride, fpel, best); break;
  case 7: taa_h264_save_8x8_7(stride, fpel, best); break;
  case 8: taa_h264_save_8x8_8(stride, fpel, best); break;
  case 9: taa_h264_save_8x8_9(stride, fpel, best); break;
  case 10: taa_h264_save_8x8_10(stride, fpel, best); break;
  case 11: taa_h264_save_8x8_11(stride, fpel, best); break;
  case 12: taa_h264_save_8x8_12(stride, fpel, best); break;
  case 13: taa_h264_save_8x8_13(stride, fpel, best); break;
  case 14: taa_h264_save_8x8_14(stride, fpel, best); break;
  case 15: taa_h264_save_8x8_15(stride, fpel, best); break;
  }
}
