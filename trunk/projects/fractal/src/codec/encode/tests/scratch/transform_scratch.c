/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#include <stdio.h>
/* Section 8.5.10 */
#define BLOCK_SIZE 4

int tmp[64];

static void forward4x4(
  int block[4][4],
  int tblock[4][4],
  int pos_y,
  int pos_x)
{
  int i, ii;
  int * pTmp = tmp, * pblock;
  int p0, p1, p2, p3;
  int t0, t1, t2, t3;

  /* Horizontal */
  for (i = pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &block[i][pos_x];
    p0 = *(pblock++);
    p1 = *(pblock++);
    p2 = *(pblock++);
    p3 = *(pblock  );

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    *(pTmp++) =  t0 + t1;
    *(pTmp++) = (t3 << 1) + t2;
    *(pTmp++) =  t0 - t1;
    *(pTmp++) =  t3 - (t2 << 1);
  }

  /* Vertical */
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    p0 = *pTmp;
    p1 = *(pTmp += BLOCK_SIZE);
    p2 = *(pTmp += BLOCK_SIZE);
    p3 = *(pTmp += BLOCK_SIZE);

    t0 = p0 + p3;
    t1 = p1 + p2;
    t2 = p1 - p2;
    t3 = p0 - p3;

    ii = pos_x + i;
    tblock[pos_y    ][ii] = t0 +  t1;
    tblock[pos_y + 1][ii] = t2 + (t3 << 1);
    tblock[pos_y + 2][ii] = t0 -  t1;
    tblock[pos_y + 3][ii] = t3 - (t2 << 1);
  }
}


static void inverse4x4(
  int tblock[4][4],
  int block[4][4],
  int pos_y,
  int pos_x)
{
  int i, ii;
  int * pTmp = tmp, * pblock;
  int p0, p1, p2, p3;
  int t0, t1, t2, t3;

  /* Horizontal */
  for (i = pos_y; i < pos_y + BLOCK_SIZE; i++)
  {
    pblock = &tblock[i][pos_x];
    t0 = *(pblock++);
    t1 = *(pblock++);
    t2 = *(pblock++);
    t3 = *(pblock  );

    p0 =  t0 + t2;
    p1 =  t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 =  t1 + (t3 >> 1);

    *(pTmp++) = p0 + p3;
    *(pTmp++) = p1 + p2;
    *(pTmp++) = p1 - p2;
    *(pTmp++) = p0 - p3;
  }

  /*  Vertical */
  for (i = 0; i < BLOCK_SIZE; i++)
  {
    pTmp = tmp + i;
    t0 = *pTmp;
    t1 = *(pTmp += BLOCK_SIZE);
    t2 = *(pTmp += BLOCK_SIZE);
    t3 = *(pTmp += BLOCK_SIZE);

    p0 = t0 + t2;
    p1 = t0 - t2;
    p2 = (t1 >> 1) - t3;
    p3 = t1 + (t3 >> 1);

    ii = i + pos_x;
    block[pos_y    ][ii] = p0 + p3;
    block[pos_y + 1][ii] = p1 + p2;
    block[pos_y + 2][ii] = p1 - p2;
    block[pos_y + 3][ii] = p0 - p3;
  }
}


int main (
  void)
{
  int out[4][4];
  int in3[4][4] =
  { {    31,   201,   251,   130 },
    {   170,   211,     5,   129 },
    {   150,    14,    37,   249 },
    {   236,    18,    17,   129 } };
  int in[4][4] =
  { {    31,   201,   251,   130 },
    {   170,   211,     5,   129 },
    {   150,    14,    37,   249 },
    {   236,    18,    17,   129 } };

  int in2[4][4] =
  { {    0,   1,   1,   0 },
    {    2,   0,   0,   0 },
    {    0,   0,   0,   0 },
    {    0,   0,   0,   0 } };

  forward4x4 (in, out, 0, 0);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf ("%2d ", out[i][j]);
    }
    printf ("\n");
  }

  int coeffs[4][4] =
  { {  1978,    34,   470,  -318 },
    {   491,  -417, -1507,  -526 },
    {    48,  -100,  -392,   530 },
    {    83, -1481,   -91,   532 } };
  inverse4x4 (coeffs, out, 0, 0);
  printf ("Inverse: \n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      printf ("%5d ", out[i][j]);
    }
    printf ("\n");
  }

  return 0;
}
