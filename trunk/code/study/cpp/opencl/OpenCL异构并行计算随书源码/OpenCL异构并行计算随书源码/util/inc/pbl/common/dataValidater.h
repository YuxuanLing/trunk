/**
 * @file dataValidater.h
 * @author yyfn
 * @date 20100909
 **/
#pragma once
#include <math.h>
#include <stdio.h>

/**
 * @namespace DataValidater
 * @brief use for validate data
 *
 **/
#ifdef __cplusplus

namespace DataValidater{
/**
 * @brief Compare two float arrays using L2-norm with an epsilon tolerance for equality
 *
 * @return true if \a reference and \a data are identical, otherwise false
 * @param reference handle to the reference data / gold image
 * @param data handle to the computed data
 * @param len number of elements in reference and data
 * @param epsilon epsilon to use for the comparison
 **/
template<typename T>
bool compare2ArrayWithErrorRatio(const T* reference, const T* data,
		const size_t len, const float epsilon) {

	if (epsilon < 0.0f) {
		printf("epsilon must be larger than zero\n");
		exit(1);
	}

	if (reference == NULL) {
		printf("reference == NULL\n");
		exit(1);
	}

	if (data == NULL) {
		printf("data == NULL\n");
		exit(1);
	}

	float ref = 0.0f;
	float error = 0.0f;

	for (size_t i = 0; i < len; i++) {
		float diff = reference[i] - data[i];
		error += diff * diff;
		ref += reference[i] * reference[i];
	}

	if (fabs(ref) < 1e-7) {
		std::cerr << "ERROR, reference l2-norm is 0\n";
		return false;
	}

	bool result = sqrtf(error / ref) < epsilon;

	return result;
}
/**
 * @brief Compare two arrays of arbitrary type
 * @return true if \a reference and \a data are identical, otherwise false
 * @param reference  handle to the reference data
 * @param data       handle to the computed data
 * @param len        number of elements in reference and data
 * @param epsilon    epsilon to use for the comparison
 **/
template<typename T>
int compare2ArrayWithErrorLimit(const T* reference, const T* data,
		const size_t len, const float epsilon) {

	if (epsilon < 0.0f) {
		printf("epsilon must be larger than zero\n");
		exit(1);
	}

	if (reference == NULL) {
		printf("reference == NULL\n");
		exit(1);
	}

	if (data == NULL) {
		printf("data == NULL\n");
		exit(1);
	}

	int errorCounter = 0;

	for (size_t i = 0; i < len; i++) {
		if (fabs(reference[i]) < 1e-6) {
			printf("reference = %f is too small\n", reference[i]);
			continue;
		}
		bool comp = (1.0f - 1.0f * data[i] / reference[i]) < epsilon;

		if (comp) {
			errorCounter++;
			printf("id = %d (reference = %f) - (data = %f) = %f\n", i,
					reference[i], data[i], reference[i] - data[i]);
		}
	}
	return errorCounter;
}

int compare2ArrayWithErrorLimit(const int *reference, const int *data,
		size_t num) {
	int count = 0;
	for (size_t i = 0; i < num; i++) {
		if (reference[i] != data[i]) {
			count++;
			printf("i = %ld (reference = %d) - (data = %d) = %d\n", i,
					reference[i], data[i], reference[i] - data[i]);
		}
	}

	return count;
}
};

#endif
