/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#include <stdio.h>

typedef unsigned char byte;
#define TRUE 1
#define FALSE 0
#define Q_BITS 15
#define MAX_VALUE       999999

/* [k, j, i] = [qp%6, column, row] */
const int quant_coef_jm[6][4][4] = {
  { { 13107, 8066, 13107, 8066}, { 8066, 5243, 8066, 5243}, { 13107, 8066, 13107, 8066}, { 8066, 5243, 8066, 5243}},
  { { 11916, 7490, 11916, 7490}, { 7490, 4660, 7490, 4660}, { 11916, 7490, 11916, 7490}, { 7490, 4660, 7490, 4660}},
  { { 10082, 6554, 10082, 6554}, { 6554, 4194, 6554, 4194}, { 10082, 6554, 10082, 6554}, { 6554, 4194, 6554, 4194}},
  { { 9362, 5825, 9362, 5825}, { 5825, 3647, 5825, 3647}, { 9362, 5825, 9362, 5825}, { 5825, 3647, 5825, 3647}},
  { { 8192, 5243, 8192, 5243}, { 5243, 3355, 5243, 3355}, { 8192, 5243, 8192, 5243}, { 5243, 3355, 5243, 3355}},
  { { 7282, 4559, 7282, 4559}, { 4559, 2893, 4559, 2893}, { 7282, 4559, 7282, 4559}, { 4559, 2893, 4559, 2893}}
};

const int dequant_coef[6][4][4] = {
  { { 10, 13, 10, 13}, { 13, 16, 13, 16}, { 10, 13, 10, 13}, { 13, 16, 13, 16}},
  { { 11, 14, 11, 14}, { 14, 18, 14, 18}, { 11, 14, 11, 14}, { 14, 18, 14, 18}},
  { { 13, 16, 13, 16}, { 16, 20, 16, 20}, { 13, 16, 13, 16}, { 16, 20, 16, 20}},
  { { 14, 18, 14, 18}, { 18, 23, 18, 23}, { 14, 18, 14, 18}, { 18, 23, 18, 23}},
  { { 16, 20, 16, 20}, { 20, 25, 20, 25}, { 16, 20, 16, 20}, { 20, 25, 20, 25}},
  { { 18, 23, 18, 23}, { 23, 29, 23, 29}, { 18, 23, 18, 23}, { 23, 29, 23, 29}}
};

const byte pos_scan[16][2] =
{
  { 0, 0}, { 0, 1}, { 1, 0}, { 2, 0},
  { 1, 1}, { 0, 2}, { 0, 3}, { 1, 2},
  { 2, 1}, { 3, 0}, { 3, 1}, { 2, 2},
  { 1, 3}, { 2, 3}, { 3, 2}, { 3, 3}
};

const byte COEFF_COST4x4[2][16] =
{
  { 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9}
};

static inline int iabs(
  int x)
{
  return ((x) < 0) ? -(x) : (x);
}

static inline int isignab(
  int a,
  int b)
{
  return ((b) < 0) ? -iabs(a) : iabs(a);
}

static inline int rshift_rnd_sf(
  int x,
  int a)
{
  return ((x + (1 << (a - 1) )) >> a);
}


/* From the reference software encoder */


int quant_4x4_normal(
  int        tblock[4][4],
  int        block_y,
  int        block_x,
  int        qp,
  int *      ACLevel,
  int *      ACRun,
  int * *    fadjust4x4,
  const int  levelscale[4][4],
  const int  invlevelscale[4][4],
  int        leveloffset[4][4],
  int *      coeff_cost,
  const byte pos_scan[16][2],
  const byte c_cost[16])
{
  static int i, j, coeff_ctr;

  static int * m7;
  static int scaled_coeff;

  int level, run = 0;
  int nonzero = FALSE;
  int qp_per = qp / 6;
  int q_bits = Q_BITS + qp_per;           /* each 6th -> q_bits increases */
  const byte * p_scan = &pos_scan[0][0];
  int *  ACL = &ACLevel[0];
  int *  ACR = &ACRun[0];

  /* Quantization */
  printf ("quantization\n");
  for (coeff_ctr = 0; coeff_ctr < 16; coeff_ctr++)
  {
    printf ("-%d-\n", coeff_ctr);

    i = *p_scan++;      /* horizontal position */
    j = *p_scan++;      /* vertical position */

    m7 = &tblock[j][block_x + i];
    scaled_coeff = iabs (*m7) * levelscale[j][i];
    level = (scaled_coeff + leveloffset[j][i]) >> q_bits;
    printf ("m7             = %d\n", *m7);
    printf ("levelscale     = %d\n", levelscale[j][i]);
    printf ("q_bits         = %d\n", q_bits);
    printf ("level (so far) = %d\n", level);
    if (level != 0)
    {
      /* if (params->symbol_mode == CAVLC && img->qp < 10) */
      /*     level = imin(level, CAVLC_LEVEL_LIMIT); */

      nonzero = TRUE;

      *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

      level  = isignab(level, *m7);
      *m7    = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 4);
      /* inverse scale can be alternative performed as follows to ensure 16bit */
      /* arithmetic is satisfied. */
      /* *m7 = (qp_per<4) ? rshift_rnd_sf((level*invlevelscale[j][i]),4-qp_per) : (level*invlevelscale[j][i])<<(qp_per-4); */
      *ACL++ = level;
      *ACR++ = run;
      /* reset zero level counter */
      run    = 0;
    }
    else
    {
      run++;
      *m7 = 0;
    }
    printf ("level          = %d\n", level);
  }
  *ACL = 0;

  return nonzero;
}


