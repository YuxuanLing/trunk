#ifndef SNR_H
#define SNR_H

float snr (
  const unsigned char * data1,
  const unsigned char * data2,
  const int width,
  const int height,
  const int stride);

double bdr (
  const double ref_psnr[4],
  const double new_psnr[4],
  const double ref_bitrate[4],
  const double new_bitrate[4]);

#endif
