#ifndef SSIM_H
#define SSIM_H

float ssim(const uint8_t *original, 
		   const uint8_t *encoded, 
		   const int width, 
		   const int height, 
		   const int stride);
#endif