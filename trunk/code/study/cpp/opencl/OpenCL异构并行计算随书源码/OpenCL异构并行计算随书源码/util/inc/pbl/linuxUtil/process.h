#ifdef H_LINUXUTIL_PROCESS
#define H_LINUXUTIL_PROCESS

/**
 * @file
 * @author yyfn
 *
 * @brief wrap for linux process function
 */
#pragma once
#include <unistd.h>

int Fork() {
	int ret = fork();
	if(0 > ret){
		printf("%s, %d, fork failed\n", __FILE__, __LINE__);
	}

	return ret;
}

#define Execve(file, argv, envp) {\
	if(0 > execve(file, argv, envp)){\
		printf("%s, %d, execve failed\n", __FILE__, __LINE__);\
		exit(0);\
	}\
}

const char* Getenv(const char *name) {
	const char *ret = getenv(name);
	if(NULL == ret){
		printf("%s, %d, getenv failed\n", __FILE__, __LINE__);
	}

	return ret;
}

#define Setenv(name, value, ow) {\
	if(-1 == setenv(name, value, ow)){\
		printf("%s, %d, setenv failed\n", __FILE__, __LINE__);\
	}\
}

unsigned int Sleep(unsigned int time){
	unsigned int ret = sleep(time);
	if(0 != ret){
		printf("i am interrupted\n");
	}

	return ret;
}

#define Waitpid(pid, status, options) {\
	if(1 > waitpid(pid, status, options)){\
		printf("%s, %d, waitpid failed\n", __FILE__, __LINE__);\
	}\
}

#define Wait(status) {\
	if(1 > wait(status)){\
		printf("%s, %d, wait failed\n", __FILE__, __LINE__);\
	}\
}

#define Kill(pid, sig) {\
	if(0 != kill(pid, sig)){\
		printf("%s, %d, kill failed\n", __FILE__, __LINE__);\
	}\
}

unsigned int Alarm(unsigned int time){
	unsigned int ret = alarm(time);
	if(0 != ret){
		printf("this alarm is reset\n");
	}

	return ret;
}

#define Signal(sig, handler) {\
	if(SIG_ERR == signal(sig, handler)){\
		printf("%s, %d, invoke signal failed\n", __FILE__, __LINE__);\
	}\
}

#define Popen(command) {\
	if(NULL == popen(command)){\
		printf("%s, %d, popen failed\n", __FILE__, __LINE__);\
	}\
}

#define Pclose(fp) {\
	if(0 > pclose(fp)){\
		printf("%s, %d, pclose failed\n", __FILE__, __LINE__);\
	}\
}

#endif
