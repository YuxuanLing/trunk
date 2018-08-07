//define TIMELOG if you used this
#pragma once
#include<stdio.h>
#include <assert.h>
#include "getMicroSecond.h"

#ifdef TIMELOG
typedef struct{
	char label[256];
	float startTime;
	float endTime;
	FILE *flog;
}TimeLog;

TimeLog timeLog;
#endif

inline void initLogging(const char* timeLogFilename){
#ifdef TIMELOG	
	timeLog.flog = fopen(timeLogFilename, "w");
	assert(timeLog.flog != NULL);
#endif	
}

inline void startLogging(){
#ifdef TIMELOG
	timeLog.startTime = getMicroSecond();
#endif
}

inline void endLogging(){
#ifdef TIMELOG
	timeLog.endTime = getMicroSecond();
#endif
}

inline void cleanLogging(){
#ifdef TIMELOG
	if(timeLog.flog != NULL) fclose(timeLog.flog);
#endif
}

inline void printTimeLog(const char* functionName){
#ifdef TIMELOG
	fprintf(timeLog.flog, "method=[ %s ] cputime=[ %.3f ]\n", functionName, timeLog.endTime-timeLog.startTime);
#endif
}