static int
quant_ac4x4_normal(
  int        tblock[4][4],
  int        block_y,
  int        block_x,
  int        qp,
  int *      ACLevel,
  int *      ACRun,
  int * *    fadjust4x4,
  const int  levelscale[4][4],
  const int  invlevelscale[4][4],
  int        leveloffset[4][4],
  int *      coeff_cost,
  const byte pos_scan[16][2],
  const byte c_cost[16])
{
  static int i, j, coeff_ctr;

  static int * m7;
  static int scaled_coeff;

  int level, run = 0;
  int nonzero = FALSE;
  int qp_per = qp / 6;
  int q_bits = Q_BITS + qp_per;
  const byte * p_scan = &pos_scan[1][0];
  int *  ACL = &ACLevel[0];
  int *  ACR = &ACRun[0];

  /* Quantization */
  for (coeff_ctr = 1; coeff_ctr < 16; coeff_ctr++)
  {
    j = *p_scan++;      /* vertical position */
    i = *p_scan++;      /* horizontal position */

    m7 = &tblock[j][block_x + i];
    scaled_coeff = iabs (*m7) * levelscale[j][i];
    level = (scaled_coeff + leveloffset[j][i]) >> q_bits;

    if (level != 0)
    {
      /* What's this */
      /* if (params->symbol_mode == CAVLC && img->qp < 10) */
      /*     level = imin(level, CAVLC_LEVEL_LIMIT); */

      nonzero = TRUE;

      *coeff_cost += (level > 1) ? MAX_VALUE : c_cost[run];

      level  = isignab(level, *m7);
      *m7    = rshift_rnd_sf(((level * invlevelscale[j][i]) << qp_per), 4);
      printf ("coeff %5d dequant %2d ---> ", level, invlevelscale[j][i]);
      printf ("recons %5d\n", *m7);
      *ACL++ = level;
      *ACR++ = run;
      run    = 0;
    }
    else
    {
      run++;
      *m7 = 0;
    }
  }

  *ACL = 0;

  return nonzero;
}


static const int quant_coef[6][16] = {
  { 13107, 8066, 8066, 13107, 5243, 13107, 8066, 8066, 8066, 8066, 5243, 13107, 5243, 8066, 8066, 5243},
  { 11916, 7490, 7490, 11916, 4660, 11916, 7490, 7490, 7490, 7490, 4660, 11916, 4660, 7490, 7490, 4660},
  { 10082, 6554, 6554, 10082, 4194, 10082, 6554, 6554, 6554, 6554, 4194, 10082, 4194, 6554, 6554, 4194},
  { 9362, 5825, 5825,  9362, 3647,  9362, 5825, 5825, 5825, 5825, 3647,  9362, 3647, 5825, 5825, 3647},
  { 8192, 5243, 5243,  8192, 3355,  8192, 5243, 5243, 5243, 5243, 3355,  8192, 3355, 5243, 5243, 3355},
  { 7282, 4559, 4559,  7282, 2893,  7282, 4559, 4559, 4559, 4559, 2893,  7282, 2893, 4559, 4559, 2893}
};

static int
quantize_4x4(
  int   coeff[4][4],
  int   quant,
  int   inter,
  int   chroma,
  int * num_of_coeffs,
  int * sum_of_levels)
{
  int quant_div_6 = quant / 6;
  int quant_mod_6 = quant % 6;
  int q_bits = Q_BITS + quant_div_6;

  for (int x = 0; x < 16; x++)
  {
    /* Process in zig-zag order */
    int i = pos_scan[x][0];             /* row */
    int j = pos_scan[x][1];             /* column */
    int tmp = coeff[i][j] * quant_coef[quant_mod_6][x];
    int level;

    if (tmp < 0)
      level = -((-tmp) >> q_bits);
    else
      level = tmp >> q_bits;

    coeff[i][j] = level;
    *sum_of_levels += level;
    *num_of_coeffs += 1;
  }
  return 1;
}



