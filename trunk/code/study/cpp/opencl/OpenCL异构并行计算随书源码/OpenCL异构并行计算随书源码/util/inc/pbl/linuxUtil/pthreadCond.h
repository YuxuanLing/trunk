#pragma once
#include "yyfnpthread.h"


template<typename T>
class CondMutex{
public:
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	T value;

	CondMutex(T v){
		value = v;
		Pthread_mutex_init(&mutex, NULL);
		Pthread_cond_init(&cond, NULL);
	}

	~CondMutex(){
		Pthread_mutex_destroy(&mutex);
		Pthread_cond_destroy(&cond);
	}
private:
	CondMutex(const CondMutex& cm){}
};

