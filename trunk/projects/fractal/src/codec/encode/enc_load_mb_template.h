static void __declspec(noinline) taa_h264_pad_mb_NxN (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t mb_y[N][N],
  uint8_t mb_u[N/2][N/2],
  uint8_t mb_v[N/2][N/2],
  const int stride,
  int lpad,
  int rpad,
  int tpad,
  int bpad
  )
{
  const int cstride = stride >> 1;

  __assume_aligned(mb_y, 16);
  __assume_aligned(mb_u, 16);
  __assume_aligned(mb_v, 16);

  // Luma y
  for (int j = tpad; j < N-bpad; j++)
    for (int i = lpad; i < N-rpad; i++)
      mb_y[j][i] = mbsrc_y[(j-3)*stride+i-3];

  // Chroma u
  for (int j = tpad; j <(N-bpad)/2; j++)
    for (int i = lpad; i < (N-rpad)/2; i++)
      mb_u[j][i] = mbsrc_u[(j-3)*cstride+i-3];

  // Chroma v
  for (int j = tpad; j <(N-bpad)/2; j++)
    for (int i = lpad; i < (N-rpad)/2; i++)
      mb_v[j][i] = mbsrc_v[(j-3)*cstride+i-3];

  // Pad top
  if (tpad)
  {
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N; i++)
        mb_y[j][i] = mb_y[3][i];
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N/2; i++)
        mb_u[j][i] = mb_u[3][i];
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N/2; i++)
        mb_v[j][i] = mb_v[3][i];
  }
  // Pad left
  if (lpad)
  {
    for (int j = 0; j < N; j++)
      for (int i = 0; i < 3; i++)
        mb_y[j][i] = mb_y[j][3];
    for (int j = 0; j < N/2; j++)
      for (int i = 0; i < 3; i++)
        mb_u[j][i] = mb_u[j][3];
    for (int j = 0; j < N/2; j++)
      for (int i = 0; i < 3; i++)
        mb_v[j][i] = mb_v[j][3];
  }
  // Pad bottom
  if (bpad)
  {
    for (int j = N-bpad; j < N; j++)
      for (int i = 0; i < N; i++)
        mb_y[j][i] = mb_y[N-bpad][i];
    for (int j = (N-bpad)/2; j < N/2; j++)
      for (int i = 0; i < N/2; i++)
        mb_u[j][i] = mb_u[N-bpad][i];
    for (int j = (N-bpad)/2; j < N/2; j++)
      for (int i = 0; i < N/2; i++)
        mb_v[j][i] = mb_v[N-bpad][i];
  }
  // Pad right
  if (rpad)
  {
    for (int j = 0; j < N; j++)
      for (int i = N-rpad; i < N; i++)
        mb_y[j][i] = mb_y[j][N-rpad];
    for (int j = 0; j < N/2; j++)
      for (int i = (N-rpad)/2; i < N/2; i++)
        mb_u[j][i] = mb_u[j][(N-rpad)/2];
    for (int j = 0; j < N/2; j++)
      for (int i = (N-rpad)/2; i < N/2; i++)
        mb_v[j][i] = mb_v[j][(N-rpad)/2];
  }
}

static void __declspec(noinline) taa_h264_load_mb_NxN (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t mb_y[N][N],
  uint8_t mb_u[N/2][N/2],
  uint8_t mb_v[N/2][N/2],
  const int stride)
{
  const int cstride = stride >> 1;

  __assume_aligned(mb_y, 16);
  __assume_aligned(mb_u, 16);
  __assume_aligned(mb_v, 16);

  // Luma y
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      mb_y[i][j] = mbsrc_y[(i-3)*stride+j-3];

  // Chroma u
  for (int i = 0; i < N/2; i++)
    for (int j = 0; j < N/2; j++)
      mb_u[i][j] = mbsrc_u[(i-3)*cstride+j-3];

  // Chroma v
  for (int i = 0; i < N/2; i++)
    for (int j = 0; j < N/2; j++)
      mb_v[i][j] = mbsrc_v[(i-3)*cstride+j-3];
}
