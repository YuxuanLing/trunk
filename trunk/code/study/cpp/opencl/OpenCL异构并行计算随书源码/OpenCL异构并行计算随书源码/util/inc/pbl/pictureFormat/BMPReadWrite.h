/**
 * @file
 * @author yyfn
 *
 * @brief wrap for accessing BMP file, considered CUDA pinned memory
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __CUDACC__
#include "../CUDA/checkCUDAError.h"
#endif

typedef unsigned char uchar;
//currently just support 24 bit bmp file
#define DEFAULTBIT  24

typedef struct {
	short type; // === "BM"
	int fileSize; //Bytes
	int reserved; // === 0
	int offset;
	int headSize;
	int width;
	int height;
	short planes; //=== 1
}__attribute__((packed)) BMPHeader;

/*bitmap file information struct*/
typedef struct {
	short bpp; //bit per pixel
	int compression;
	int size;
	int h_resolution;
	int v_resolution;
	int colors;
	int important_colors;
}__attribute__((packed)) BMPInfo;

/*bitmap pixel data struct*/
typedef struct {
	uchar *r;
	uchar *g;
	uchar *b;
} Pixels;

inline void mallocPixelsHost(Pixels *ps, size_t width, size_t height) {
#ifdef __CUDACC__
	checkCUDAError(
			cudaMallocHost((void**) &(ps->r), width * height * sizeof(uchar)));
	checkCUDAError(
			cudaMallocHost((void**) &(ps->g), width * height * sizeof(uchar)));
	checkCUDAError(
			cudaMallocHost((void**) &(ps->b), width * height * sizeof(uchar)));
#else
	ps->r = (uchar*)malloc(width*height*sizeof(uchar));
	ps->g = (uchar*)malloc(width*height*sizeof(uchar));
	ps->b = (uchar*)malloc(width*height*sizeof(uchar));
#endif
}

#ifdef __CUDACC__
inline void mallocPixelsDevice(Pixels *d_ps, size_t *pitch, size_t width,
		size_t height) {
	size_t spitch;
	checkCUDAError(
			cudaMallocPitch((void**) &(d_ps->r), &spitch, width * sizeof(uchar),
					height));
	spitch /= sizeof(uchar);
	*pitch = spitch;

	checkCUDAError(
			cudaMalloc((void**) &(d_ps->g), spitch * height * sizeof(uchar)));
	checkCUDAError(
			cudaMalloc((void**) &(d_ps->b), spitch * height * sizeof(uchar)));
}

inline void freePixelsDevice(Pixels *d_ps) {
	checkCUDAError(cudaFree(d_ps->r));
	checkCUDAError(cudaFree(d_ps->g));
	checkCUDAError(cudaFree(d_ps->b));
}

inline void memcpyPixels(Pixels *src, size_t spitch, Pixels *dst, size_t dpitch,
		size_t widthInBytes, size_t height, cudaMemcpyKind dir) {
	if (cudaMemcpyDeviceToDevice == dir) {
		checkCUDAError(
				cudaMemcpy2D(src->r, spitch, dst->r, dpitch, widthInBytes,
						height, dir));
	} else {
		checkCUDAError(
				cudaMemcpy2DAsync(src->r, spitch, dst->r, dpitch, widthInBytes,
						height, dir, 0));
		checkCUDAError(
				cudaMemcpy2DAsync(src->g, spitch, dst->g, dpitch, widthInBytes,
						height, dir, 0));
		checkCUDAError(
				cudaMemcpy2DAsync(src->b, spitch, dst->b, dpitch, widthInBytes,
						height, dir, 0));
	}
}
#endif

inline void freePixelsHost(Pixels *p) {
#ifdef __CUDACC__
	checkCUDAError(cudaFreeHost(p->r));
	checkCUDAError(cudaFreeHost(p->g));
	checkCUDAError(cudaFreeHost(p->b));
#else
	::free(p->r);
	::free(p->g);
	::free(p->b);
#endif
}
//pixel store in one dimemsion, row first
void writeBMPFile(const char *file, Pixels ps, BMPHeader bmpHeader,
		BMPInfo bmpInfo) {
	if (NULL == file) {
		printf("parameter ps and file is NULL\n");
		exit(1);
	}

	FILE *fp = fopen(file, "w");
	if (NULL == fp) {
		printf("%s %d ", __FILE__, __LINE__);
		perror("");
		exit(1);
	}
//write header
	fwrite((void*) &bmpHeader, 1, sizeof(BMPHeader), fp);

//write info
	fwrite((void*) &bmpInfo, 1, sizeof(BMPInfo), fp);

	int width = bmpHeader.width;
	int height = bmpHeader.height;

	if (0 != fseek(fp, bmpHeader.offset, SEEK_SET)) {
		printf("%s %d ", __FILE__, __LINE__);
		perror("");
		fclose(fp);
		return;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int p = i * width + j;
			fwrite((void *) &(ps.b[p]), 1, sizeof(uchar), fp);
			fwrite((void *) &(ps.g[p]), 1, sizeof(uchar), fp);
			fwrite((void *) &(ps.r[p]), 1, sizeof(uchar), fp);
		}

		if ((0 != width % 4)
				&& (0 != fseek(fp, 4 - (3 * width) % 4, SEEK_CUR))) {
			printf("%s %d ", __FILE__, __LINE__);
			perror("seek error\n");
			fclose(fp);
			exit(1);
		}
	}

	if (NULL != fp) {
		fclose(fp);
	}
}

