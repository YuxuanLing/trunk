#include <time.h>

void print_result (
  const char * name,
  clock_t      start,
  clock_t      finish);

int test_enc_me_speed();
int test_enc_deblock_speed();
int test_enc_interpolate_speed();
