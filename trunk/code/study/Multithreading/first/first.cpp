// first.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"  
#include <windows.h>  
#include <omp.h>  
#include<iostream>
#include<thread>

#define NUMBER 1000000000  
void hello()
{
	std::cout << "Hello Concurrent World\n";
}
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

class background_task
{
public:
	void operator () ()const
	{
		std::cout << "background_task operator ()\n";
	}

};


int _tmain(int argc, _TCHAR* argv[])
{
	background_task tsk;
	std::thread t0(hello);
	std::thread t1(test1);
	std::thread t2(test2);
	//std::thread t3{ background_task() };
	std::thread t3((background_task()));
	std::thread t4(tsk);
	

	t0.join();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	return 0;
}
