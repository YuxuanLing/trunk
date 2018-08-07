/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#include <stdio.h>


#define BLOCK_SIZE 4
int tmp[64];


void dump_4x4 (
  int          m[4][4],
  const char * title)
{
  printf ("=============================\n");
  printf ("%s\n", title);
  printf ("-----------------------------\n");
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      printf ("%5d ", m[i][j]);
    }
    printf ("\n");
  }
}


void forward4x4(
  int block[4][4],
  int tblock[4][4])
{
  int i, ii;
  int * pTmp = tmp, * pblock;
  static int p0, p1, p2, p3;
  static int t0, t1, t2, t3;
  int pos_y = 0;
  int pos_x = 0;

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
  int block[4][4])
{
  int i, ii;
  int * pTmp = tmp, * pblock;
  int p0, p1, p2, p3;
  int t0, t1, t2, t3;
  int pos_y = 0;
  int pos_x = 0;

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

static inline int imin(
  int a,
  int b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int imax(
  int a,
  int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int rshift_rnd_sf(
  int x,
  int a)
{
  return ((x + (1 << (a - 1) )) >> a);
}

static inline int iClip1(
  int high,
  int x)
{
  x = imax(x, 0);
  x = imin(x, high);

  return x;
}


int main ()
{
  /* int residual[4][4] = */
  /*     { {    31 ,   201 ,   251 ,   130 } , */
  /*       {   170 ,   211 ,     5 ,   129 } , */
  /*       {   150 ,    14 ,    37 ,   249 } , */
  /*       {   236 ,    18 ,    17 ,   129 } }; */
  int residual[4][4] =
  { {    1,   2,   3,   4 },
    {   5,   6,     7,   8 },
    {   9,    10,    11,   12 },
    {   13,    14,    15,   16 } };
  int in1[4][4] =
  { {     0,     0,     0,     0 },
    {     0,     0,     0,     0 },
    {     0,     0,     0,     0 },
    {     0,     0,     0,     0 } };

  int coeff[4][4];
  int rresidual[4][4];

  inverse4x4 (in1, rresidual);
  dump_4x4 (rresidual, "Recon residual");



  dump_4x4 (residual, "Residual");
  forward4x4 (residual, coeff);
  dump_4x4 (coeff, "Coeffisients");
  inverse4x4 (coeff, rresidual);
  dump_4x4 (rresidual, "Recon residual");
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++) {
      int tmp = rresidual[i][j];
      rresidual[i][j] = rshift_rnd_sf (tmp, 6);
    }
  }
  dump_4x4 (rresidual, "Scaled recon residual");

  return 0;
}
