// first.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"  
#include <windows.h>  
#include <omp.h>  

#define NUMBER 1000000000  

int process(int start, int end)
{
	int total;

	total = 0;
	while (start < end) {
		total += start % 2;
		start++;
	}

	return total;
}

void test1()
{
	int i;
	int time;
	struct {
		int start;
		int end;
	}value[] = {
		{ 0 , NUMBER >> 1 },
		{ NUMBER >> 1, NUMBER }
	};
	int total[2] = { 0 };
	int result;

	time = GetTickCount();

#pragma omp parallel for  
	for (i = 0; i < 2; i++)
	{
		total[i] = process(value[i].start, value[i].end);
	}

	result = total[0] + total[1];
	printf("%d\n", GetTickCount() - time);
}

void test2()
{
	int i;
	int value;
	int total;

	total = 0;
	value = GetTickCount();

	for (i = 0; i < NUMBER; i++)
	{
		total += i % 2;
	}
	printf("%d\n", GetTickCount() - value);
}


int _tmain(int argc, _TCHAR* argv[])
{
	test1();
	test2();
	return 0;
}
