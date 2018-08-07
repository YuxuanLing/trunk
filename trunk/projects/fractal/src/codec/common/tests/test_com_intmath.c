#include "com_intmath.h"
#include "fractaltest.h"

static void test_div6 (void)
{
  for (int qp = 0; qp < 52; qp++)
  {
    FASSERT (qp / 6 == TAA_H264_DIV6 (qp));
  }
}

void test_com_intmath (void)
{
  test_div6 ();
}
