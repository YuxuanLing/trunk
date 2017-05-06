/**
 * @file 
 * @author yyfn
 *
 * @brief this file is time counter classes implemention
 **/
#pragma once
#include <time.h>
#if defined(_WIN32) && defined(_MSC_VER)
#include <windows.h>
#endif
#include "../CUDA.h"
#include "../linuxUtil.h"

/**
 * @class TimeCounter
 * @brief super class, use for timing, every timing class must extends this class and override method getTimeDiff() and printTimeDiff()
 */
class CPUTimer {
protected:
	///start timing point var
	clock_t startp;
	///end timing point var
	clock_t endp;
	/**
	 * @brief constructor, init startp and endp
	 */
	CPUTimer() :
			startp(-1), endp(-1) {
	}

public:
	///start timing
	virtual void start() {
#ifdef __CUDACC__
		CudaDeviceSynchronize();
#endif
		startp = clock();
	}

	///stop timing
	virtual void stop() {
		if ((clock_t) -1 == startp) {
			perror("you must set start point at first");
		} else {
#ifdef __CUDACC__
			CudaDeviceSynchronize();
#endif
			endp = clock();
		}
	}
	/**
	 * @brief abstract method
	 * @return time difference
	 */
	virtual long getTimeDiff()=0;

	/**
	 * @brief abstract method, print time difference
	 */
	virtual void printTimeDiff()=0;
};

/**
 * @class SecondCounter
 * @brief second time counter
 */
class SecondCPUTimer: public CPUTimer {
public:
	long getTimeDiff() {

		if ((clock_t) -1 == endp) {
			perror("you must set stop point before invoke this function");
			exit(1);
		} else {
			return (endp - startp) / CLOCKS_PER_SEC;
		}
	}
	void printTimeDiff() {
		long temp = getTimeDiff();
		printf("use time :%lds\n", temp);
	}
};

/**
 * @class MillisecondCounter
 * @brief millisecond time counter
 */
class MillisecondCPUTimer: public CPUTimer {
public:
	long getTimeDiff() {

		if ((clock_t) -1 == endp) {
			perror("you must set stop point before invoke this function");
			exit(1);
		} else {
			return 1.0f * (endp - startp) / CLOCKS_PER_SEC * 1000;
		}
	}
	void printTimeDiff() {
		long temp = getTimeDiff();
		printf("use time :%ldms\n", temp);
	}
};

/**
 * @class MicrosecondCounter
 * @brief microsecond time counter
 */

class MicrosecondCPUTimer: public CPUTimer {
public:
	long getTimeDiff() {
		if ((clock_t) -1 == endp) {
			printf("please set start point or end point\n");
			exit(1);
		} else {
			return 1.0f * (endp - startp) / CLOCKS_PER_SEC * 1000000;
		}
	}
	void printTimeDiff() {
		long temp = getTimeDiff();
		printf("use time:%ld us\n", temp);
	}
};

class MicroSysCPUTimer: public CPUTimer {
private:
	long s, e;
	long getTimeInMicroSecs() {
		long ret = 0.0;
#if defined(_WIN32) && defined(_MSC_VER)

		__int64 freq;
		__int64 clock;
		QueryPerformanceFrequency( (LARGE_INTEGER *)&freq );
		QueryPerformanceCounter( (LARGE_INTEGER *)&clock );
		ret = clock/freq*1000*1000;
#else
ret = getMicroSecond();
#endif

		return ret;
	}

public:
	void start() {
#ifdef __CUDACC__
		CudaDeviceSynchronize();
#endif
		s = getTimeInMicroSecs();
	}

	void end() {
#ifdef __CUDACC__
		CudaDeviceSynchronize();
#endif
		e = getTimeInMicroSecs();
	}

	long getTimeDiff() {
		return e - s;
	}

	void printTimeDiff() {
		printf("use time :%ldmicro second\n", getTimeDiff());
	}

};