void test_4x4_quantization ()
{
  int qp = 6;

  /* int coeff[4][4] = */
  /*     { {  1978 ,    34 ,   470 ,  -318 } , */
  /*       {   491 ,  -417 , -1507 ,  -526 } , */
  /*       {    48 ,  -100 ,  -392 ,   530 } , */
  /*       {    83 , -1481 ,   -91 ,   532 } }; */

  /* int orig[4][4] = */
  /*     { {  1978 ,    34 ,   470 ,  -318 } , */
  /*       {   491 ,  -417 , -1507 ,  -526 } , */
  /*       {    48 ,  -100 ,  -392 ,   530 } , */
  /*       {    83 , -1481 ,   -91 ,   532 } }; */


  int coeff[4][4] =
  { { -231,   -10,   -11,     5 },
    { -50,   -13,    24,    11 },
    {   5,     8,    -3,    -1 },
    {  -5,    -9,    -3,    -2 } };
  int orig[4][4] =
  { { -231,   -10,   -11,     5 },
    { -50,   -13,    24,    11 },
    {   5,     8,    -3,    -1 },
    {  -5,    -9,    -3,    -2 } };


  int aclevel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int acrun[16]   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int leveloffset[4][4] = { { 0, 0, 0, 0},
                            { 0, 0, 0, 0},
                            { 0, 0, 0, 0},
                            { 0, 0, 0, 0}};
  int coeff_cost = 0;

  quant_4x4_normal (coeff, 0, 0, qp,
                    aclevel, acrun,
                    0, quant_coef_jm[qp % 6], dequant_coef[qp % 6], leveloffset,
                    &coeff_cost, pos_scan, COEFF_COST4x4[0]);
  printf ("orig: ");
  for (int i = 0; i < 16; i++)
  {
    printf ("%5d ", orig[pos_scan[i][0]][pos_scan[i][1]]);
  }
  printf ("\n");

  printf ("level : ");
  for (int i = 0; i < 16; i++)
  {
    printf ("%5d ", aclevel[i]);
  }
  printf ("\n");

  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {

    }
  }

  printf ("run:   ");
  for (int i = 0; i < 16; i++)
  {
    printf ("%5d ", acrun[i]);
  }
  printf ("\n");

  printf ("dequant: ");
  for (int i = 0; i < 16; i++) {
    printf ("%5d ", coeff[pos_scan[i][0]][pos_scan[i][1]]);
  }
  printf ("\n");


  /**************************************/
  int num_of_coeffs = 0;
  int sum_of_levels = 0;
  quantize_4x4 (orig, qp, 0, 0, &num_of_coeffs, &sum_of_levels);
  printf ("tntm:  ");
  for (int i = 1; i < 16; i++)
  {
    printf ("%5d ", orig[pos_scan[i][0]][pos_scan[i][1]]);
  }
  printf ("\n");
}

void test_2x2_quantization ()
{
  int qp = 6;
  int coeff[2][2] = { { -765, -19},
                      { -21,   1}};
  int dclevel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int dcrun[16]   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  int aclevel[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int acrun[16]   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int leveloffset[4][4] = { { 0, 0, 0, 0},
                            { 0, 0, 0, 0},
                            { 0, 0, 0, 0},
                            { 0, 0, 0, 0}};
  int coeff_cost = 0;

  printf ("before\n");
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++)
    {
      printf ("%5d   ", coeff[i][j]);
    }
    printf ("\n");
  }
  printf ("\n");
  /* quant_dc2x2_normal (coeff, qp, dclevel, dcrun, 0, */
  /*                     quant_coef_jm[qp%6][0][0], */
  /*                     dequant_coef[qp%6][0][0], */
  /*                     0, pos_scan, 0); */
  printf ("after\n");
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++)
    {
      printf ("%5d   ", coeff[i][j]);
    }
    printf ("\n");
  }

}

void qadd_table (
  int inter)
{
  const int max_q_add = 5592405;
  /* This table is for intra, >> 1 if inter */
  printf ("%s\n", inter ? "inter" : "intra");
  for (int qp = 0; qp < 52; qp++)
  {
    int qp_div_6 = qp / 6;
    int old = 1398101 >> (7 - qp_div_6 + (inter ? 1 : 0));
    int div = inter ? 6 : 3;
    int q_add = (2 << (15 + (qp_div_6))) / div;
    int new_q_add = max_q_add >> (8 - qp_div_6 + (inter ? 1 : 0));
    printf ("qp = %2d\t div6 = %d\tq_add = %7d\t new = %7d old = %7d\n",
            qp, qp_div_6, q_add, new_q_add, old);
  }
}

int main ()
{
  qadd_table (1);
  qadd_table (0);

  return 0;
}