void readBMPFile(const char *file, Pixels *ps, BMPHeader *bmpHeader,
		BMPInfo *bmpInfo) {
	if (NULL == file || NULL == ps || NULL == bmpHeader || NULL == bmpInfo) {
		printf("parameter error\n");
		exit(1);
	}

	FILE *fp = fopen(file, "r");
	if (NULL == fp) {
		printf("%s %d ", __FILE__, __LINE__);
		perror("");
		exit(1);
	}

	int bytes = fread(bmpHeader, 1, sizeof(BMPHeader), fp);
	if (sizeof(BMPHeader) != bytes) {
		printf("read bmp header error, bytes = %d!\n", bytes);
		fclose(fp);
		exit(1);
	}
	/*  printf("sizeof BMPHeader = %ld\n", sizeof(BMPHeader));
	 printf("type = %x\n", bmpHeade->type); // === "BM"
	 printf("size = %d\n", bmpHeade->fileSize); //Bytes
	 printf("reserved = %d\n", bmpHeade->reserved); // === 0
	 printf("offset = %d\n", bmpHeade->offset);
	 printf("headerSize = %d\n", bmpHeade->headSize);
	 printf("width = %d\n", bmpHeade->width);
	 printf("height = %d\n", bmpHeade->height);
	 printf("plane = %d\n", bmpHeade->planes); //=== 1
	 */
	bytes = fread(bmpInfo, 1, sizeof(BMPInfo), fp);
	if (sizeof(BMPInfo) != bytes) {
		printf("read bmp information error, %d bytes\n", bytes);
//        perror("");
		fclose(fp);
		exit(1);
	}
	/*
	 printf("bit per pixel = %d\n", bmpInfo->bpp);//bit per pixel
	 printf("compressed ? %d\n", bmpInfo->compression);
	 printf("data size = %d\n", bmpInfo->size);
	 printf("h_resolution = %d\n", bmpInfo->h_resolution);
	 printf("v_resolution = %d\n", bmpInfo->v_resolution);
	 printf("colors = %d\n", bmpInfo->colors);
	 printf("important_colors = %d\n", bmpInfo->important_colors);
	 */
	if (DEFAULTBIT != bmpInfo->bpp) {
		printf("bpp %d is not support!\n", bmpInfo->bpp);
		fclose(fp);
		exit(1);
	}

	size_t width = bmpHeader->width;
	size_t height = bmpHeader->height;

	if (0 != fseek(fp, bmpHeader->offset, SEEK_SET)) {
		printf("%s %d ", __FILE__, __LINE__);
		perror("seek error\n");
		fclose(fp);
		exit(1);
	}

	mallocPixelsHost(ps, width, height);

	for (size_t i = 0; i < height; i++) {
		for (size_t j = 0; j < width; j++) {
			size_t p = i * width + j;
			bool check = true;
			check =
					check
							&& (sizeof(uchar)
									== fread((void *) &(ps->b[p]), 1,
											sizeof(uchar), fp));
			check =
					check
							&& (sizeof(uchar)
									== fread((void *) &(ps->g[p]), 1,
											sizeof(uchar), fp));
			check =
					check
							&& (sizeof(uchar)
									== fread((void *) &(ps->r[p]), 1,
											sizeof(uchar), fp));
			if (!check) {
				printf("%s %d, i = %ld, j = %ld\n", __FILE__, __LINE__, i, j);
				printf("read bmp data error, bmp file is not complete!\n");
				fclose(fp);
				exit(1);
			}
		}
		//row data is aligned to 4 bytes
		if ((0 != width % 4)
				&& (0 != fseek(fp, 4 - (width * 3) % 4, SEEK_CUR))) {
			printf("%s %d ", __FILE__, __LINE__);
			perror("seek error\n");
			fclose(fp);
			exit(1);
		}
	}

	if (NULL != fp) {
		fclose(fp);
	}
}
